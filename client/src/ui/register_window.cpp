#include "register_window.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QMetaObject>
#include <QNetworkAccessManager>
#include <QPushButton>

#include "auth_api_client.h"

RegisterWindow::RegisterWindow(const QSharedPointer<AuthApiClient>& auth_client,
                               QWidget* parent)
    : QWidget(parent),
      username_edit_(new QLineEdit(this)),
      password_edit_(new QLineEdit(this)),
      confirm_edit_(new QLineEdit(this)),
      auth_client_(auth_client) {
  setWindowTitle(tr("Регистрация"));
  password_edit_->setEchoMode(QLineEdit::Password);
  confirm_edit_->setEchoMode(QLineEdit::Password);
  username_edit_->setPlaceholderText(tr("Логин"));
  password_edit_->setPlaceholderText(tr("Пароль"));
  confirm_edit_->setPlaceholderText(tr("Подтверждение пароля"));
  setAttribute(Qt::WA_StyledBackground, true);

  auto* register_button = new QPushButton(tr("Зарегистрироваться"), this);
  register_button->setObjectName("primaryButton");
  auto* login_button = new QPushButton(tr("У меня уже есть аккаунт"), this);
  login_button->setObjectName("linkButton");

  auto* form_layout = new QFormLayout();
  form_layout->addRow(tr("Логин"), username_edit_);
  form_layout->addRow(tr("Пароль"), password_edit_);
  form_layout->addRow(tr("Подтверждение"), confirm_edit_);
  form_layout->setSpacing(12);

  auto* buttons_layout = new QHBoxLayout();
  buttons_layout->addWidget(register_button);
  buttons_layout->addWidget(login_button);
  buttons_layout->setSpacing(12);

  auto* root_layout = new QVBoxLayout(this);
  root_layout->addLayout(form_layout);
  root_layout->addLayout(buttons_layout);
  root_layout->setContentsMargins(32, 32, 32, 32);
  setLayout(root_layout);

  connect(register_button, &QPushButton::clicked, this,
          &RegisterWindow::handleRegister);
  connect(login_button, &QPushButton::clicked, this,
          &RegisterWindow::handleGoToLogin);
  connect(auth_client_.get(), &AuthApiClient::registrationFinished, this,
          &RegisterWindow::onRegistrationFinished);
}

void RegisterWindow::handleRegister() const {
  QMetaObject::invokeMethod(auth_client_.get(), "cancel", Qt::QueuedConnection);

  const auto username = username_edit_->text().trimmed();
  const auto password = password_edit_->text();
  const auto confirm = confirm_edit_->text();
  if (username.isEmpty() || password.isEmpty() || confirm.isEmpty()) {
    return;
  }
  if (password != confirm) {
    return;
  }
  QMetaObject::invokeMethod(auth_client_.get(), "registerUser",
                            Qt::QueuedConnection, Q_ARG(QString, username),
                            Q_ARG(QString, password));
}

void RegisterWindow::handleGoToLogin() {
  QMetaObject::invokeMethod(auth_client_.get(), "cancel", Qt::QueuedConnection);
  emit loginRequested();
}

void RegisterWindow::onRegistrationFinished(const bool ok,
                                            const QString& error_message) {
  if (!ok) {
    QMessageBox::warning(this, tr("Ошибка регистрации"), error_message);
    return;
  }
  emit registrationSucceeded();
}
