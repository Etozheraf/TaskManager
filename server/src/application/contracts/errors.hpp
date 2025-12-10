#pragma once

enum class RegistrationError {
  NameAlreadyExists,
  InternalError,
};

enum class LoginError {
  UserNotFound,
  WrongPassword,
  InternalError,
};