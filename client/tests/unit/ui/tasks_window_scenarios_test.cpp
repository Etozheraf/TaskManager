#include <QApplication>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QToolButton>
#include <QtTest/QtTest>

#include "mock_tasks_api_client.h"
#include "tasks_window.h"

class TasksWindowScenariosTest : public QObject {
  Q_OBJECT
 private slots:
  void createTask_ok_sendsRequestAndAddsItem();
  void createTask_emptyTitle_showsWarningAndNoRequest();
  void deleteTask_ok_removesItem();
  void deleteTask_error_reenablesButton();
  void updateStatus_changesBadge();
};

TasksWindow* makeWindow(QSharedPointer<MockTasksApiClient>& outClient,
                        QVector<Task> initial = {}) {
  auto* w = new TasksWindow(outClient, QStringLiteral("user-1"), initial);
  w->resize(600, 400);
  w->show();
  QTest::qWait(20);
  return w;
}

void TasksWindowScenariosTest::createTask_ok_sendsRequestAndAddsItem() {
  QSharedPointer<MockTasksApiClient> mock =
      QSharedPointer<MockTasksApiClient>::create();
  std::unique_ptr<TasksWindow> w(makeWindow(mock));
  auto* addBtn = w->findChild<QPushButton*>("primaryButton");
  QVERIFY(addBtn);

  QTimer::singleShot(100, [&]() {
    for (QWidget* tl : QApplication::topLevelWidgets()) {
      QDialog* dlg = qobject_cast<QDialog*>(tl);
      if (!dlg) {
        continue;
      }
      auto* combo = dlg->findChild<QComboBox*>();
      QVERIFY(combo);
      combo->setCurrentText("Done");

      auto edits = dlg->findChildren<QLineEdit*>();
      QVERIFY(!edits.isEmpty());
      edits[0]->setText("Title");

      auto* desc = dlg->findChild<QTextEdit*>();
      QVERIFY(desc);
      desc->setPlainText("Desc");

      auto* buttons = dlg->findChild<QDialogButtonBox*>();
      QVERIFY(buttons);
      buttons->button(QDialogButtonBox::Ok)->click();
    }
  });
  QTest::mouseClick(addBtn, Qt::LeftButton);
  QTest::qWait(200);

  QCOMPARE(mock->calls.create_userId, QStringLiteral("user-1"));
  QCOMPARE(mock->calls.create_title, QStringLiteral("Title"));
  QCOMPARE(mock->calls.create_desc, QStringLiteral("Desc"));
  QCOMPARE(mock->calls.create_status, QStringLiteral("Done"));

  Task created{"id-1", "user-1", "Title", "Desc", "Done"};
  emit mock->taskCreated(true, created, QString());
  auto* list = w->findChild<QListWidget*>();
  QVERIFY(list);
  QVERIFY(list->count() == 1);
}

void TasksWindowScenariosTest::
    createTask_emptyTitle_showsWarningAndNoRequest() {

  QSharedPointer<MockTasksApiClient> mock =
      QSharedPointer<MockTasksApiClient>::create();
  std::unique_ptr<TasksWindow> w(makeWindow(mock));
  auto* addBtn = w->findChild<QPushButton*>("primaryButton");
  QVERIFY(addBtn);

  QTimer::singleShot(50, [&]() {
    for (QWidget* tl : QApplication::topLevelWidgets()) {
      if (auto* dlg = qobject_cast<QDialog*>(tl)) {
        auto* buttons = dlg->findChild<QDialogButtonBox*>();
        QVERIFY(buttons);
        buttons->button(QDialogButtonBox::Ok)->click();
      }
    }
  });
  QTimer::singleShot(100, [&]() {
    if (auto* mb =
            qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
      auto* okBtn = mb->button(QMessageBox::Ok);
      QVERIFY(okBtn);
      QTest::mouseClick(okBtn, Qt::LeftButton);
    }
  });

  QTest::mouseClick(addBtn, Qt::LeftButton);
  QTest::qWait(200);
  QVERIFY(mock->calls.create_title.isEmpty());
}

void TasksWindowScenariosTest::deleteTask_ok_removesItem() {
  QSharedPointer<MockTasksApiClient> mock =
      QSharedPointer<MockTasksApiClient>::create();
  QVector<Task> initial{{"t1", "user-1", "Title", "Desc", "InProgress"}};
  std::unique_ptr<TasksWindow> w(makeWindow(mock, initial));

  auto* list = w->findChild<QListWidget*>();
  QWidget* itemWidget = list->itemWidget(list->item(0));
  auto* removeBtn = itemWidget->findChild<QToolButton*>("removeButton");

  QTest::mouseClick(removeBtn, Qt::LeftButton);
  QTest::qWait(100);
  QCOMPARE(mock->calls.delete_id, QStringLiteral("t1"));

  emit mock->taskDeleted(true, QStringLiteral("t1"), QString());
  QTest::qWait(100);
  QCOMPARE(list->count(), 0);
}

void TasksWindowScenariosTest::deleteTask_error_reenablesButton() {
  QSharedPointer<MockTasksApiClient> mock =
      QSharedPointer<MockTasksApiClient>::create();
  QVector<Task> initial{{"t1", "user-1", "Title", "Desc", "InProgress"}};
  std::unique_ptr<TasksWindow> w(makeWindow(mock, initial));
  auto* list = w->findChild<QListWidget*>();
  QWidget* itemWidget = list->itemWidget(list->item(0));
  auto* removeBtn = itemWidget->findChild<QToolButton*>("removeButton");

  QTimer::singleShot(50, [&]() {
    if (auto* mb =
            qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) {
      auto* okBtn = mb->button(QMessageBox::Ok);
      QVERIFY(okBtn);
      QTest::mouseClick(okBtn, Qt::LeftButton);
    }
  });
  QTest::mouseClick(removeBtn, Qt::LeftButton);
  emit mock->taskDeleted(false, QStringLiteral("t1"), QStringLiteral("err"));
  QTest::qWait(100);
  QVERIFY(removeBtn->isEnabled());
}

void TasksWindowScenariosTest::updateStatus_changesBadge() {
  QSharedPointer<MockTasksApiClient> mock =
      QSharedPointer<MockTasksApiClient>::create();
  QVector<Task> initial{{"t1", "user-1", "Title", "Desc", "InProgress"}};
  std::unique_ptr<TasksWindow> w(makeWindow(mock, initial));
  auto* list = w->findChild<QListWidget*>();
  QWidget* itemWidget = list->itemWidget(list->item(0));
  auto* badge = itemWidget->findChild<QLabel*>("statusBadge");

  // Эмулируем сигнал изменения статуса
  emit mock->taskStatusUpdated(true, QStringLiteral("t1"),
                               QStringLiteral("Done"), QString());
  QTest::qWait(10);
  QCOMPARE(badge->text(), QStringLiteral("Done"));
}

QTEST_MAIN(TasksWindowScenariosTest)
#include "tasks_window_scenarios_test.moc"
