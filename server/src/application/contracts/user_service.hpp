#pragma once
#include <expected>
#include <string>

#include "../../domain/user.hpp"
#include "errors.hpp"

class UserService {
 public:
  virtual std::expected<User, RegistrationError> Registration(
      const std::string& login, const std::string& password) = 0;

  virtual std::expected<User, LoginError> Login(const std::string& login,
                                                const std::string& password) = 0;
  virtual ~UserService() = default;
};