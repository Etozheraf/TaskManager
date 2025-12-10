#include "elided_label.h"

ElidedLabel::ElidedLabel(const QString& text, QWidget* parent)
    : QLabel(text, parent), full_text_(text) {
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  setMinimumWidth(0);
  updateElide();
}

void ElidedLabel::setElideMode(const Qt::TextElideMode mode) {
  if (elide_mode_ == mode)
    return;
  elide_mode_ = mode;
  updateElide();
}

Qt::TextElideMode ElidedLabel::elideMode() const {
  return elide_mode_;
}

void ElidedLabel::setMaxLines(int maxLines) {
  if (maxLines < 1)
    maxLines = 1;
  if (max_lines_ == maxLines)
    return;
  max_lines_ = maxLines;
  updateElide();
}

int ElidedLabel::maxLines() const {
  return max_lines_;
}

void ElidedLabel::setText(const QString& text) {
  full_text_ = text;
  updateElide();
}

QString ElidedLabel::fullText() const {
  return full_text_;
}

void ElidedLabel::resizeEvent(QResizeEvent* event) {
  QLabel::resizeEvent(event);
  updateElide();
}

QSize ElidedLabel::minimumSizeHint() const {
  QSize s = QLabel::minimumSizeHint();
  s.setWidth(0);
  return s;
}

void ElidedLabel::updateElide() {
  const int availableWidth = width();
  if (availableWidth <= 0) {
    QLabel::setText(QString());
    setToolTip(QString());
    return;
  }

  const QFontMetrics fm(font());
  QString displayed;

  QTextOption option;
  option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
  QTextLayout layout(full_text_, font());
  layout.setTextOption(option);
  layout.beginLayout();

  int startPos = 0;
  for (int lineIndex = 0; lineIndex < max_lines_;) {
    QTextLine line = layout.createLine();
    if (!line.isValid())
      break;
    line.setLineWidth(availableWidth);
    const int length = line.textLength();
    const int nextPos = startPos + length;

    if (lineIndex == max_lines_ - 1 && nextPos < full_text_.size()) {
      const QString rest = full_text_.mid(startPos);
      displayed +=
          fm.elidedText(rest, elide_mode_, availableWidth, Qt::TextSingleLine);
      break;
    }
    displayed += full_text_.mid(startPos, length);
    startPos = nextPos;
    ++lineIndex;
    if (lineIndex < max_lines_)
      displayed += QLatin1Char('\n');
  }
  layout.endLayout();

  QLabel::setText(displayed);
  if (displayed != full_text_) {
    setToolTip(full_text_);
  } else {
    setToolTip(QString());
  }
}
