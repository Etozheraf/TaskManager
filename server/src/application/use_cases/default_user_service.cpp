#include "default_user_service.hpp"

std::expected<User, RegistrationError> DefaultUserService::Registration(
    const std::string& login, const std::string& password) {
  auto user = user_repository_->FindUser(login);
  if (user.has_value()) {
    return std::unexpected(RegistrationError::NameAlreadyExists);
  }
  if (user.error() == FindUserError::RepositoryError) {
    return std::unexpected(RegistrationError::InternalError);
  }

  auto result = user_repository_->AddUser(User{login, password});
  if (result.has_value()) {
    return result.value();
  }
  return std::unexpected(RegistrationError::InternalError);
}

std::expected<User, LoginError> DefaultUserService::Login(
    const std::string& login, const std::string& password) {
  auto user = user_repository_->FindUser(login);
  if (!user.has_value() && (user.error() == FindUserError::RepositoryError)) {
    return std::unexpected(LoginError::InternalError);
  }
  if (!user.has_value() && (user.error() == FindUserError::NotFound)) {
    return std::unexpected(LoginError::UserNotFound);
  }

  if (!user.value().ComparePassword(password)) {
    return std::unexpected(LoginError::WrongPassword);
  }
  return user.value();
}