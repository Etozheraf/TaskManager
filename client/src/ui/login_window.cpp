#include "login_window.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QMetaObject>
#include <QNetworkAccessManager>
#include <QPushButton>
#include <QTimer>

#include "auth_api_client.h"
#include "tasks_api_client.h"

LoginWindow::LoginWindow(const QSharedPointer<AuthApiClient>& auth_client,
                         const QSharedPointer<TasksApiClient>& tasks_client,
                         QWidget* parent)
    : QWidget(parent),
      username_edit_(new QLineEdit(this)),
      password_edit_(new QLineEdit(this)),
      auth_client_(auth_client),
      tasks_client_(tasks_client) {
  password_edit_->setPlaceholderText(tr("Пароль"));
  password_edit_->setEchoMode(QLineEdit::Password);
  username_edit_->setPlaceholderText(tr("Логин"));
  setAttribute(Qt::WA_StyledBackground, true);

  auto* login_button = new QPushButton(tr("Войти"), this);
  login_button->setObjectName("primaryButton");
  auto* signup_button = new QPushButton(tr("Регистрация"), this);
  signup_button->setObjectName("linkButton");

  auto* form_layout = new QFormLayout();
  form_layout->addRow(tr("Логин"), username_edit_);
  form_layout->addRow(tr("Пароль"), password_edit_);
  form_layout->setSpacing(12);

  auto* buttons_layout = new QHBoxLayout();
  buttons_layout->addWidget(login_button);
  buttons_layout->addWidget(signup_button);
  buttons_layout->setSpacing(12);

  auto* root_layout = new QVBoxLayout(this);
  root_layout->addLayout(form_layout);
  root_layout->addLayout(buttons_layout);
  root_layout->setContentsMargins(32, 32, 32, 32);

  connect(login_button, &QPushButton::clicked, this, &LoginWindow::handleLogin);
  connect(auth_client_.get(), &AuthApiClient::loginFinished, this,
          &LoginWindow::onLoginFinished);
  connect(tasks_client_.get(), &TasksApiClient::tasksFetched, this,
          &LoginWindow::onTasksFetched);

  connect(signup_button, &QPushButton::clicked, this,
          &LoginWindow::handleSignUp);
}

void LoginWindow::handleLogin() {
  QMetaObject::invokeMethod(auth_client_.get(), "cancel", Qt::QueuedConnection);
  QMetaObject::invokeMethod(tasks_client_.get(), "cancel",
                            Qt::QueuedConnection);

  const auto username = username_edit_->text().trimmed();
  const auto password = password_edit_->text();
  if (username.isEmpty() || password.isEmpty()) {
    return;
  }

  pending_user_uuid_.clear();
  QMetaObject::invokeMethod(auth_client_.get(), "login", Qt::QueuedConnection,
                            Q_ARG(QString, username), Q_ARG(QString, password));
}

void LoginWindow::handleSignUp() {
  QMetaObject::invokeMethod(auth_client_.get(), "cancel", Qt::QueuedConnection);
  QMetaObject::invokeMethod(tasks_client_.get(), "cancel",
                            Qt::QueuedConnection);
  emit signUpRequested();
}

void LoginWindow::onLoginFinished(const bool ok, const QString& user_uuid,
                                  const QString& error_message) {
  if (!ok) {
    QMessageBox::warning(this, tr("Ошибка входа"),
                         error_message.isEmpty() ? tr("Неверные учетные данные")
                                                 : error_message);
    return;
  }
  pending_user_uuid_ = user_uuid;
  QMetaObject::invokeMethod(tasks_client_.get(), "fetchTasksByUser",
                            Qt::QueuedConnection, Q_ARG(QString, user_uuid));
}

void LoginWindow::onTasksFetched(const bool ok, const QVector<Task>& tasks,
                                 const QString& error_message) {
  if (!ok) {
    QMessageBox::warning(this, tr("Ошибка загрузки задач"),
                         error_message.isEmpty()
                             ? tr("Не удалось получить задачи")
                             : error_message);
    return;
  }
  if (pending_user_uuid_.isEmpty()) {
    return;
  }
  emit loginSucceeded(pending_user_uuid_, tasks);
}