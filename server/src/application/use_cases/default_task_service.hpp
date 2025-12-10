#pragma once
#include <memory>
#include <expected>
#include <string>
#include <vector>

#include "task_repository.hpp"
#include "task_service.hpp"


class DefaultTaskService final : public TaskService {
 public:
  explicit DefaultTaskService(std::unique_ptr<TaskRepository> task_repository)
      : task_repository_(std::move(task_repository)) {}

  std::expected<Task, AddTaskError> CreateTask(const std::string& user_id,
                                               const std::string& title,
                                               const std::string& description,
                                               const std::string& status) override;

  std::expected<Task, FindTaskError> GetTask(const std::string& task_id) override;
  std::expected<std::vector<Task>, FindTaskError> GetUserTasks(const std::string& user_id) override;
  std::expected<Task, UpdateTaskError> ChangeStatus(const std::string& task_id,
                                                    const std::string& status) override;
  std::expected<void, DeleteTaskError> DeleteTask(const std::string& task_id) override;

  ~DefaultTaskService() override = default;

 private:
  std::unique_ptr<TaskRepository> task_repository_;
};


