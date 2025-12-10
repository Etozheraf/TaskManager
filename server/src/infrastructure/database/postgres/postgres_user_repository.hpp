#pragma once

#include <expected>
#include <pqxx/pqxx>
#include <string>

#include "user_repository.hpp"
#include "connection_pool.hpp"

class PostgreSQLUserRepository : public UserRepository {
 public:
  explicit PostgreSQLUserRepository(std::shared_ptr<PqxxConnectionPool> pool);
  ~PostgreSQLUserRepository() override = default;

  std::expected<User, FindUserError> FindUser(
      const std::string& login) override;
  std::expected<User, AddUserError> AddUser(User user) override;

 private:
  std::shared_ptr<PqxxConnectionPool> pool_;
};
