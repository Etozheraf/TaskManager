#include "router.hpp"

#include <boost/log/trivial.hpp>

#include "handlers/handler.hpp"

void SimpleRouter::Add(std::shared_ptr<Handler> handler) {
  const std::string endpoint = handler->GetMethodEndpoint();
  const bool existed =
      endpoint_to_handler_.find(endpoint) != endpoint_to_handler_.end();
  endpoint_to_handler_[endpoint] = std::move(handler);
  if (existed) {
    BOOST_LOG_TRIVIAL(warning) << "Handler replaced: " << endpoint;
  } else {
    BOOST_LOG_TRIVIAL(info) << "Handler registered: " << endpoint;
  }
}

http::response<http::string_body> SimpleRouter::Dispatch(
    const http::request<http::string_body>& req) const {
  const std::string key =
      std::string(http::to_string(req.method())) + " " +
      std::string(req.target().substr(0, req.target().find('?')));

  auto it = endpoint_to_handler_.find(key);
  if (it == endpoint_to_handler_.end()) {
    http::response<http::string_body> response;
    response.result(http::status::not_found);
    response.set(http::field::content_type, "application/json");
    response.body() = R"({"error":"not_found"})";
    response.prepare_payload();
    return response;
  }
  try {
    return it->second->Execute(req);
  } catch (const std::exception& ex) {
    BOOST_LOG_TRIVIAL(error) << ex.what();
    http::response<http::string_body> response;
    response.result(http::status::internal_server_error);
    response.set(http::field::content_type, "application/json");
    response.body() = R"({"error":"internal"})";
    response.prepare_payload();
    return response;
  }
}
