#pragma once

#include <expected>
#include <pqxx/pqxx>
#include <string>
#include <vector>

#include "task_repository.hpp"
#include "connection_pool.hpp"

class PostgreSQLTaskRepository : public TaskRepository {
 public:
  explicit PostgreSQLTaskRepository(std::shared_ptr<PqxxConnectionPool> pool);
  ~PostgreSQLTaskRepository() override = default;

  std::expected<Task, AddTaskError> AddTask(const Task& task) override;
  std::expected<Task, FindTaskError> GetTaskById(
      const std::string& task_id) override;
  std::expected<std::vector<Task>, FindTaskError> GetTasksByUser(
      const std::string& user_id) override;
  std::expected<Task, UpdateTaskError> UpdateTaskStatus(
      const std::string& task_id, const std::string& status) override;
  std::expected<void, DeleteTaskError> DeleteTaskById(
      const std::string& task_id) override;

 private:
  std::shared_ptr<PqxxConnectionPool> pool_;
};
