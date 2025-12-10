#pragma once
#include <string>

class User {
 public:
  User(std::string uuid, std::string name, std::string password);
  User(std::string name, std::string password);

  bool ComparePassword(const std::string& password) const;

  const std::string& GetId() const { return uuid_; }
  const std::string& GetName() const { return name_; }
  const std::string& GetPassword() const { return password_; }

 private:
  std::string uuid_{0};
  std::string name_;
  std::string password_;
};