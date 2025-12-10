#include "auth_api_client.h"

#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

AuthApiClient::AuthApiClient(const QSharedPointer<QNetworkAccessManager>& nam,
                             const QUrl& baseUrl, QObject* parent)
    : QObject(parent), nam_(nam), base_url_(baseUrl) {}

void AuthApiClient::cancel() {
  if (current_reply_) {
    disconnect(current_reply_, nullptr, this, nullptr);
    current_reply_->abort();
    current_reply_->deleteLater();
    current_reply_.clear();
  }
}

void AuthApiClient::registerUser(const QString& username,
                                 const QString& password) {
  cancel();

  const QUrl url = base_url_.resolved(QUrl(QStringLiteral("user/register")));
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader,
                QStringLiteral("application/json"));

  QJsonObject payload{{QStringLiteral("login"), username},
                      {QStringLiteral("password"), password}};
  const QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);

  QNetworkReply* reply = nam_->post(req, body);
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
      emit registrationFinished(false, reply->errorString());
      return;
    }

    emit registrationFinished(true, QString());
  });
}

void AuthApiClient::login(const QString& username, const QString& password) {
  cancel();

  const QUrl url = base_url_.resolved(QUrl(QStringLiteral("user/login")));
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader,
                QStringLiteral("application/json"));
  QJsonObject payload{{QStringLiteral("login"), username},
                      {QStringLiteral("password"), password}};
  const QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);

  QNetworkReply* reply = nam_->post(req, body);
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
      emit loginFinished(false, QString(), reply->errorString());
      return;
    }

    const QByteArray data = reply->readAll();
    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
      emit loginFinished(false, QString(), tr("Некорректный ответ сервера"));
      return;
    }

    const QString uuid = doc.object().value(QStringLiteral("id")).toString();

    if (uuid.isEmpty()) {
      emit loginFinished(false, QString(), tr("Не найден user_uuid в ответе"));
      return;
    }
    emit loginFinished(true, uuid, QString());
  });
}