#pragma once

#include <boost/beast/http.hpp>
#include <string>

#include "handler.hpp"
#include "task_service.hpp"

namespace http = boost::beast::http;

class CreateTaskHandler : public Handler {
 public:
  explicit CreateTaskHandler(std::shared_ptr<TaskService> task_service)
      : task_service_(std::move(task_service)) {}
  std::string GetMethodEndpoint() const override { return "POST /task"; }
  http::response<http::string_body> Execute(
      const http::request<http::string_body>& req) override;
 private:
  std::shared_ptr<TaskService> task_service_;
};

class ChangeStatusTaskHandler : public Handler {
 public:
  explicit ChangeStatusTaskHandler(std::shared_ptr<TaskService> task_service)
      : task_service_(std::move(task_service)) {}
  std::string GetMethodEndpoint() const override { return "PATCH /task/status"; }
  http::response<http::string_body> Execute(
      const http::request<http::string_body>& req) override;
 private:
  std::shared_ptr<TaskService> task_service_;
};

class DeleteTaskHandler : public Handler {
 public:
  explicit DeleteTaskHandler(std::shared_ptr<TaskService> task_service)
      : task_service_(std::move(task_service)) {}
  std::string GetMethodEndpoint() const override { return "DELETE /task/delete"; }
  http::response<http::string_body> Execute(
      const http::request<http::string_body>& req) override;
 private:
  std::shared_ptr<TaskService> task_service_;
};

class GetAllTasksHandler : public Handler {
 public:
  explicit GetAllTasksHandler(std::shared_ptr<TaskService> task_service)
      : task_service_(std::move(task_service)) {}
  std::string GetMethodEndpoint() const override { return "GET /task/getByUser"; }
  http::response<http::string_body> Execute(
      const http::request<http::string_body>& req) override;
 private:
  std::shared_ptr<TaskService> task_service_;
};

class GetOneTaskHandler : public Handler {
 public:
  explicit GetOneTaskHandler(std::shared_ptr<TaskService> task_service)
      : task_service_(std::move(task_service)) {}
  std::string GetMethodEndpoint() const override { return "GET /task/get"; }
  http::response<http::string_body> Execute(
      const http::request<http::string_body>& req) override;
 private:
  std::shared_ptr<TaskService> task_service_;
};
