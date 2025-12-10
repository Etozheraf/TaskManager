#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <boost/beast/http.hpp>
#include <memory>
#include <nlohmann/json.hpp>

#include "handlers/user_handlers.hpp"
#include "mocks/mock_user_service.hpp"

using ::testing::_;
using ::testing::Return;

namespace http = boost::beast::http;

class UserHandlersTest : public ::testing::Test {
 protected:
  MockUserService* service_{};
  std::unique_ptr<RegisterUserHandler> register_handler_;
  std::unique_ptr<LoginUserHandler> login_handler_;

  void SetUp() override {
    auto service_ptr = std::make_shared<MockUserService>();
    service_ = service_ptr.get();
    register_handler_ = std::make_unique<RegisterUserHandler>(service_ptr);
    login_handler_ = std::make_unique<LoginUserHandler>(service_ptr);
  }

  static http::request<http::string_body> MakeJsonReq(
      const std::string& method, const std::string& target,
      const nlohmann::json& body) {
    http::request<http::string_body> r;
    r.method(http::string_to_verb(method));
    r.target(target);
    r.body() = body.dump();
    r.prepare_payload();
    return r;
  }
};

TEST_F(UserHandlersTest, Register_Success) {
  EXPECT_CALL(*service_, Registration("alice", "pwd"))
      .WillOnce(Return(User{"id-1", "alice", "pwd"}));
  auto req = MakeJsonReq("POST", "/user/register",
                         {{"login", "alice"}, {"password", "pwd"}});

  auto resp = register_handler_->Execute(req);
  auto body = nlohmann::json::parse(resp.body());

  EXPECT_EQ(resp.result(), http::status::ok);
  EXPECT_EQ(body["id"], "id-1");
}

TEST_F(UserHandlersTest, Register_NameAlreadyExists) {
  EXPECT_CALL(*service_, Registration("alice", "pwd"))
      .WillOnce(Return(std::unexpected(RegistrationError::NameAlreadyExists)));
  auto req = MakeJsonReq("POST", "/user/register",
                         {{"login", "alice"}, {"password", "pwd"}});

  auto resp = register_handler_->Execute(req);

  EXPECT_EQ(resp.result(), http::status::conflict);
}

TEST_F(UserHandlersTest, Register_BadRequest) {
  auto req = MakeJsonReq("POST", "/user/register", {{"login", "alice"}});

  auto resp = register_handler_->Execute(req);

  EXPECT_EQ(resp.result(), http::status::bad_request);
}

TEST_F(UserHandlersTest, Register_InternalError) {
  EXPECT_CALL(*service_, Registration("alice", "pwd"))
      .WillOnce(Return(std::unexpected(RegistrationError::InternalError)));
  auto req = MakeJsonReq("POST", "/user/register",
                         {{"login", "alice"}, {"password", "pwd"}});

  auto resp = register_handler_->Execute(req);

  EXPECT_EQ(resp.result(), http::status::internal_server_error);
}

TEST_F(UserHandlersTest, Login_Success) {
  EXPECT_CALL(*service_, Login("alice", "pwd"))
      .WillOnce(Return(User{"id-1", "alice", "pwd"}));
  auto req = MakeJsonReq("POST", "/user/login",
                         {{"login", "alice"}, {"password", "pwd"}});

  auto resp = login_handler_->Execute(req);

  EXPECT_EQ(resp.result(), http::status::ok);
}

TEST_F(UserHandlersTest, Login_UserNotFound) {
  EXPECT_CALL(*service_, Login("alice", "pwd"))
      .WillOnce(Return(std::unexpected(LoginError::UserNotFound)));
  auto req = MakeJsonReq("POST", "/user/login",
                         {{"login", "alice"}, {"password", "pwd"}});

  auto resp = login_handler_->Execute(req);

  EXPECT_EQ(resp.result(), http::status::unauthorized);
}

TEST_F(UserHandlersTest, Login_WrongPassword) {
  EXPECT_CALL(*service_, Login("alice", "pwd"))
      .WillOnce(Return(std::unexpected(LoginError::WrongPassword)));
  auto req = MakeJsonReq("POST", "/user/login",
                         {{"login", "alice"}, {"password", "pwd"}});

  auto resp = login_handler_->Execute(req);

  EXPECT_EQ(resp.result(), http::status::unauthorized);
}

TEST_F(UserHandlersTest, Login_InternalError) {
  EXPECT_CALL(*service_, Login("alice", "pwd"))
      .WillOnce(Return(std::unexpected(LoginError::InternalError)));
  auto req = MakeJsonReq("POST", "/user/login",
                         {{"login", "alice"}, {"password", "pwd"}});

  auto resp = login_handler_->Execute(req);

  EXPECT_EQ(resp.result(), http::status::internal_server_error);
}

TEST_F(UserHandlersTest, Login_BadRequest) {
  auto req = MakeJsonReq("POST", "/user/login", {{"login", "alice"}});

  auto resp = login_handler_->Execute(req);

  EXPECT_EQ(resp.result(), http::status::bad_request);
}