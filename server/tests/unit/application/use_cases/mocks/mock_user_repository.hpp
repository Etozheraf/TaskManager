#pragma once

#include <gmock/gmock.h>
#include <expected>

#include "user_repository.hpp"

class MockUserRepository : public UserRepository {
 public:
  using FindUserResult = std::expected<User, FindUserError>;
  using AddUserResult = std::expected<User, AddUserError>;

  MOCK_METHOD(FindUserResult, FindUser, (const std::string& login), (override));
  MOCK_METHOD(AddUserResult, AddUser, (User user), (override));
  ~MockUserRepository() override = default;
};


