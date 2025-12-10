#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

#include "bootstrap/routes.hpp"
#include "router.hpp"
#include "server.hpp"

#include "default_user_service.hpp"
#include "postgres_user_repository.hpp"
#include "connection_pool.hpp"

int main(int /*argc*/, char* /*argv*/[]) {
  try {
    boost::log::add_common_attributes();
    boost::log::add_console_log(
        std::clog,
        boost::log::keywords::format = "%TimeStamp% [%Severity%] %Message%");

    const std::string host =
        std::getenv("APP_HOST") ? std::getenv("APP_HOST") : "0.0.0.0";
    const uint16_t port =
        std::getenv("APP_PORT")
            ? static_cast<uint16_t>(std::stoi(std::getenv("APP_PORT")))
            : 8080;
    const std::size_t threads = std::thread::hardware_concurrency();
    std::cout << "Threads count: " << threads << "\n";

    auto router = std::make_shared<SimpleRouter>();

    const char* db_env = std::getenv("DB_CONNECTION_STRING");
    const std::string connection_string =
        db_env ? std::string{db_env} : std::string{};

    const std::size_t pool_size = threads;
    auto pool = std::make_shared<PqxxConnectionPool>(connection_string, pool_size);

    std::shared_ptr<UserService> user_service =
        std::make_shared<DefaultUserService>(
            std::make_unique<PostgreSQLUserRepository>(pool));
    std::shared_ptr<TaskService> task_service =
        std::make_shared<DefaultTaskService>(
            std::make_unique<PostgreSQLTaskRepository>(pool));

    RouterDefaultConfigure(router, user_service, task_service);

    HttpServer server(host, port, router, threads);
    BOOST_LOG_TRIVIAL(info) << "Starting Task Manager server on " << host << ":"
                            << port << std::endl;
    server.Run();
  } catch (const std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}