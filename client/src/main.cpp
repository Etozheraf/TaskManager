#include <QApplication>
#include <QFile>
#include <QNetworkAccessManager>

#include "main_window.h"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  Q_INIT_RESOURCE(resources);

  if (QFile qss(":/styles.qss"); qss.open(QFile::ReadOnly)) {
    const QString style = QString::fromUtf8(qss.readAll());
    app.setStyleSheet(style);
  }

  QApplication::setWindowIcon(QIcon(":/images/logo.png"));

  const QUrl url("http://localhost:8080/");
  MainWindow window(url);
  window.setWindowTitle(QStringLiteral(" "));

  window.show();
  return QApplication::exec();
}