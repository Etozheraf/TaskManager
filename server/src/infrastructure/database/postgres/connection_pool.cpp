#include "connection_pool.hpp"

#include <boost/log/trivial.hpp>

PqxxConnectionPool::ConnectionHolder::ConnectionHolder(
    std::shared_ptr<PqxxConnectionPool> pool)
    : pool_(pool) {}

PqxxConnectionPool::ConnectionHolder::~ConnectionHolder() {
  if (connection && pool_) {
    pool_->Release(std::move(connection));
  }
}

PqxxConnectionPool::PqxxConnectionPool(std::string connection_string,
                                       std::size_t pool_size)
    : connection_string_(std::move(connection_string)),
      capacity_(pool_size),
      created_(0) {
  if (pool_size == 0) {
    throw std::runtime_error("Pool size must be greater than 0");
  }
  try {
    auto connection = std::make_unique<pqxx::connection>(connection_string_);
    if (!connection->is_open()) {
      throw std::runtime_error("PostgreSQL connection is not open");
    }
    ++created_;
    BOOST_LOG_TRIVIAL(info) << "pqxx: new connection created";
    pool_.push(std::move(connection));
  } catch (const std::exception& e) {
    throw std::runtime_error(std::string("pqxx connect failed: ") + e.what());
  }
}

PqxxConnectionPool::ConnectionHolder PqxxConnectionPool::AcquireHolder() {
  ConnectionHolder holder(shared_from_this());
  {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !pool_.empty() || created_ < capacity_; });

    if (!pool_.empty()) {
      holder.connection = std::move(pool_.front());
      pool_.pop();
      return holder;
    }

    try {
      holder.connection =
          std::make_unique<pqxx::connection>(connection_string_);
      if (!holder.connection->is_open()) {
        throw std::runtime_error("PostgreSQL connection is not open");
      }
      ++created_;
      BOOST_LOG_TRIVIAL(info) << "pqxx: new connection created";
      return holder;
    } catch (const std::exception& e) {
      throw std::runtime_error(std::string("pqxx connect failed: ") + e.what());
    }
  }
}

void PqxxConnectionPool::Release(std::unique_ptr<pqxx::connection> connection) {
  if (connection == nullptr)
    return;
  std::lock_guard<std::mutex> lock(mutex_);
  pool_.push(std::move(connection));
  cv_.notify_one();
}