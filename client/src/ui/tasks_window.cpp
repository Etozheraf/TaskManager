#include "tasks_window.h"

#include <QAction>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QMetaObject>
#include <QNetworkAccessManager>
#include <QPushButton>
#include <QSizePolicy>
#include <QTextEdit>
#include <QTextOption>
#include <QToolButton>

#include "elided_label.h"
#include "tasks_api_client.h"
#include "types.h"

TasksWindow::TasksWindow(const QSharedPointer<TasksApiClient>& tasks_client,
                         const QString& user_id, const QVector<Task>& tasks,
                         QWidget* parent)
    : QWidget(parent),
      tasks_list_(nullptr),
      tasks_client_(tasks_client),
      user_id_(user_id) {
  tasks_list_ = new QListWidget(this);
  tasks_list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  tasks_list_->setSpacing(4);

  auto* add_button = new QPushButton(tr("Добавить"), this);
  add_button->setObjectName("primaryButton");

  auto* logout_button = new QPushButton(tr("Выйти"), this);
  logout_button->setObjectName("linkButton");

  setAttribute(Qt::WA_StyledBackground, true);

  auto* header = new QLabel(tr("Мои задачи"), this);
  header->setObjectName("headerTitle");
  auto* top_layout = new QHBoxLayout();
  top_layout->addWidget(header);
  top_layout->addStretch();
  top_layout->addWidget(add_button);
  top_layout->addWidget(logout_button);
  top_layout->setSpacing(12);

  auto* root_layout = new QVBoxLayout(this);
  root_layout->addLayout(top_layout);
  root_layout->addWidget(tasks_list_);
  root_layout->setContentsMargins(32, 24, 32, 32);
  root_layout->setSpacing(12);
  setLayout(root_layout);

  connect(logout_button, &QPushButton::clicked, this,
          &TasksWindow::handleLogout);
  connect(add_button, &QPushButton::clicked, this, &TasksWindow::addTaskDialog);
  connect(tasks_list_, &QListWidget::itemClicked, this,
          &TasksWindow::showTaskDetails);
  connect(tasks_client_.get(), &TasksApiClient::taskDeleted, this,
          &TasksWindow::handleTaskDeleted);
  connect(tasks_client_.get(), &TasksApiClient::taskCreated, this,
          &TasksWindow::handleTaskCreated);
  connect(tasks_client_.get(), &TasksApiClient::taskStatusUpdated, this,
          &TasksWindow::handleTaskStatusUpdated);

  for (const auto& t : tasks) {
    auto* item = new QListWidgetItem(tasks_list_);
    QWidget* w = createTaskItemWidget(t);
    item->setData(Qt::UserRole, t.id);
    tasks_list_->addItem(item);
    tasks_list_->setItemWidget(item, w);
    item->setSizeHint(w->minimumSizeHint());
  }
}

QWidget* TasksWindow::createTaskItemWidget(const Task& task) {
  auto* container = new QWidget(tasks_list_);
  auto* layout = new QHBoxLayout(container);
  layout->setContentsMargins(10, 8, 10, 8);
  layout->setSpacing(12);

  auto* status = new QLabel(task.status, container);
  status->setObjectName("statusBadge");
  status->setProperty("status", task.status);
  status->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
  status->setCursor(Qt::PointingHandCursor);
  status->installEventFilter(this);
  status->setProperty("taskId", task.id);

  auto* sep1 = new QWidget(container);
  sep1->setObjectName("vSeparator");
  sep1->setFixedWidth(1);
  sep1->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  auto* title = new ElidedLabel(task.title, container);
  title->setObjectName("taskTitle");
  title->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  title->setMaxLines(1);

  auto* sep2 = new QWidget(container);
  sep2->setObjectName("vSeparator");
  sep2->setFixedWidth(1);
  sep2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  auto* description = new ElidedLabel(task.description, container);
  description->setObjectName("taskDescription");
  description->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  description->setMaxLines(1);

  auto* removeBtn = new QToolButton(container);
  removeBtn->setText("×");
  removeBtn->setAutoRaise(true);
  removeBtn->setCursor(Qt::PointingHandCursor);
  removeBtn->setToolTip(tr("Удалить задачу"));
  removeBtn->setObjectName("removeButton");
  const QString idCopy = task.id;
  connect(removeBtn, &QToolButton::clicked, this, [this, idCopy, removeBtn]() {
    if (!idCopy.isEmpty() && tasks_client_) {
      removeBtn->setEnabled(false);
      QMetaObject::invokeMethod(tasks_client_.get(), "deleteTask",
                                Qt::QueuedConnection, Q_ARG(QString, idCopy));
    }
  });

  layout->addWidget(status, 0, Qt::AlignTop);
  layout->addWidget(sep1, 0);
  layout->addWidget(title, 1);
  layout->addWidget(sep2, 0);
  layout->addWidget(description, 2);
  layout->addWidget(removeBtn, 0, Qt::AlignRight | Qt::AlignVCenter);
  container->setLayout(layout);

  return container;
}

bool TasksWindow::eventFilter(QObject* obj, QEvent* event) {
  const auto* statusLabel = qobject_cast<QLabel*>(obj);
  if (!statusLabel) {
    return QWidget::eventFilter(obj, event);
  }

  if (statusLabel->objectName() != QStringLiteral("statusBadge") ||
      event->type() != QEvent::MouseButtonRelease) {
    return QWidget::eventFilter(obj, event);
  }
  const QString taskId = statusLabel->property("taskId").toString();
  QMenu menu(this);
  menu.addAction(QStringLiteral("InProgress"));
  menu.addAction(QStringLiteral("InProject"));
  menu.addAction(QStringLiteral("Done"));
  const QAction* chosen =
      menu.exec(statusLabel->mapToGlobal(statusLabel->rect().bottomLeft()));
  if (!chosen)
    return true;

  if (!taskId.isEmpty() && !chosen->text().isEmpty()) {
    requestUpdateStatus(taskId, chosen->text());
  }
  return true;
}

void TasksWindow::handleLogout() {
  emit logoutRequested();
}

void TasksWindow::handleTaskCreated(const bool ok, const Task& task,
                                    const QString& error_message) {
  if (!ok) {
    QMessageBox::warning(this, tr("Ошибка"),
                         error_message.isEmpty()
                             ? tr("Не удалось создать задачу")
                             : error_message);
    return;
  }

  auto* item = new QListWidgetItem(tasks_list_);
  QWidget* w = createTaskItemWidget(task);
  item->setData(Qt::UserRole, task.id);
  tasks_list_->addItem(item);
  tasks_list_->setItemWidget(item, w);
  item->setSizeHint(w->minimumSizeHint());
}

void TasksWindow::handleTaskDeleted(const bool ok, const QString& taskId,
                                    const QString& error_message) {
  QListWidgetItem* item = nullptr;
  for (int i = 0; i < tasks_list_->count(); ++i) {
    if (tasks_list_->item(i)->data(Qt::UserRole).toString() != taskId) {
      continue;
    }
    item = tasks_list_->item(i);
    break;
  }
  if (!item) {
    QMessageBox::warning(this, tr("Ошибка"), tr("Задача не найдена"));
    return;
  }

  if (!ok) {
    tasks_list_->itemWidget(item)
        ->findChild<QToolButton*>("removeButton")
        ->setEnabled(true);
    QMessageBox::warning(this, tr("Ошибка"), error_message);
    return;
  }

  tasks_list_->removeItemWidget(item);
  delete item;
}

void TasksWindow::handleTaskStatusUpdated(const bool ok, const QString& task_id,
                                          const QString& task_status,
                                          const QString& error_message) {
  if (!ok) {
    QMessageBox::warning(this, tr("Ошибка"), error_message);
    return;
  }
  for (int i = 0; i < tasks_list_->count(); ++i) {
    QListWidgetItem* item = tasks_list_->item(i);

    if (item->data(Qt::UserRole).toString() != task_id)
      continue;

    auto* label =
        tasks_list_->itemWidget(item)->findChild<QLabel*>("statusBadge");
    label->setText(task_status);
    label->setProperty("status", task_status);
    label->style()->unpolish(label);
    label->style()->polish(label);
    label->update();
    break;
  }
}

void TasksWindow::showTaskDetails(QListWidgetItem* item) {
  const QWidget* w = tasks_list_->itemWidget(item);
  if (!w)
    return;

  const QString title = w->findChild<ElidedLabel*>("taskTitle")->fullText();
  const QString description = w->findChild<ElidedLabel*>("taskDescription")->fullText();
  const QString status = w->findChild<QLabel*>("statusBadge")->text();

  QDialog dlg(this);
  dlg.setWindowTitle(tr("Задача"));
  dlg.setMinimumWidth(400);
  dlg.setMinimumHeight(400);

  auto* statusLabel = new QLabel(status, &dlg);
  statusLabel->setObjectName("statusBadge");
  statusLabel->setProperty("status", status);
  statusLabel->setFixedWidth(100);

  auto* titleLabel = new QLabel(title, &dlg);
  titleLabel->setObjectName("taskTitle");
  titleLabel->setWordWrap(true);
  titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  titleLabel->setMaximumWidth(400);
  titleLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);


  auto* descLabel = new QLabel(description, &dlg);
  descLabel->setObjectName("taskDescription");
  descLabel->setWordWrap(true);
  descLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  descLabel->setMaximumWidth(400);
  descLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

  auto* form = new QFormLayout(&dlg);
  form->addRow(tr("Статус:"), statusLabel);
  form->addItem(new QSpacerItem(0, 5, QSizePolicy::Minimum, QSizePolicy::Fixed));
  form->addRow(tr("Заголовок:"), titleLabel);
  form->setAlignment(titleLabel, Qt::AlignTop );
  form->addRow(tr("Описание:"), descLabel);
  form->setAlignment(descLabel, Qt::AlignTop );

  dlg.exec();
}

void TasksWindow::addTaskDialog() {
  auto* dlg = new QDialog(this);
  dlg->setAttribute(Qt::WA_DeleteOnClose);
  dlg->setWindowTitle(tr("Новая задача"));
  dlg->setMinimumWidth(400);
  dlg->setMinimumHeight(400);

  auto* statusCombo = new QComboBox(dlg);
  statusCombo->addItems({QStringLiteral("InProgress"),
                         QStringLiteral("InProject"), QStringLiteral("Done")});
  auto* titleEdit = new QLineEdit(dlg);
  auto* descEdit = new QTextEdit(dlg);
  descEdit->setAcceptRichText(false);
  descEdit->setWordWrapMode(QTextOption::WordWrap);
  descEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  auto* form = new QFormLayout();
  form->addRow(tr("Статус"), statusCombo);
  form->addRow(tr("Заголовок"), titleEdit);
  form->addRow(tr("Описание"), descEdit);

  auto* buttons = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, dlg);

  auto* v = new QVBoxLayout(dlg);
  v->addLayout(form);
  v->addWidget(buttons);

  connect(buttons, &QDialogButtonBox::accepted, dlg, [=]() {
    const QString title = titleEdit->text().trimmed();
    const QString description = descEdit->toPlainText().trimmed();
    const QString status = statusCombo->currentText();

    dlg->accept();
    if (title.isEmpty()) {
      QMessageBox::warning(this, tr("Ошибка"), tr("Заполните заголовок"));
      return;
    }
    if (user_id_.isEmpty()) {
      QMessageBox::warning(this, tr("Ошибка"),
                           tr("Не определён пользователь для создания задачи"));
      return;
    }

    if (!tasks_client_)
      return;
    QMetaObject::invokeMethod(
        tasks_client_.get(), "createTask", Qt::QueuedConnection,
        Q_ARG(QString, user_id_), Q_ARG(QString, title),
        Q_ARG(QString, description), Q_ARG(QString, status));
  });
  connect(buttons, &QDialogButtonBox::rejected, dlg, &QDialog::reject);

  dlg->show();
}

void TasksWindow::requestUpdateStatus(const QString& taskId,
                                      const QString& status) const {
  if (!tasks_client_)
    return;
  QMetaObject::invokeMethod(tasks_client_.get(), "updateTaskStatus",
                            Qt::QueuedConnection, Q_ARG(QString, taskId),
                            Q_ARG(QString, status));
}