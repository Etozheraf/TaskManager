#pragma once

#include <boost/beast/http.hpp>
#include <string>

namespace http = boost::beast::http;

class Handler {
 public:
  virtual ~Handler() = default;
  virtual std::string GetMethodEndpoint() const = 0;
  virtual http::response<http::string_body> Execute(
      const http::request<http::string_body>& req) = 0;
};

