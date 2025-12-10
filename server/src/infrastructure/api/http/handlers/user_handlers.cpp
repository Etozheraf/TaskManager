#include "user_handlers.hpp"

#include <nlohmann/json.hpp>

using nlohmann::json;
namespace http = boost::beast::http;

http::response<http::string_body> RegisterUserHandler::Execute(
    const http::request<http::string_body>& req) {
  http::response<http::string_body> resp;
  try {
    const auto body = json::parse(req.body());
    const auto login = body.at("login").get<std::string>();
    const auto password = body.at("password").get<std::string>();

    auto result = user_service_->Registration(login, password);
    if (!result.has_value() &&
        (result.error() == RegistrationError::NameAlreadyExists)) {
      resp.result(http::status::conflict);
      resp.body() = R"({"error":"registration_failed"})";
    } else if (!result.has_value()) {
      resp.result(http::status::internal_server_error);
      resp.body() = R"({"error":"registration_failed"})";
    } else {
      resp.result(http::status::ok);
      json j;
      j["status"] = "ok";
      j["id"] = result->GetId();
      resp.body() = j.dump();
    }
  } catch (const std::exception&) {
    resp.result(http::status::bad_request);
    resp.body() = R"({"error":"invalid_json"})";
  }
  resp.set(http::field::content_type, "application/json");
  resp.prepare_payload();
  return resp;
}

http::response<http::string_body> LoginUserHandler::Execute(
    const http::request<http::string_body>& req) {
  http::response<http::string_body> resp;
  try {
    const auto body = json::parse(req.body());
    const auto login = body.at("login").get<std::string>();
    const auto password = body.at("password").get<std::string>();

    auto result = user_service_->Login(login, password);
    if (result.has_value()) {
      resp.result(http::status::ok);
      json j;
      j["status"] = "ok";
      j["id"] = result->GetId();
      resp.body() = j.dump();
    } else if (result.error() == LoginError::UserNotFound || result.error() == LoginError::WrongPassword) {
      resp.result(http::status::unauthorized);
      resp.body() = R"({"error":"login_failed"})";
    } else {
      resp.result(http::status::internal_server_error);
      resp.body() = R"({"error":"login_failed"})";
    }
  } catch (const std::exception&) {
    resp.result(http::status::bad_request);
    resp.body() = R"({"error":"invalid_json"})";
  }
  resp.set(http::field::content_type, "application/json");
  resp.prepare_payload();
  return resp;
}
