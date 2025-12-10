#pragma once

#include <memory>

#include "handlers/task_handlers.hpp"
#include "handlers/user_handlers.hpp"
#include "router.hpp"
#include "../application/use_cases/default_task_service.hpp"
#include "../infrastructure/database/postgres/postgres_task_repository.hpp"

inline void RouterDefaultConfigure(const std::shared_ptr<Router>& router,
                            std::shared_ptr<UserService>& user_service,
                            std::shared_ptr<TaskService>& task_service) {
  router->Add(std::make_shared<RegisterUserHandler>(user_service));
  router->Add(std::make_shared<LoginUserHandler>(user_service));
  
  router->Add(std::make_shared<CreateTaskHandler>(task_service));
  router->Add(std::make_shared<ChangeStatusTaskHandler>(task_service));
  router->Add(std::make_shared<DeleteTaskHandler>(task_service));
  router->Add(std::make_shared<GetAllTasksHandler>(task_service));
  router->Add(std::make_shared<GetOneTaskHandler>(task_service));
}
