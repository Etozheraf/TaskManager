#pragma once

#include <boost/beast/http.hpp>
#include <memory>
#include <string>

#include "user_service.hpp"
#include "handler.hpp"

namespace http = boost::beast::http;

class RegisterUserHandler : public Handler {
 public:
  explicit RegisterUserHandler(std::shared_ptr<UserService> user_service)
      : user_service_(std::move(user_service)) {}

  std::string GetMethodEndpoint() const override { return "POST /user/register"; }
  
  http::response<http::string_body> Execute(
      const http::request<http::string_body>& req) override;

 private:
  std::shared_ptr<UserService> user_service_;
};

class LoginUserHandler : public Handler {
 public:
  explicit LoginUserHandler(std::shared_ptr<UserService> user_service)
      : user_service_(std::move(user_service)) {}

  std::string GetMethodEndpoint() const override { return "POST /user/login"; }
  
  http::response<http::string_body> Execute(
      const http::request<http::string_body>& req) override;

 private:
  std::shared_ptr<UserService> user_service_;
};
