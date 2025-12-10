#pragma once

#include <QObject>
#include <QPointer>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

class AuthApiClient final : public QObject {
  Q_OBJECT
 public:
  explicit AuthApiClient(const QSharedPointer<QNetworkAccessManager>& nam,
                         const QUrl& baseUrl, QObject* parent = nullptr);

 public slots:
  void registerUser(const QString& username, const QString& password);
  void login(const QString& username, const QString& password);
  void cancel();

 signals:
  void registrationFinished(bool ok, const QString& error_message);
  void loginFinished(bool ok, const QString& user_uuid,
                     const QString& error_message);

 private:
  QSharedPointer<QNetworkAccessManager> nam_;
  QUrl base_url_;
  QPointer<QNetworkReply> current_reply_;
};
