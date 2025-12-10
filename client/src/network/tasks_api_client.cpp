#include "tasks_api_client.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

TasksApiClient::TasksApiClient(const QSharedPointer<QNetworkAccessManager>& nam,
                               const QUrl& baseUrl, QObject* parent)
    : QObject(parent), nam_(nam), base_url_(baseUrl) {}

void TasksApiClient::fetchTasksByUser(const QString& userUuid) {
  cancel();

  if (!nam_) {
    emit tasksFetched(false, {},
                      QObject::tr("Сетевой менеджер не инициализирован"));
    return;
  }
  if (userUuid.isEmpty()) {
    emit tasksFetched(false, {}, QObject::tr("Пустой пользователь"));
    return;
  }

  const QUrl url = base_url_.resolved(
      QUrl(QStringLiteral("/task/getByUser?user_id=%1").arg(userUuid)));
  const QNetworkRequest req(url);

  QNetworkReply* reply = nam_->get(req);
  current_reply_ = reply;

  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    if (reply != current_reply_) {
      reply->deleteLater();
      return;
    }

    const auto guard = qScopeGuard([&]() {
      current_reply_.clear();
      reply->deleteLater();
    });

    if (reply->error() != QNetworkReply::NoError) {
      emit tasksFetched(false, {}, reply->errorString());
      return;
    }

    const QByteArray data = reply->readAll();
    QJsonParseError err{};
    QVector<Task> tasks;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject() ||
        !doc.object().contains("tasks")) {
      emit tasksFetched(false, {}, QObject::tr("Некорректный JSON"));
      return;
    }
    const QJsonValue tasksValue = doc.object().value(QStringLiteral("tasks"));
    if (!tasksValue.isArray()) {
      emit tasksFetched(false, {}, QObject::tr("Некорректный JSON"));
      return;
    }
    const QJsonArray arr = tasksValue.toArray();
    tasks.reserve(arr.size());
    for (const auto& v : arr) {
      if (!v.isObject())
        continue;
      const QJsonObject o = v.toObject();
      Task t;
      t.id = o.value(QStringLiteral("id")).toString();
      t.user_id = o.value(QStringLiteral("user_id")).toString();
      t.title = o.value(QStringLiteral("title")).toString();
      t.description = o.value(QStringLiteral("description")).toString();
      t.status = o.value(QStringLiteral("status")).toString();
      tasks.push_back(std::move(t));
    }

    emit tasksFetched(true, tasks, QString());
  });
}

void TasksApiClient::cancel() {
  if (current_reply_) {
    current_reply_->abort();
    current_reply_->deleteLater();
    current_reply_.clear();
  }
}

void TasksApiClient::createTask(const QString& userId, const QString& title,
                                const QString& description,
                                const QString& status) {
  cancel();
  if (!nam_) {
    emit taskCreated(false, Task{},
                     QObject::tr("Сетевой менеджер не инициализирован"));
    return;
  }
  const QUrl url = base_url_.resolved(QUrl(QStringLiteral("/task")));
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader,
                QStringLiteral("application/json"));

  const QJsonObject payload{{QStringLiteral("user_id"), userId},
                            {QStringLiteral("title"), title},
                            {QStringLiteral("description"), description},
                            {QStringLiteral("status"), status}};
  const QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);

  QNetworkReply* reply = nam_->post(req, body);
  current_reply_ = reply;

  connect(reply, &QNetworkReply::finished, this,
          [this, reply, userId, title, description, status]() {
            if (reply != current_reply_) {
              reply->deleteLater();
              return;
            }
            const auto guard = qScopeGuard([&]() {
              current_reply_.clear();
              reply->deleteLater();
            });

            if (reply->error() != QNetworkReply::NoError) {
              emit taskCreated(false, Task{}, reply->errorString());
              return;
            }

            const QByteArray data = reply->readAll();
            QJsonParseError err{};
            const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
            if (err.error != QJsonParseError::NoError || !doc.isObject() ||
                !doc.object().contains("id")) {
              emit taskCreated(false, Task{}, QObject::tr("Некорректный JSON"));
              return;
            }
            const QJsonObject o = doc.object();
            Task t;
            t.id = o.value(QStringLiteral("id")).toString();
            t.user_id = userId;
            t.title = title;
            t.description = description;
            t.status = status;
            emit taskCreated(true, t, QString());
          });
}

void TasksApiClient::deleteTask(const QString& taskId) {
  cancel();
  if (!nam_) {
    emit taskDeleted(false, taskId,
                     QObject::tr("Сетевой менеджер не инициализирован"));
    return;
  }
  const QUrl url = base_url_.resolved(
      QUrl(QStringLiteral("/task/delete?id=%1").arg(taskId)));
  const QNetworkRequest req(url);
  QNetworkReply* reply = nam_->deleteResource(req);
  current_reply_ = reply;

  connect(reply, &QNetworkReply::finished, this, [this, reply, taskId]() {
    if (reply != current_reply_) {
      reply->deleteLater();
      return;
    }
    const auto guard = qScopeGuard([&]() {
      current_reply_.clear();
      reply->deleteLater();
    });
    if (reply->error() != QNetworkReply::NoError) {
      emit taskDeleted(false, taskId, reply->errorString());
      return;
    }
    emit taskDeleted(true, taskId, QString());
  });
}

void TasksApiClient::updateTaskStatus(const QString& taskId,
                                      const QString& status) {
  cancel();
  if (!nam_) {
    emit taskStatusUpdated(false, taskId, status,
                           QObject::tr("Сетевой менеджер не инициализирован"));
    return;
  }
  if (taskId.isEmpty()) {
    emit taskStatusUpdated(false, taskId, status,
                           QObject::tr("Пустой идентификатор задачи"));
    return;
  }

  const QUrl url = base_url_.resolved(QUrl(QStringLiteral("/task/status")));
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader,
                QStringLiteral("application/json"));

  QJsonObject payload{{QStringLiteral("id"), taskId},
                      {QStringLiteral("status"), status}};
  const QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);

  QNetworkReply* reply = nam_->sendCustomRequest(req, "PATCH", body);
  current_reply_ = reply;

  connect(
      reply, &QNetworkReply::finished, this, [this, reply, taskId, status]() {
        if (reply != current_reply_) {
          reply->deleteLater();
          return;
        }
        const auto guard = qScopeGuard([&]() {
          current_reply_.clear();
          reply->deleteLater();
        });

        if (reply->error() != QNetworkReply::NoError) {
          emit taskStatusUpdated(false, taskId, status, reply->errorString());
          return;
        }

        emit taskStatusUpdated(true, taskId, status, QString());
      });
}