#pragma once

#include <QVector>
#include <QWidget>
#include "types.h"

class QLineEdit;
class QPushButton;
class QNetworkAccessManager;
class QUrl;
class AuthApiClient;
class TasksApiClient;

class LoginWindow final : public QWidget {
  Q_OBJECT

 public:
  explicit LoginWindow(const QSharedPointer<AuthApiClient>& auth_client,
                       const QSharedPointer<TasksApiClient>& tasks_client,
                       QWidget* parent = nullptr);

  QSize minimumSizeHint() const override { return {320, 260}; }

 signals:
  void signUpRequested();
  void loginSucceeded(const QString& user_id, const QVector<Task>& tasks);

 private slots:
  void handleLogin();
  void handleSignUp();
  void onLoginFinished(bool ok, const QString& user_uuid,
                       const QString& error_message);
  void onTasksFetched(bool ok, const QVector<Task>& tasks,
                      const QString& error_message);

 private:
  QLineEdit* username_edit_;
  QLineEdit* password_edit_;

  QSharedPointer<AuthApiClient> auth_client_;
  QSharedPointer<TasksApiClient> tasks_client_;
  QString pending_user_uuid_;
};
