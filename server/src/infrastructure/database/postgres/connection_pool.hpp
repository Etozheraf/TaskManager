#pragma once

#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <pqxx/pqxx>
#include <queue>
#include <string>

class PqxxConnectionPool
    : public std::enable_shared_from_this<PqxxConnectionPool> {
 public:
  explicit PqxxConnectionPool(std::string connection_string,
                              std::size_t pool_size);

  struct ConnectionHolder {
    ConnectionHolder(std::shared_ptr<PqxxConnectionPool> pool);
    ConnectionHolder(ConnectionHolder&&) noexcept = default;
    ConnectionHolder& operator=(ConnectionHolder&&) noexcept = default;
    ConnectionHolder(const ConnectionHolder&) = delete;
    ConnectionHolder& operator=(const ConnectionHolder&) = delete;
    ~ConnectionHolder();

    std::unique_ptr<pqxx::connection> connection;

   private:
    std::shared_ptr<PqxxConnectionPool> pool_;
  };

  ConnectionHolder AcquireHolder();
  void Release(std::unique_ptr<pqxx::connection> connection);

 private:
  std::string connection_string_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::queue<std::unique_ptr<pqxx::connection>> pool_;
  std::size_t capacity_;
  std::size_t created_;
};
