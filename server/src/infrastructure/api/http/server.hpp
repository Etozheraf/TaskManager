#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <cstdint>
#include <memory>
#include <string>

#include "router.hpp"

namespace net = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace ip = net::ip;

class HttpServer {
 public:
  HttpServer(std::string host, std::uint16_t port,
             std::shared_ptr<Router> router, std::size_t thread_count = 0);

  ~HttpServer();

  void Run();
  void Stop();

 private:
  void StartAcceptLoop();
  void DoAccept(std::shared_ptr<ip::tcp::socket> socket);

  void DoRead(std::shared_ptr<ip::tcp::socket> socket,
              std::shared_ptr<beast::flat_buffer> buffer,
              std::shared_ptr<http::request<http::string_body>> request);

  void DoWrite(std::shared_ptr<ip::tcp::socket> socket,
               std::shared_ptr<http::response<http::string_body>> response,
               bool keep_alive);

  // On* методы — колбэки завершения async-операций
  void OnAccept(const boost::system::error_code& ec,
                std::shared_ptr<ip::tcp::socket> socket);

  void OnRead(beast::error_code ec, std::size_t /*bytes_transferred*/,
              std::shared_ptr<ip::tcp::socket> socket,
              std::shared_ptr<beast::flat_buffer> buffer,
              std::shared_ptr<http::request<http::string_body>> request);

  void OnWrite(beast::error_code ec, std::size_t /*bytes_transferred*/,
               std::shared_ptr<ip::tcp::socket> socket,
               std::shared_ptr<http::response<http::string_body>> response,
               bool keep_alive);

  std::string host_;
  std::uint16_t port_;

  net::io_context io_context_;
  ip::tcp::acceptor acceptor_;
  std::size_t thread_count_;
  std::unique_ptr<net::thread_pool> thread_pool_;
  std::vector<std::thread> workers_;
  std::optional<net::executor_work_guard<net::io_context::executor_type>>
      work_guard_;

  std::shared_ptr<Router> router_;
};
