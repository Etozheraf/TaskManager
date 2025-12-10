#include <QApplication>
#include <QVBoxLayout>
#include <QtTest/QtTest>

#include "elided_label.h"
#include "elided_label_test.h"

static QWidget* makeContainerWith(ElidedLabel* label, int width) {
  auto* container = new QWidget;
  auto* layout = new QVBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(label);
  container->resize(width, 200);
  container->show();
  QTest::qWait(10);
  return container;
}

void ElidedLabelTest::singleLineElide_whenTooNarrow_addsEllipsis() {
  ElidedLabel* label = new ElidedLabel(
      "This is a very very long title text that should elide", nullptr);
  label->setMaxLines(1);
  std::unique_ptr<QWidget> container(makeContainerWith(label, 80));
  QVERIFY(label->text().contains(QChar(0x2026)) ||
          label->text().endsWith("..."));
}

void ElidedLabelTest::multiLineElide_respectsMaxLinesAndAddsEllipsisOnLast() {
  QString longText(200, 'A');
  ElidedLabel* label = new ElidedLabel(longText, nullptr);
  label->setMaxLines(2);
  std::unique_ptr<QWidget> container(makeContainerWith(label, 120));
  const QStringList lines = label->text().split('\n');
  QVERIFY(lines.size() <= 2);
  if (lines.size() == 2) {
    QVERIFY(lines.back().contains(QChar(0x2026)) ||
            lines.back().endsWith("..."));
  }
}

void ElidedLabelTest::reactsToResize_smallerWidthProducesMoreElision() {
  const QString base = "Some moderately long text to elide on resize";
  auto* label = new ElidedLabel(base, nullptr);
  label->setMaxLines(1);
  std::unique_ptr<QWidget> container(makeContainerWith(label, 200));
  const QString at200 = label->text();
  container->resize(80, 200);
  QTest::qWait(10);
  const QString at80 = label->text();
  QVERIFY(at80.size() < at200.size());
}

void ElidedLabelTest::singleLineElide_resultContainsNoNewlines() {
  auto* label = new ElidedLabel("Line1 Line2 Line3", nullptr);
  label->setMaxLines(1);
  std::unique_ptr<QWidget> container(makeContainerWith(label, 80));
  QVERIFY(!label->text().contains('\n'));
}

void ElidedLabelTest::growWidth_reducesElisionOrRemovesEllipsis() {
  const QString text = "Some text that might be elided";
  auto* label = new ElidedLabel(text, nullptr);
  label->setMaxLines(1);
  std::unique_ptr<QWidget> container(makeContainerWith(label, 80));
  const QString narrow = label->text();
  container->resize(400, 200);
  QTest::qWait(10);
  const QString wide = label->text();
  QVERIFY(wide.size() >= narrow.size());
}

void ElidedLabelTest::elideModeMiddle_keepsEndOfString() {
  const QString text = "Prefix-Content-Suffix";
  auto* label = new ElidedLabel(text, nullptr);
  label->setMaxLines(1);
  label->setElideMode(Qt::ElideMiddle);
  std::unique_ptr<QWidget> container(makeContainerWith(label, 80));
  const QString shown = label->text();
  QVERIFY(shown.contains("Prefix") && shown.contains("Suffix"));
}

void ElidedLabelTest::updatesOnSetText_recalculatesDisplay() {
  auto* label = new ElidedLabel("Short", nullptr);
  label->setMaxLines(1);
  std::unique_ptr<QWidget> container(makeContainerWith(label, 60));
  const QString first = label->text();
  label->setText("A much longer string that should be elided now");
  QTest::qWait(10);
  const QString second = label->text();
  QVERIFY(second.size() <= first.size() || second.contains(QChar(0x2026)) || second.endsWith("..."));
}

QTEST_MAIN(ElidedLabelTest)
