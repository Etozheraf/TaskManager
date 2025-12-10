#pragma once
#include <gmock/gmock.h>
#include <expected>
#include <vector>

#include "task_repository.hpp"

class MockTaskRepository : public TaskRepository {
 public:
  using AddTaskResult = std::expected<Task, AddTaskError>;
  using GetTaskByIdResult = std::expected<Task, FindTaskError>;
  using GetTasksByUserResult = std::expected<std::vector<Task>, FindTaskError>;
  using UpdateTaskStatusResult = std::expected<Task, UpdateTaskError>;
  using DeleteTaskByIdResult = std::expected<void, DeleteTaskError>;

  MOCK_METHOD(AddTaskResult, AddTask, (const Task& task), (override));
  MOCK_METHOD(GetTaskByIdResult, GetTaskById, (const std::string& task_id), (override));
  MOCK_METHOD(GetTasksByUserResult, GetTasksByUser, (const std::string& user_id), (override));
  MOCK_METHOD(UpdateTaskStatusResult, UpdateTaskStatus, (const std::string& task_id, const std::string& status), (override));
  MOCK_METHOD(DeleteTaskByIdResult, DeleteTaskById, (const std::string& task_id), (override));
  ~MockTaskRepository() override = default;
};