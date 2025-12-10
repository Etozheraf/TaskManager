#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <expected>
#include <memory>

#include "default_user_service.hpp"
#include "mocks/mock_user_repository.hpp"

using ::testing::_;
using ::testing::Return;

class DefaultUserServiceTest : public ::testing::Test {
 protected:
  MockUserRepository* repo_{};  // raw pointer на мок
  std::unique_ptr<DefaultUserService> service_;

  void SetUp() override {
    auto repo_ptr = std::make_unique<MockUserRepository>();
    repo_ = repo_ptr.get();
    service_ = std::make_unique<DefaultUserService>(std::move(repo_ptr));
  }
};

TEST_F(DefaultUserServiceTest, Registration_Success) {
  EXPECT_CALL(*repo_, FindUser("alice"))
      .WillOnce(Return(std::unexpected(FindUserError::NotFound)));
  EXPECT_CALL(*repo_, AddUser(_))
      .WillOnce(Return(User{"id-1", "alice", "pwd"}));

  auto r = service_->Registration("alice", "pwd");

  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->GetId(), "id-1");
  EXPECT_EQ(r->GetName(), "alice");
  EXPECT_EQ(r->GetPassword(), "pwd");
}

TEST_F(DefaultUserServiceTest, Registration_Duplicate) {
  EXPECT_CALL(*repo_, FindUser("alice"))
      .WillOnce(Return(User{"id-1", "alice", "pwd"}));

  auto r = service_->Registration("alice", "pwd");

  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), RegistrationError::NameAlreadyExists);
}

TEST_F(DefaultUserServiceTest, Registration_RepositoryFail) {
  EXPECT_CALL(*repo_, FindUser("alice"))
      .WillOnce(Return(std::unexpected(FindUserError::RepositoryError)));

  auto r = service_->Registration("alice", "pwd");

  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), RegistrationError::InternalError);
}

TEST_F(DefaultUserServiceTest, Login_Ok) {
  EXPECT_CALL(*repo_, FindUser("alice"))
      .WillOnce(Return(User{"id-1", "alice", "pwd"}));

  auto r = service_->Login("alice", "pwd");

  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->GetId(), "id-1");
  EXPECT_EQ(r->GetName(), "alice");
  EXPECT_EQ(r->GetPassword(), "pwd");
}

TEST_F(DefaultUserServiceTest, Login_NotFound) {
  EXPECT_CALL(*repo_, FindUser("alice"))
      .WillOnce(Return(std::unexpected(FindUserError::NotFound)));

  auto r = service_->Login("alice", "pwd");

  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), LoginError::UserNotFound);
}

TEST_F(DefaultUserServiceTest, Login_WrongPassword) {
  EXPECT_CALL(*repo_, FindUser("alice"))
      .WillOnce(Return(User{"id-1", "alice", "pwd"}));

  auto r = service_->Login("alice", "bad");

  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), LoginError::WrongPassword);
}

TEST_F(DefaultUserServiceTest, Login_RepositoryFail) {
  EXPECT_CALL(*repo_, FindUser("alice"))
      .WillOnce(Return(std::unexpected(FindUserError::RepositoryError)));

  auto r = service_->Login("alice", "bad");

  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), LoginError::InternalError);
}