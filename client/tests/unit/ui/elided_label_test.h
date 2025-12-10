 #pragma once
 
 #include <QObject>
 
 class ElidedLabelTest : public QObject {
   Q_OBJECT
  private slots:
   void singleLineElide_whenTooNarrow_addsEllipsis();
   void multiLineElide_respectsMaxLinesAndAddsEllipsisOnLast();
   void reactsToResize_smallerWidthProducesMoreElision();
   void singleLineElide_resultContainsNoNewlines();
   void growWidth_reducesElisionOrRemovesEllipsis();
   void elideModeMiddle_keepsEndOfString();
   void updatesOnSetText_recalculatesDisplay();
 };
 

