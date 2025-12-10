#pragma once
#include <expected>

#include "user.hpp"
#include "errors.hpp"

class UserRepository {
 public:
  UserRepository() = default;
  virtual std::expected<User, FindUserError> FindUser(const std::string& login) = 0;
  virtual std::expected<User, AddUserError> AddUser(User) = 0;
  virtual ~UserRepository() = 0;
};

inline UserRepository::~UserRepository() = default;