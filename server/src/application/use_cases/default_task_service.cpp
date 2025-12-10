#include "default_task_service.hpp"
#include <algorithm>

std::expected<Task, AddTaskError> DefaultTaskService::CreateTask(
    const std::string& user_id, const std::string& title,
    const std::string& description, const std::string& status) {
  return task_repository_->AddTask(Task{user_id, title, description, status});
}

std::expected<Task, FindTaskError> DefaultTaskService::GetTask(
    const std::string& task_id) {
  return task_repository_->GetTaskById(task_id);
}

std::expected<std::vector<Task>, FindTaskError>
DefaultTaskService::GetUserTasks(const std::string& user_id) {
  auto tasks = task_repository_->GetTasksByUser(user_id);
  if (!tasks) {
    return tasks;
  }

  std::sort(tasks->begin(), tasks->end(), [](const Task& a, const Task& b) {
    if (a.GetStatus() == b.GetStatus()) {
      return a.GetTitle() < b.GetTitle();
    }
    if (a.GetStatus() == "InProgress") {
      return true;
    }
    if (b.GetStatus() == "InProgress") {
      return false;
    }

    if (a.GetStatus() == "InProject") {
      return true;
    }

    return false;
  });

  return tasks;
}

std::expected<Task, UpdateTaskError> DefaultTaskService::ChangeStatus(
    const std::string& task_id, const std::string& status) {
  return task_repository_->UpdateTaskStatus(task_id, status);
}

std::expected<void, DeleteTaskError> DefaultTaskService::DeleteTask(
    const std::string& task_id) {
  return task_repository_->DeleteTaskById(task_id);
}
