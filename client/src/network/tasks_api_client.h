#pragma once

#include <QObject>
#include <QPointer>
#include <QUrl>
#include <QVector>

#include "types.h"

class QNetworkAccessManager;
class QNetworkReply;

class TasksApiClient : public QObject {
  Q_OBJECT
 public:
  explicit TasksApiClient(const QSharedPointer<QNetworkAccessManager>& nam,
                          const QUrl& baseUrl, QObject* parent = nullptr);

 public slots:
  void fetchTasksByUser(const QString& userUuid);
  void createTask(const QString& userId, const QString& title,
                  const QString& description, const QString& status);
  void deleteTask(const QString& taskId);
  void updateTaskStatus(const QString& taskId, const QString& status);
  void cancel();

 signals:
  void tasksFetched(bool ok, const QVector<Task>& tasks,
                    const QString& error_message);
  void taskCreated(bool ok, const Task& task, const QString& error_message);
  void taskDeleted(bool ok, const QString& taskId,
                   const QString& error_message);
  void taskStatusUpdated(bool ok, const QString& taskId, const QString& status,
                         const QString& error_message);

 private:
  QSharedPointer<QNetworkAccessManager> nam_;
  QUrl base_url_;
  QPointer<QNetworkReply> current_reply_;
};
