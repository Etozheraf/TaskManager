#include "postgres_task_repository.hpp"
#include <boost/log/trivial.hpp>

PostgreSQLTaskRepository::PostgreSQLTaskRepository(
    std::shared_ptr<PqxxConnectionPool> pool)
    : pool_(std::move(pool)) {}

std::expected<Task, AddTaskError> PostgreSQLTaskRepository::AddTask(
    const Task& task) {
  try {
    auto holder = pool_->AcquireHolder();
    pqxx::work txn(*holder.connection);
    auto r = txn.exec(
        "INSERT INTO tasks (user_id, title, description, status) VALUES "
        "($1,$2,$3,$4) RETURNING id",
        pqxx::params{task.GetUserId(), task.GetTitle(), task.GetDescription(),
                     task.GetStatus()});
    if (r.empty()) {
      return std::unexpected(AddTaskError::RepositoryError);
    }
    std::string id = r[0][0].as<std::string>();
    txn.commit();
    return Task{id, task.GetUserId(), task.GetTitle(), task.GetDescription(),
                task.GetStatus()};
  } catch (const pqxx::unique_violation&) {
    return std::unexpected(AddTaskError::AlreadyExists);
  } catch (...) {
    return std::unexpected(AddTaskError::RepositoryError);
  }
}

std::expected<Task, FindTaskError> PostgreSQLTaskRepository::GetTaskById(
    const std::string& task_id) {
  try {
    auto holder = pool_->AcquireHolder();
    pqxx::work txn(*holder.connection);
    auto r = txn.exec(
        "SELECT id, user_id, title, description, status FROM tasks WHERE id=$1",
        pqxx::params{task_id});
    if (r.empty())
      return std::unexpected(FindTaskError::NotFound);
    auto row = r[0];
    return Task{row[0].as<std::string>(), row[1].as<std::string>(),
                row[2].as<std::string>(), row[3].as<std::string>(),
                row[4].as<std::string>()};
  } catch (...) {
    return std::unexpected(FindTaskError::RepositoryError);
  }
}

std::expected<std::vector<Task>, FindTaskError>
PostgreSQLTaskRepository::GetTasksByUser(const std::string& user_id) {
  try {
    auto holder = pool_->AcquireHolder();
    pqxx::work txn(*holder.connection);
    auto r = txn.exec(
        "SELECT id, user_id, title, description, status FROM tasks WHERE "
        "user_id=$1 ORDER BY id",
        pqxx::params{user_id});
    std::vector<Task> tasks;
    tasks.reserve(r.size());
    for (const auto& row : r) {
      tasks.emplace_back(row[0].as<std::string>(), row[1].as<std::string>(),
                         row[2].as<std::string>(), row[3].as<std::string>(),
                         row[4].as<std::string>());
    }
    return tasks;
  } catch (...) {
    return std::unexpected(FindTaskError::RepositoryError);
  }
}

std::expected<Task, UpdateTaskError> PostgreSQLTaskRepository::UpdateTaskStatus(
    const std::string& task_id, const std::string& status) {
  try {
    auto holder = pool_->AcquireHolder();
    pqxx::work txn(*holder.connection);
    auto r = txn.exec(
        "UPDATE tasks SET status=$2 WHERE id=$1 RETURNING id, user_id, title, "
        "description, status",
        pqxx::params{task_id, status});
    if (r.empty())
      return std::unexpected(UpdateTaskError::NotFound);
    auto row = r[0];
    txn.commit();
    return Task{row[0].as<std::string>(), row[1].as<std::string>(),
                row[2].as<std::string>(), row[3].as<std::string>(),
                row[4].as<std::string>()};
  } catch (...) {
    return std::unexpected(UpdateTaskError::RepositoryError);
  }
}

std::expected<void, DeleteTaskError> PostgreSQLTaskRepository::DeleteTaskById(
    const std::string& task_id) {
  try {
    auto holder = pool_->AcquireHolder();
    pqxx::work txn(*holder.connection);
    auto r = txn.exec("DELETE FROM tasks WHERE id=$1", pqxx::params{task_id});
    if (r.affected_rows() == 0)
      return std::unexpected(DeleteTaskError::NotFound);
    txn.commit();
    return {};
  } catch (...) {
    return std::unexpected(DeleteTaskError::RepositoryError);
  }
}
