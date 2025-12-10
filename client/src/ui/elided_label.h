 #pragma once
 
 #include <QLabel>
 #include <QTextLayout>
 
 class ElidedLabel final : public QLabel {
   Q_OBJECT
  public:
   explicit ElidedLabel(const QString& text = QString(), QWidget* parent = nullptr);
 
   void setElideMode(Qt::TextElideMode mode);
   Qt::TextElideMode elideMode() const;
 
   void setMaxLines(int maxLines);
   int maxLines() const;
 
   void setText(const QString& text);
   QString fullText() const;
 
  protected:
   void resizeEvent(QResizeEvent* event) override;
   QSize minimumSizeHint() const override;
 
  private:
   void updateElide();
 
   QString full_text_;
   Qt::TextElideMode elide_mode_ = Qt::ElideRight;
   int max_lines_ = 1;
 };
 

