#include "user.hpp"

User::User(std::string uuid, std::string name, std::string password) 
: uuid_(uuid), name_(name), password_(password) {}

User::User(std::string name, std::string password)
    : name_(std::move(name)), password_(std::move(password)) {}

bool User::ComparePassword(const std::string& password) const {
  return password_ == password;
}