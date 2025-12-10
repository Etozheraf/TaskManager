#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <boost/beast/http.hpp>

#include "router.hpp"
#include "mocks/mock_handler.hpp"

namespace http = boost::beast::http;
using ::testing::NiceMock;
using ::testing::Return;

class RouterTest : public ::testing::Test {
 protected:
  static http::request<http::string_body> MakeReq(const std::string& method,
                                                  const std::string& target) {
    http::request<http::string_body> r;
    r.method(http::string_to_verb(method));
    r.target(target);
    r.prepare_payload();
    return r;
  }

  std::shared_ptr<NiceMock<MockHandler>> MakeOkHandler(const std::string& endpoint) {
    http::response<http::string_body> ok;
    ok.result(http::status::ok);
    ok.body() = "ok";
    ok.prepare_payload();

    auto h = std::make_shared<NiceMock<MockHandler>>();
    ON_CALL(*h, GetMethodEndpoint()).WillByDefault(Return(endpoint));
    ON_CALL(*h, Execute(::testing::_)).WillByDefault(Return(ok));
    return h;
  }

  void SetUp() override {
    router_.Add(MakeOkHandler("POST /task"));
    router_.Add(MakeOkHandler("PATCH /task/status"));
    router_.Add(MakeOkHandler("DELETE /task/delete"));
    router_.Add(MakeOkHandler("GET /task/getByUser"));
    router_.Add(MakeOkHandler("GET /task/get"));
    router_.Add(MakeOkHandler("POST /user/register"));
    router_.Add(MakeOkHandler("POST /user/login"));
  }

  SimpleRouter router_;
};

TEST_F(RouterTest, NotFound) {
  auto resp = router_.Dispatch(MakeReq("GET", "/unknown"));
  EXPECT_EQ(resp.result(), http::status::not_found);
}

TEST_F(RouterTest, PostTask_RoutesOk) {
  EXPECT_EQ(router_.Dispatch(MakeReq("POST", "/task")).result(),
            http::status::ok);
}

TEST_F(RouterTest, PatchTaskStatus_RoutesOk) {
  EXPECT_EQ(router_.Dispatch(MakeReq("PATCH", "/task/status?id=1")).result(),
            http::status::ok);
}

TEST_F(RouterTest, DeleteTask_RoutesOk) {
  EXPECT_EQ(router_.Dispatch(MakeReq("DELETE", "/task/delete?id=1")).result(),
            http::status::ok);
}

TEST_F(RouterTest, GetTasksByUser_RoutesOk) {
  EXPECT_EQ(router_.Dispatch(MakeReq("GET", "/task/getByUser?user_id=1")).result(),
            http::status::ok);
}

TEST_F(RouterTest, GetTask_RoutesOk) {
  EXPECT_EQ(router_.Dispatch(MakeReq("GET", "/task/get?id=1")).result(),
            http::status::ok);
}

TEST_F(RouterTest, UserRegister_RoutesOk) {
  EXPECT_EQ(router_.Dispatch(MakeReq("POST", "/user/register")).result(),
            http::status::ok);
}

TEST_F(RouterTest, UserLogin_RoutesOk) {
  EXPECT_EQ(router_.Dispatch(MakeReq("POST", "/user/login")).result(),
            http::status::ok);
}


