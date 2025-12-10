#include "main_window.h"

#include <QNetworkAccessManager>
#include <QUrl>

#include "login_window.h"
#include "register_window.h"
#include "tasks_window.h"

MainWindow::MainWindow(const QUrl& url,
                       QWidget* parent)
    : QMainWindow(parent) {
  setWindowTitle(QStringLiteral(" "));

  network_thread_.start(QThread::Priority::InheritPriority);
  auto nam = QSharedPointer<QNetworkAccessManager>::create(nullptr);
  nam->moveToThread(&network_thread_);
  auth_client_ = QSharedPointer<AuthApiClient>::create(nam, url);
  auth_client_->moveToThread(&network_thread_);
  tasks_client_ = QSharedPointer<TasksApiClient>::create(nam, url);
  tasks_client_->moveToThread(&network_thread_);

  showLogin();
}

MainWindow::~MainWindow() {
  network_thread_.quit();
  network_thread_.wait();
}


void MainWindow::showLogin() {
  if (auto* old = centralWidget()) {
    old->deleteLater();
  }

  auto* login = new LoginWindow(auth_client_, tasks_client_, this);
  setCentralWidget(login);
  setMinimumSize(login->minimumSizeHint());
  resize(login->sizeHint());

  connect(login, &LoginWindow::signUpRequested, this,
          &MainWindow::showRegister);
  connect(login, &LoginWindow::loginSucceeded, this, &MainWindow::showTasks);
}

void MainWindow::showRegister() {
  if (auto* old = centralWidget()) {
    old->deleteLater();
  }
  auto* reg = new RegisterWindow(auth_client_, this);
  setCentralWidget(reg);
  setMinimumSize(reg->minimumSizeHint());
  resize(reg->minimumSizeHint());

  connect(reg, &RegisterWindow::loginRequested, this, &MainWindow::showLogin);
  connect(reg, &RegisterWindow::registrationSucceeded, this,
          &MainWindow::showLogin);
}

void MainWindow::showTasks(const QString& user_id, const QVector<Task>& tasks) {
  if (auto* old = centralWidget()) {
    old->deleteLater();
  }
  auto* tasks_window = new TasksWindow(tasks_client_, user_id, tasks, this);
  setCentralWidget(tasks_window);
  setMinimumSize(tasks_window->minimumSizeHint());
  resize(tasks_window->minimumSizeHint());

  connect(tasks_window, &TasksWindow::logoutRequested, this,
          &MainWindow::showLogin);
}
