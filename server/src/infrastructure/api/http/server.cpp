#include "server.hpp"

#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/log/trivial.hpp>

HttpServer::HttpServer(std::string host, std::uint16_t port,
                       std::shared_ptr<Router> router, std::size_t thread_count)
    : host_(std::move(host)),
      port_(port),
      acceptor_(io_context_),
      router_(std::move(router)) {
  if (thread_count == 0) {
    const auto hw = std::thread::hardware_concurrency();
    thread_count_ = hw == 0 ? 1 : static_cast<std::size_t>(hw);
  } else {
    thread_count_ = thread_count;
  }
}

HttpServer::~HttpServer() {
  Stop();
}

void HttpServer::Run() {
  work_guard_.emplace(net::make_work_guard(io_context_));

  workers_.reserve(thread_count_);
  for (std::size_t i = 0; i < thread_count_; ++i) {
    workers_.emplace_back([this, i] {
      try {
        io_context_.run();
      } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(error)
            << "io_context run exception: " << e.what() << "in thread " << i;
      }
    });
  }

  beast::error_code ec;
  ip::tcp::endpoint endpoint(ip::make_address(host_, ec), port_);
  if (ec) {
    throw std::runtime_error("Invalid host address: " + host_ +
                             ", error: " + ec.message());
  }
  ec = acceptor_.open(endpoint.protocol(), ec);
  if (ec)
    throw std::runtime_error(ec.message());
  acceptor_.set_option(ip::tcp::acceptor::reuse_address(true));
  ec = acceptor_.bind(endpoint, ec);
  if (ec)
    throw std::runtime_error(ec.message());
  ec = acceptor_.listen(net::socket_base::max_listen_connections, ec);
  if (ec)
    throw std::runtime_error(ec.message());

  for (int i = 0; i < thread_count_; ++i) {
    StartAcceptLoop();
  }

  for (auto& t : workers_)
    t.join();
}

void HttpServer::Stop() {
  beast::error_code ec;
  acceptor_.cancel(ec);
  acceptor_.close(ec);
  if (work_guard_)
    work_guard_->reset();
  io_context_.stop();
  for (auto& t : workers_) {
    if (t.joinable())
      t.join();
  }
  workers_.clear();
}

void HttpServer::StartAcceptLoop() {
  auto socket = std::make_shared<ip::tcp::socket>(io_context_);
  acceptor_.async_accept(*socket,
                         [this, socket](const boost::system::error_code& ec) {
                           OnAccept(ec, socket);
                         });
}

void HttpServer::DoAccept(std::shared_ptr<ip::tcp::socket> socket) {
  auto buffer = std::make_shared<beast::flat_buffer>();
  auto request = std::make_shared<http::request<http::string_body>>();
  http::async_read(
      *socket, *buffer, *request,
      [this, socket, buffer, request](beast::error_code ec, std::size_t bytes) {
        OnRead(ec, bytes, socket, buffer, request);
      });
}

void HttpServer::DoRead(
    std::shared_ptr<ip::tcp::socket> socket,
    std::shared_ptr<beast::flat_buffer> /*buffer*/,
    std::shared_ptr<http::request<http::string_body>> request) {
  auto response = router_->Dispatch(*request);

  BOOST_LOG_TRIVIAL(info) << "Request received: " << request->method_string()
                          << " " << request->target()
                          << " | Thread ID: " << std::this_thread::get_id();

  DoWrite(
      socket,
      std::make_shared<http::response<http::string_body>>(std::move(response)),
      request->keep_alive());
}

void HttpServer::DoWrite(
    std::shared_ptr<ip::tcp::socket> socket,
    std::shared_ptr<http::response<http::string_body>> response,
    bool keep_alive) {
  http::async_write(*socket, *response,
                    [this, socket, response, keep_alive](beast::error_code ec,
                                                         std::size_t bytes) {
                      OnWrite(ec, bytes, socket, response, keep_alive);
                    });
}

void HttpServer::OnAccept(const boost::system::error_code& ec,
                          std::shared_ptr<ip::tcp::socket> socket) {
  if (ec) {
    BOOST_LOG_TRIVIAL(error) << ec.message();
    return;
  }
  DoAccept(std::move(socket));

  auto new_socket = std::make_shared<ip::tcp::socket>(io_context_);
  acceptor_.async_accept(
      *new_socket, [this, new_socket](const boost::system::error_code& ec) {
        OnAccept(ec, new_socket);
      });
}

void HttpServer::OnRead(
    beast::error_code ec, std::size_t /*bytes_transferred*/,
    std::shared_ptr<ip::tcp::socket> socket,
    std::shared_ptr<beast::flat_buffer> buffer,
    std::shared_ptr<http::request<http::string_body>> request) {
  if (ec) {
    if (ec == http::error::end_of_stream) {
      socket->shutdown(ip::tcp::socket::shutdown_both);
      return;
    }
    BOOST_LOG_TRIVIAL(error) << ec.message();
    return;
  }

  DoRead(std::move(socket), std::move(buffer), std::move(request));
}

void HttpServer::OnWrite(
    beast::error_code ec, std::size_t /*bytes_transferred*/,
    std::shared_ptr<ip::tcp::socket> socket,
    std::shared_ptr<http::response<http::string_body>> /*response*/,
    bool keep_alive) {
  if (ec) {
    BOOST_LOG_TRIVIAL(error) << ec.message();
    return;
  }

  if (keep_alive) {
    auto buffer = std::make_shared<beast::flat_buffer>();
    auto request = std::make_shared<http::request<http::string_body>>();
    http::async_read(*socket, *buffer, *request,
                     [this, socket, buffer, request](beast::error_code ec2,
                                                     std::size_t bytes2) {
                       OnRead(ec2, bytes2, socket, buffer, request);
                     });
  } else {
    socket->shutdown(ip::tcp::socket::shutdown_both);
  }
}
