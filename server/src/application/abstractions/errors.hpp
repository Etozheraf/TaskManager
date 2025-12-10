#pragma once

enum class FindUserError {
  NotFound,
  RepositoryError,
};

enum class AddUserError {
  AlreadyExists,
  RepositoryError,
};

enum class FindTaskError {
  NotFound,
  RepositoryError,
};

enum class AddTaskError {
  AlreadyExists,
  RepositoryError,
};

enum class UpdateTaskError {
  NotFound,
  RepositoryError,
};

enum class DeleteTaskError {
  NotFound,
  RepositoryError,
};