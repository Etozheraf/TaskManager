#pragma once
#include <expected>
#include <string>
#include <vector>

#include "task.hpp"
#include "errors.hpp"

class TaskRepository {
 public:
  TaskRepository() = default;
  virtual std::expected<Task, AddTaskError> AddTask(const Task& task) = 0;
  virtual std::expected<Task, FindTaskError> GetTaskById(const std::string& task_id) = 0;
  virtual std::expected<std::vector<Task>, FindTaskError> GetTasksByUser(const std::string& user_id) = 0;
  virtual std::expected<Task, UpdateTaskError> UpdateTaskStatus(const std::string& task_id, const std::string& status) = 0;
  virtual std::expected<void, DeleteTaskError> DeleteTaskById(const std::string& task_id) = 0;
  virtual ~TaskRepository() = 0;
};

inline TaskRepository::~TaskRepository() = default;


