#pragma once
#include <expected>
#include <string>
#include <vector>

#include "../../domain/task.hpp"
#include "../abstractions/errors.hpp"

class TaskService {
 public:
  virtual std::expected<Task, AddTaskError> CreateTask(const std::string& user_id,
                                                       const std::string& title,
                                                       const std::string& description,
                                                       const std::string& status) = 0;

  virtual std::expected<Task, FindTaskError> GetTask(const std::string& task_id) = 0;
  virtual std::expected<std::vector<Task>, FindTaskError> GetUserTasks(const std::string& user_id) = 0;
  virtual std::expected<Task, UpdateTaskError> ChangeStatus(const std::string& task_id,
                                                            const std::string& status) = 0;
  virtual std::expected<void, DeleteTaskError> DeleteTask(const std::string& task_id) = 0;
  virtual ~TaskService() = default;
};


