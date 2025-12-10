#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <boost/beast/http.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>

#include "handlers/task_handlers.hpp"
#include "mocks/mock_task_service.hpp"

using ::testing::_;
using ::testing::Return;

namespace http = boost::beast::http;

class TaskHandlersTest : public ::testing::Test {
 protected:
  MockTaskService* service_{};
  std::unique_ptr<CreateTaskHandler> create_task_handler_;
  std::unique_ptr<GetAllTasksHandler> get_user_tasks_handler_;
  std::unique_ptr<GetOneTaskHandler> get_task_handler_;
  std::unique_ptr<ChangeStatusTaskHandler> change_task_status_handler_;
  std::unique_ptr<DeleteTaskHandler> delete_task_handler_;

  void SetUp() override {
    auto service_ptr = std::make_shared<MockTaskService>();
    service_ = service_ptr.get();
    create_task_handler_ = std::make_unique<CreateTaskHandler>(service_ptr);
    get_user_tasks_handler_ = std::make_unique<GetAllTasksHandler>(service_ptr);
    get_task_handler_ = std::make_unique<GetOneTaskHandler>(service_ptr);
    change_task_status_handler_ =
        std::make_unique<ChangeStatusTaskHandler>(service_ptr);
    delete_task_handler_ = std::make_unique<DeleteTaskHandler>(service_ptr);
  }

  static http::request<http::string_body> MakeReq(const std::string& method,
                                                  const std::string& target,
                                                  const nlohmann::json& body) {
    http::request<http::string_body> r;
    r.method(http::string_to_verb(method));
    r.target(target);
    r.body() = body.dump();
    r.prepare_payload();
    return r;
  }

  static http::request<http::string_body> MakeReq(const std::string& method,
                                                  const std::string& target) {
    http::request<http::string_body> r;
    r.method(http::string_to_verb(method));
    r.target(target);
    return r;
  }
};

TEST_F(TaskHandlersTest, CreateTask_Success) {
  EXPECT_CALL(*service_, CreateTask("u1", "title", "desc", "InProject"))
      .WillOnce(Return(Task{"id-1", "u1", "title", "desc", "InProject"}));
  auto req = MakeReq("POST", "/task",
                     {{"user_id", "u1"},
                      {"title", "title"},
                      {"description", "desc"},
                      {"status", "InProject"}});

  auto resp = create_task_handler_->Execute(req);
  auto body = nlohmann::json::parse(resp.body());

  EXPECT_EQ(resp.result(), http::status::ok);
  EXPECT_EQ(body["id"], "id-1");
}

TEST_F(TaskHandlersTest, CreateTask_BadRequest) {
  auto req = MakeReq("POST", "/task", {{"title", "title"}});

  auto resp = create_task_handler_->Execute(req);

  EXPECT_EQ(resp.result(), http::status::bad_request);
}

TEST_F(TaskHandlersTest, CreateTask_InternalError) {
  EXPECT_CALL(*service_, CreateTask("u1", "title", "desc", "InProject"))
      .WillOnce(Return(std::unexpected(AddTaskError::RepositoryError)));
  auto req = MakeReq("POST", "/task",
                     {{"user_id", "u1"},
                      {"title", "title"},
                      {"description", "desc"},
                      {"status", "InProject"}});

  auto resp = create_task_handler_->Execute(req);

  EXPECT_EQ(resp.result(), http::status::internal_server_error);
}

TEST_F(TaskHandlersTest, GetUserTasks_Success) {
  EXPECT_CALL(*service_, GetUserTasks("u1"))
      .WillOnce(Return(
          std::vector<Task>{Task{"id-1", "u1", "t1", "d1", "InProject"},
                            Task{"id-2", "u1", "t2", "d2", "InProject"}}));
  auto req = MakeReq("GET", "/task/getByUser?user_id=u1");

  auto resp = get_user_tasks_handler_->Execute(req);
  auto body = nlohmann::json::parse(resp.body());

  EXPECT_EQ(resp.result(), http::status::ok);
  ASSERT_TRUE(body.contains("tasks"));
  ASSERT_EQ(body["tasks"].size(), 2);
  EXPECT_EQ(body["tasks"][0]["id"], "id-1");
  EXPECT_EQ(body["tasks"][1]["id"], "id-2");
}

TEST_F(TaskHandlersTest, GetUserTasks_Empty) {
  EXPECT_CALL(*service_, GetUserTasks("u1"))
      .WillOnce(Return(std::vector<Task>{}));
  auto req = MakeReq("GET", "/task/getByUser?user_id=u1");

  auto resp = get_user_tasks_handler_->Execute(req);
  auto body = nlohmann::json::parse(resp.body());

  EXPECT_EQ(resp.result(), http::status::ok);
  ASSERT_TRUE(body.contains("tasks"));
  EXPECT_TRUE(body["tasks"].empty());
}

TEST_F(TaskHandlersTest, GetTask_Success) {
  EXPECT_CALL(*service_, GetTask("id-1"))
      .WillOnce(Return(Task{"id-1", "u1", "t", "d", "InProject"}));
  auto req = MakeReq("GET", "/task/get?id=id-1");

  auto resp = get_task_handler_->Execute(req);
  auto body = nlohmann::json::parse(resp.body());

  EXPECT_EQ(resp.result(), http::status::ok);
  EXPECT_EQ(body["id"], "id-1");
  EXPECT_EQ(body["user_id"], "u1");
}

TEST_F(TaskHandlersTest, GetTask_NotFound) {
  EXPECT_CALL(*service_, GetTask("id-1"))
      .WillOnce(Return(std::unexpected(FindTaskError::NotFound)));
  auto req = MakeReq("GET", "/task/get?id=id-1");

  auto resp = get_task_handler_->Execute(req);

  EXPECT_EQ(resp.result(), http::status::not_found);
}

TEST_F(TaskHandlersTest, ChangeStatus_Success) {
  EXPECT_CALL(*service_, ChangeStatus("id-1", "Done"))
      .WillOnce(Return(Task{"id-1", "u1", "t", "d", "Done"}));
  auto req =
      MakeReq("PATCH", "/task/status", {{"id", "id-1"}, {"status", "Done"}});

  auto resp = change_task_status_handler_->Execute(req);
  auto body = nlohmann::json::parse(resp.body());

  EXPECT_EQ(resp.result(), http::status::ok);
  EXPECT_EQ(body["new_status"], "Done");
}

TEST_F(TaskHandlersTest, ChangeStatus_BadRequest) {
  auto req = MakeReq("PATCH", "/task/id-1/status", {});

  auto resp = change_task_status_handler_->Execute(req);

  EXPECT_EQ(resp.result(), http::status::bad_request);
}

TEST_F(TaskHandlersTest, ChangeStatus_NotFound) {
  EXPECT_CALL(*service_, ChangeStatus("id-1", "Done"))
      .WillOnce(Return(std::unexpected(UpdateTaskError::NotFound)));
  auto req =
      MakeReq("PATCH", "/task/status", {{"id", "id-1"}, {"status", "Done"}});

  auto resp = change_task_status_handler_->Execute(req);

  EXPECT_EQ(resp.result(), http::status::not_found);
}

TEST_F(TaskHandlersTest, ChangeStatus_InternalError) {
  EXPECT_CALL(*service_, ChangeStatus("id-1", "Done"))
      .WillOnce(Return(std::unexpected(UpdateTaskError::RepositoryError)));
  auto req = MakeReq("PATCH", "/task/status", {{"id", "id-1"}, {"status", "Done"}});

  auto resp = change_task_status_handler_->Execute(req);

  EXPECT_EQ(resp.result(), http::status::bad_request);
}

TEST_F(TaskHandlersTest, DeleteTask_Success) {
  EXPECT_CALL(*service_, DeleteTask("id-1"))
      .WillOnce(Return(std::expected<void, DeleteTaskError>{}));
  auto req = MakeReq("DELETE", "/task/delete?id=id-1");

  auto resp = delete_task_handler_->Execute(req);

  EXPECT_EQ(resp.result(), http::status::ok);
}

TEST_F(TaskHandlersTest, DeleteTask_NotFound) {
  EXPECT_CALL(*service_, DeleteTask("id-1"))
      .WillOnce(Return(std::unexpected(DeleteTaskError::NotFound)));
  auto req = MakeReq("DELETE", "/task/delete?id=id-1");

  auto resp = delete_task_handler_->Execute(req);

  EXPECT_EQ(resp.result(), http::status::not_found);
}

TEST_F(TaskHandlersTest, DeleteTask_InternalError) {
  EXPECT_CALL(*service_, DeleteTask("id-1"))
      .WillOnce(Return(std::unexpected(DeleteTaskError::RepositoryError)));
  auto req = MakeReq("DELETE", "/task/delete?id=id-1");

  auto resp = delete_task_handler_->Execute(req);

  EXPECT_EQ(resp.result(), http::status::bad_request);
}