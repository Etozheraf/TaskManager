#pragma once
#include <string>

class Task {
 public:
  Task(std::string id,
       std::string user_id,
       std::string title,
       std::string description,
       std::string status)
      : id_(std::move(id)),
        user_id_(std::move(user_id)),
        title_(std::move(title)),
        description_(std::move(description)),
        status_(std::move(status)) {}

  Task(std::string user_id,
       std::string title,
       std::string description,
       std::string status)
      : user_id_(std::move(user_id)),
        title_(std::move(title)),
        description_(std::move(description)),
        status_(std::move(status)) {}

  const std::string& GetId() const { return id_; }
  const std::string& GetUserId() const { return user_id_; }
  const std::string& GetTitle() const { return title_; }
  const std::string& GetDescription() const { return description_; }
  const std::string& GetStatus() const { return status_; }

 private:
  std::string id_{};
  std::string user_id_{};
  std::string title_{};
  std::string description_{};
  std::string status_{};
};


