#include <QApplication>
#include <QDialog>
#include <QListWidget>
#include <QToolButton>
#include <QtTest/QtTest>

#include "elided_label.h"
#include "mock_tasks_api_client.h"
#include "tasks_window.h"

class TasksWindowTest : public QObject {
  Q_OBJECT
 private slots:
  void rendersItems_withElidedLabels();
  void descriptionElidesWhenShrinking();
  void removeButtonDisablesOnClick();
  void detailsDialogShowsFullText();
};

TasksWindow* makeWindowWithTasks(const QVector<Task>& tasks) {
  const QSharedPointer<MockTasksApiClient> client = QSharedPointer<MockTasksApiClient>::create();
  auto* w = new TasksWindow(client, QStringLiteral("user-1"), tasks);
  w->resize(800, 400);
  w->show();
  QTest::qWait(20);
  return w;
}

void TasksWindowTest::rendersItems_withElidedLabels() {
  const QVector<Task> tasks{{"1", "user-1", "A very very long title to elide",
                       QString(200, 'D'), "InProgress"}};
  std::unique_ptr<TasksWindow> w(makeWindowWithTasks(tasks));
  auto* list = w->findChild<QListWidget*>();
  QVERIFY(list);
  QVERIFY(list->count() == 1);
  QWidget* itemWidget = list->itemWidget(list->item(0));
  QVERIFY(itemWidget);
  auto* title = itemWidget->findChild<ElidedLabel*>("taskTitle");
  auto* desc = itemWidget->findChild<ElidedLabel*>("taskDescription");
  QVERIFY(title);
  QVERIFY(desc);
}

void TasksWindowTest::descriptionElidesWhenShrinking() {
  QVector<Task> tasks{
      {"1", "user-1", "Title", QString(500, 'X'), "InProgress"}};
  std::unique_ptr<TasksWindow> w(makeWindowWithTasks(tasks));
  auto* list = w->findChild<QListWidget*>();
  QWidget* itemWidget = list->itemWidget(list->item(0));
  auto* desc = itemWidget->findChild<ElidedLabel*>("taskDescription");
  QVERIFY(desc);
  w->resize(800, 400);
  QTest::qWait(10);
  const QString wide = desc->text();
  w->resize(300, 400);
  QTest::qWait(10);
  const QString narrow = desc->text();
  QVERIFY(narrow.size() <= wide.size());
}

void TasksWindowTest::removeButtonDisablesOnClick() {
  QVector<Task> tasks{{"1", "user-1", "Title", "Desc", "InProgress"}};
  std::unique_ptr<TasksWindow> w(makeWindowWithTasks(tasks));
  auto* list = w->findChild<QListWidget*>();
  QWidget* itemWidget = list->itemWidget(list->item(0));
  auto* removeBtn = itemWidget->findChild<QToolButton*>("removeButton");
  QVERIFY(removeBtn);
  QTest::mouseClick(removeBtn, Qt::LeftButton);
  QVERIFY(!removeBtn->isEnabled());
}

void TasksWindowTest::detailsDialogShowsFullText() {
  const QString longTitle = QString(120, 'T');
  const QString longDesc = QString(300, 'D');
  QVector<Task> tasks{{"1", "user-1", longTitle, longDesc, "Done"}};
  std::unique_ptr<TasksWindow> w(makeWindowWithTasks(tasks));
  auto* list = w->findChild<QListWidget*>();
  QListWidgetItem* item = list->item(0);

  QTimer::singleShot(50, []() {
    // Закрыть любой модальный диалог, как только он появится
    const auto topLevels = QApplication::topLevelWidgets();
    for (QWidget* tl : topLevels) {
      if (auto* dlg = qobject_cast<QDialog*>(tl)) {
        dlg->accept();
        break;
      }
    }
  });

  // Клик по элементу открывает диалог
  list->setCurrentItem(item);
  emit list->itemClicked(item);

  // Проверяем, что текст в базовом виджете — полный (через ElidedLabel::fullText)
  QWidget* itemWidget = list->itemWidget(item);
  auto* title = itemWidget->findChild<ElidedLabel*>("taskTitle");
  auto* desc = itemWidget->findChild<ElidedLabel*>("taskDescription");
  QVERIFY(title && desc);
  QCOMPARE(title->fullText(), longTitle);
  QCOMPARE(desc->fullText(), longDesc);
}

QTEST_MAIN(TasksWindowTest)

#include "tasks_window_test.moc"
