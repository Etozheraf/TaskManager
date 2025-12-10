#pragma once

#include <gmock/gmock.h>
#include <expected>
#include <vector>

#include "../../../application/contracts/task_service.hpp"

class MockTaskService : public TaskService {
 public:
  using CreateTaskResult = std::expected<Task, AddTaskError>;
  using GetTaskResult = std::expected<Task, FindTaskError>;
  using GetUserTasksResult = std::expected<std::vector<Task>, FindTaskError>;
  using ChangeStatusResult = std::expected<Task, UpdateTaskError>;
  using DeleteTaskResult = std::expected<void, DeleteTaskError>;

  MOCK_METHOD(CreateTaskResult, CreateTask, (const std::string& user_id, const std::string& title, const std::string& description, const std::string& status), (override));
  MOCK_METHOD(GetTaskResult, GetTask, (const std::string& task_id), (override));
  MOCK_METHOD(GetUserTasksResult, GetUserTasks, (const std::string& user_id), (override));
  MOCK_METHOD(ChangeStatusResult, ChangeStatus, (const std::string& task_id, const std::string& status), (override));
  MOCK_METHOD(DeleteTaskResult, DeleteTask, (const std::string& task_id), (override));
  ~MockTaskService() override = default;
};


