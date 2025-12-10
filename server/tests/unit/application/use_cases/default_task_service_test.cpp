#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <expected>
#include <memory>
#include <vector>

#include "default_task_service.hpp"
#include "mocks/mock_task_repository.hpp"

using ::testing::_;
using ::testing::Return;

class DefaultTaskServiceTest : public ::testing::Test {
 protected:
  MockTaskRepository* repo_{};
  std::unique_ptr<DefaultTaskService> service_;

  void SetUp() override {
    auto repo_ptr = std::make_unique<MockTaskRepository>();
    repo_ = repo_ptr.get();
    service_ = std::make_unique<DefaultTaskService>(std::move(repo_ptr));
  }
};

TEST_F(DefaultTaskServiceTest, Create_Success) {
  EXPECT_CALL(*repo_, AddTask(_))
      .WillOnce(Return(Task{"id-1", "u1", "title", "desc", "new"}));

  auto r = service_->CreateTask("u1", "title", "desc", "new");

  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->GetId(), "id-1");
}

TEST_F(DefaultTaskServiceTest, ChangeStatus_Success) {
  EXPECT_CALL(*repo_, UpdateTaskStatus("id-1", "done"))
      .WillOnce(Return(Task{"id-1", "u1", "title", "desc", "done"}));

  auto r = service_->ChangeStatus("id-1", "done");

  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->GetStatus(), "done");
}

TEST_F(DefaultTaskServiceTest, Delete_Success) {
  EXPECT_CALL(*repo_, DeleteTaskById("id-1"))
      .WillOnce(Return(std::expected<void, DeleteTaskError>{}));

  auto r = service_->DeleteTask("id-1");

  ASSERT_TRUE(r.has_value());
}

TEST_F(DefaultTaskServiceTest, GetTask_Success) {
  EXPECT_CALL(*repo_, GetTaskById("id-1"))
      .WillOnce(Return(Task{"id-1", "u1", "t", "d", "s"}));

  auto r = service_->GetTask("id-1");

  ASSERT_TRUE(r.has_value());
  EXPECT_EQ(r->GetId(), "id-1");
  EXPECT_EQ(r->GetUserId(), "u1");
}

TEST_F(DefaultTaskServiceTest, GetTask_NotFound) {
  EXPECT_CALL(*repo_, GetTaskById("missing"))
      .WillOnce(Return(std::unexpected(FindTaskError::NotFound)));

  auto r = service_->GetTask("missing");

  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), FindTaskError::NotFound);
}

TEST_F(DefaultTaskServiceTest, GetUserTasks_Success) {
  EXPECT_CALL(*repo_, GetTasksByUser("u1"))
      .WillOnce(
          Return(std::vector<Task>{Task{"id-1", "u1", "t1", "d1", "Done"},
                                   Task{"id-2", "u1", "t2", "d2", "Done"}}));

  auto r = service_->GetUserTasks("u1");

  ASSERT_TRUE(r.has_value());
  ASSERT_EQ(r->size(), 2);
  EXPECT_EQ(r->at(0).GetId(), "id-1");
  EXPECT_EQ(r->at(1).GetId(), "id-2");
}

TEST_F(DefaultTaskServiceTest, GetUserTasks_Empty) {
  EXPECT_CALL(*repo_, GetTasksByUser("u1"))
      .WillOnce(Return(std::vector<Task>{}));

  auto r = service_->GetUserTasks("u1");

  ASSERT_TRUE(r.has_value());
  EXPECT_TRUE(r->empty());
}

TEST_F(DefaultTaskServiceTest, Create_Fail) {
  EXPECT_CALL(*repo_, AddTask(_))
      .WillOnce(Return(std::unexpected(AddTaskError::RepositoryError)));

  auto r = service_->CreateTask("u1", "title", "desc", "new");

  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), AddTaskError::RepositoryError);
}

TEST_F(DefaultTaskServiceTest, ChangeStatus_NotFound) {
  EXPECT_CALL(*repo_, UpdateTaskStatus("missing", "done"))
      .WillOnce(Return(std::unexpected(UpdateTaskError::NotFound)));

  auto r = service_->ChangeStatus("missing", "done");

  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), UpdateTaskError::NotFound);
}

TEST_F(DefaultTaskServiceTest, ChangeStatus_RepositoryError) {
  EXPECT_CALL(*repo_, UpdateTaskStatus("id-1", "done"))
      .WillOnce(Return(std::unexpected(UpdateTaskError::RepositoryError)));

  auto r = service_->ChangeStatus("id-1", "done");

  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), UpdateTaskError::RepositoryError);
}

TEST_F(DefaultTaskServiceTest, Delete_NotFound) {
  EXPECT_CALL(*repo_, DeleteTaskById("missing"))
      .WillOnce(Return(std::unexpected(DeleteTaskError::NotFound)));

  auto r = service_->DeleteTask("missing");

  ASSERT_FALSE(r.has_value());
  EXPECT_EQ(r.error(), DeleteTaskError::NotFound);
}
