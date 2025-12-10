#pragma once

#include <QWidget>

class QLineEdit;
class QPushButton;
class QNetworkAccessManager;
class QUrl;
class AuthApiClient;

class RegisterWindow : public QWidget {
  Q_OBJECT

 public:
 explicit RegisterWindow(const QSharedPointer<AuthApiClient>& auth_client,
                          QWidget* parent = nullptr);

  QSize minimumSizeHint() const override { return {400, 300}; }

 signals:
  void loginRequested();
  void registrationSucceeded();

 private slots:
  void handleRegister() const;
  void handleGoToLogin();
  void onRegistrationFinished(bool ok, const QString& error_message);

 private:
  void setupUi();
  void setupConnections();

  QLineEdit* username_edit_;
  QLineEdit* password_edit_;
  QLineEdit* confirm_edit_;

  QSharedPointer<AuthApiClient> auth_client_;
};
