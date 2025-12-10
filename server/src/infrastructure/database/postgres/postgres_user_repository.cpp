#include "postgres_user_repository.hpp"
#include <boost/log/trivial.hpp>

PostgreSQLUserRepository::PostgreSQLUserRepository(
    std::shared_ptr<PqxxConnectionPool> pool)
    : pool_(std::move(pool)) {}

std::expected<User, FindUserError> PostgreSQLUserRepository::FindUser(
    const std::string& login) {
  try {
    auto holder = pool_->AcquireHolder();
    pqxx::work txn(*holder.connection);
    auto result =
        txn.exec("SELECT id, username, password FROM users WHERE username = $1",
                 pqxx::params{login});
    if (result.empty()) {
      return std::unexpected(FindUserError::NotFound);
    }
    auto row = result[0];
    std::string uuid = row[0].as<std::string>();
    std::string username = row[1].as<std::string>();
    std::string password = row[2].as<std::string>();
    return User{uuid, username, password};
  } catch (const std::exception&) {
    return std::unexpected(FindUserError::RepositoryError);
  }
}

std::expected<User, AddUserError> PostgreSQLUserRepository::AddUser(User user) {
  try {
    auto holder = pool_->AcquireHolder();
    pqxx::work txn(*holder.connection);
    auto result = txn.exec(
        "INSERT INTO users (username, password) VALUES ($1, $2) RETURNING id",
        pqxx::params{user.GetName(), user.GetPassword()});
    if (result.empty()) {
      return std::unexpected(AddUserError::RepositoryError);
    }
    std::string uuid = result[0][0].as<std::string>();
    txn.commit();
    return User{uuid, user.GetName(), user.GetPassword()};
  } catch (const pqxx::unique_violation&) {
    return std::unexpected(AddUserError::AlreadyExists);
  } catch (const std::exception&) {
    return std::unexpected(AddUserError::RepositoryError);
  }
}
