#pragma once

#include <QVector>
#include <QWidget>
#include "types.h"

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QNetworkAccessManager;
class QUrl;
class TasksApiClient;

class TasksWindow final : public QWidget {
  Q_OBJECT

 public:
  explicit TasksWindow(const QSharedPointer<TasksApiClient>& tasks_client,
                       const QString& user_id, const QVector<Task>& tasks,
                       QWidget* parent = nullptr);

  QSize minimumSizeHint() const override { return {760, 560}; }

 protected:
  bool eventFilter(QObject* obj, QEvent* event) override;

 signals:
  void addTaskRequested();
  void logoutRequested();

 private slots:
  void addTaskDialog();
  void handleLogout();
  void handleTaskCreated(bool ok, const Task& task,
                         const QString& error_message);
  void handleTaskDeleted(bool ok, const QString& taskId,
                         const QString& error_message);
  void handleTaskStatusUpdated(bool ok, const QString& task_id,
                               const QString& task_status,
                               const QString& error_message);
  void showTaskDetails(QListWidgetItem* item);

 private:
  void setupUi();
  void setupConnections();
  QWidget* createTaskItemWidget(const Task& task);
  void requestDeleteTask(const QString& taskId);
  void requestCreateTask(const QString& title, const QString& description,
                         const QString& status);
  void requestUpdateStatus(const QString& taskId, const QString& status) const;

  QListWidget* tasks_list_;

  QSharedPointer<TasksApiClient> tasks_client_;
  QString user_id_;
};
