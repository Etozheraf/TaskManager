#pragma once

#include <memory>

#include "errors.hpp"
#include "user_repository.hpp"
#include "user_service.hpp"

class DefaultUserService final : public UserService {
 public:
  explicit DefaultUserService(std::unique_ptr<UserRepository> user_repository)
      : user_repository_(std::move(user_repository)) {}

  std::expected<User, RegistrationError> Registration(
      const std::string& login, const std::string& password) override;

  std::expected<User, LoginError> Login(const std::string& login,
                                        const std::string& password) override;
  ~DefaultUserService() override = default;

 private:
  std::unique_ptr<UserRepository> user_repository_;
};