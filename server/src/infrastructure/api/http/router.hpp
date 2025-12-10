#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <boost/beast/http.hpp>

#include "handlers/handler.hpp"

class Router {
 public:
  virtual ~Router() = default;

  virtual void Add(std::shared_ptr<Handler> handler) = 0;

  virtual http::response<http::string_body> Dispatch(
      const http::request<http::string_body>& req) const = 0;
};

class SimpleRouter final : public Router {
 public:
  void Add(std::shared_ptr<Handler> handler) override;

  http::response<http::string_body> Dispatch(
      const http::request<http::string_body>& req) const override;
  
 private:
  std::unordered_map<std::string, std::shared_ptr<Handler>>
      endpoint_to_handler_;
};
