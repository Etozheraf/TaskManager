 #pragma once
 
 #include <QObject>
 #include "tasks_api_client.h"
 
 class MockTasksApiClient final : public TasksApiClient {
   Q_OBJECT
  public:
   explicit MockTasksApiClient(QObject* parent = nullptr)
       : TasksApiClient(nullptr, QUrl(), parent) {}

   struct Calls {
     QString create_userId;
     QString create_title;
     QString create_desc;
     QString create_status;
     QString delete_id;
     QString update_id;
     QString update_status;
   } calls;
 
  public slots:
   void fetchTasksByUser(const QString& userUuid) { Q_UNUSED(userUuid); }
   void createTask(const QString& userId, const QString& title, const QString& description,
                   const QString& status) {
     calls.create_userId = userId;
     calls.create_title = title;
     calls.create_desc = description;
     calls.create_status = status;
   }
   void deleteTask(const QString& taskId) { calls.delete_id = taskId; }
   void updateTaskStatus(const QString& taskId, const QString& status) {
     calls.update_id = taskId;
     calls.update_status = status;
   }
   void cancel() {}
 };
 

