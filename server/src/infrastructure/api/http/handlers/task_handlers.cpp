#include "task_handlers.hpp"

#include <nlohmann/json.hpp>
#include <sstream>
#include <string_view>

namespace http = boost::beast::http;

http::response<http::string_body> OkJson(const std::string& body) {
  http::response<http::string_body> resp;
  resp.result(http::status::ok);
  resp.set(http::field::content_type, "application/json");
  resp.body() = body;
  resp.prepare_payload();
  return resp;
}

http::response<http::string_body> ErrorJson(http::status status,
                                            std::string_view code) {
  http::response<http::string_body> resp;
  resp.result(status);
  resp.set(http::field::content_type, "application/json");
  resp.body() = std::string("{\"error\":\"") + std::string(code) + "}";
  resp.prepare_payload();
  return resp;
}

std::string GetQueryParam(const std::string& target, const std::string& key) {
  auto pos = target.find('?');
  if (pos == std::string::npos)
    return {};
  std::string q = target.substr(pos + 1);
  std::istringstream iss(q);
  std::string kv;
  while (std::getline(iss, kv, '&')) {
    auto eq = kv.find('=');
    if (eq != std::string::npos) {
      auto k = kv.substr(0, eq);
      auto v = kv.substr(eq + 1);
      if (k == key)
        return v;
    }
  }
  return {};
}

http::response<http::string_body> CreateTaskHandler::Execute(
    const http::request<http::string_body>& req) {
  using nlohmann::json;
  try {
    const auto body = json::parse(req.body());
    const auto user_id = body.at("user_id").get<std::string>();
    const auto title = body.at("title").get<std::string>();
    const auto description = body.at("description").get<std::string>();
    const auto status = body.at("status").get<std::string>();

    if (status != "InProgress" && status != "InProject" && status != "Done") {
      return ErrorJson(http::status::bad_request, "invalid_status");
    }

    auto r = task_service_->CreateTask(user_id, title, description, status);
    if (!r.has_value() && (r.error() == AddTaskError::AlreadyExists))
      return ErrorJson(http::status::conflict, "task exist");
    if (!r.has_value())
      return ErrorJson(http::status::internal_server_error, "create_failed");
    json out;
    out["status"] = "ok";
    out["id"] = r->GetId();
    return OkJson(out.dump());
  } catch (...) {
    return ErrorJson(http::status::bad_request, "invalid_json");
  }
}

http::response<http::string_body> ChangeStatusTaskHandler::Execute(
    const http::request<http::string_body>& req) {
  using nlohmann::json;
  try {
    const auto body = json::parse(req.body());
    const auto id = body.at("id").get<std::string>();
    const auto status = body.at("status").get<std::string>();

    if (status != "InProgress" && status != "InProject" && status != "Done") {
      return ErrorJson(http::status::bad_request, "invalid_status");
    }

    auto r = task_service_->ChangeStatus(id, status);
    if (!r.has_value() && (r.error() == UpdateTaskError::NotFound))
      return ErrorJson(http::status::not_found, "update_failed");
    if (!r.has_value())
      return ErrorJson(http::status::bad_request, "update_failed");
    json out;
    out["status"] = "ok";
    out["id"] = r->GetId();
    out["new_status"] = r->GetStatus();
    return OkJson(out.dump());
  } catch (...) {
    return ErrorJson(http::status::bad_request, "invalid_json");
  }
}

http::response<http::string_body> DeleteTaskHandler::Execute(
    const http::request<http::string_body>& req) {
  const auto id = GetQueryParam(std::string(req.target()), "id");
  if (id.empty())
    return ErrorJson(http::status::bad_request, "missing_id");

  try {
    auto r = task_service_->DeleteTask(id);
    if (!r.has_value() && (r.error() == DeleteTaskError::NotFound))
      return ErrorJson(http::status::not_found, "delete_failed");
    if (!r.has_value())
      return ErrorJson(http::status::bad_request, "delete_failed");
    return OkJson(R"({"status": "ok"})");
  } catch (...) {
    return ErrorJson(http::status::bad_request, "invalid_json");
  }
}

http::response<http::string_body> GetAllTasksHandler::Execute(
    const http::request<http::string_body>& req) {
  const std::string user_id =
      GetQueryParam(std::string(req.target()), "user_id");
  if (user_id.empty())
    return ErrorJson(http::status::bad_request, "missing_user_id");
  auto r = task_service_->GetUserTasks(user_id);
  if (!r.has_value())
    return ErrorJson(http::status::bad_request, "get_failed");

  nlohmann::json out;
  out["tasks"] = nlohmann::json::array();
  for (const auto& t : r.value()) {
    out["tasks"].push_back({
        {"id", t.GetId()},
        {"user_id", t.GetUserId()},
        {"title", t.GetTitle()},
        {"description", t.GetDescription()},
        {"status", t.GetStatus()},
    });
  }
  return OkJson(out.dump());
}

http::response<http::string_body> GetOneTaskHandler::Execute(
    const http::request<http::string_body>& req) {
  const std::string id = GetQueryParam(std::string(req.target()), "id");
  if (id.empty())
    return ErrorJson(http::status::bad_request, "missing_id");
  auto r = task_service_->GetTask(id);
  if (!r.has_value())
    return ErrorJson(http::status::not_found, "not_found");
  const auto& t = r.value();
  nlohmann::json out{{"id", t.GetId()},
                     {"user_id", t.GetUserId()},
                     {"title", t.GetTitle()},
                     {"description", t.GetDescription()},
                     {"status", t.GetStatus()}};
  return OkJson(out.dump());
}
