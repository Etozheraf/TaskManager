#pragma once

#include <QMainWindow>
#include <QSharedPointer>
#include <QString>
#include <QThread>

#include "auth_api_client.h"
#include "tasks_api_client.h"
#include "types.h"

class LoginWindow;
class RegisterWindow;
class TasksWindow;
class QNetworkAccessManager;

class MainWindow final : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(const QUrl& url,
                      QWidget* parent = nullptr);

  ~MainWindow() override;

 private slots:
  void showLogin();
  void showRegister();
  void showTasks(const QString& user_id, const QVector<Task>& tasks);

 private:
  QSharedPointer<AuthApiClient> auth_client_;
  QSharedPointer<TasksApiClient> tasks_client_;

  QThread network_thread_;
};
