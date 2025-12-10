#pragma once

#include <gmock/gmock.h>
#include <expected>

#include "user_service.hpp"

class MockUserService : public UserService {
 public:
  using RegistrationResult = std::expected<User, RegistrationError>;
  using LoginResult = std::expected<User, LoginError>;

  MOCK_METHOD(RegistrationResult, Registration, (const std::string& login, const std::string& password), (override));
  MOCK_METHOD(LoginResult, Login, (const std::string& login, const std::string& password), (override));
  ~MockUserService() override = default;
};


