#pragma once

#include <gmock/gmock.h>
#include "handlers/handler.hpp"

class MockHandler : public Handler {
 public:
  MOCK_METHOD(std::string, GetMethodEndpoint, (), (const, override));
  MOCK_METHOD(http::response<http::string_body>, Execute,
              (const http::request<http::string_body>& req), (override));
};