#ifndef __qwwtipdialog_h
#define __qwwtipdialog_h

#if defined(WW_NO_TEXTBROWSER) || defined(WW_NO_PUSHBUTTON)
#define WW_NO_TIPWIDGET
#endif

#ifndef WW_NO_TIPWIDGET

#include <QWidget>
#include <QStringList>

class QAbstractItemModel;
class QTextBrowser;
class QPushButton;
class QCheckBox;
#include <QPersistentModelIndex>
#include <QFrame>
#include <wwglobal.h>

class QwwTipWidgetPrivate;
/**
 *
 *
 *
 */
class Q_WW_EXPORT QwwTipWidget : public QFrame, public QwwPrivatable {
  Q_OBJECT

  Q_PROPERTY(QStringList tips READ tips WRITE setTips)
  Q_PROPERTY(int currentTip READ currentTip WRITE setCurrentTip)
  Q_PROPERTY(QIcon defaultIcon READ defaultIcon WRITE setDefaultIcon)
  Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)
  Q_PROPERTY(Qt::Alignment iconAlignment READ iconAlignment WRITE setIconAlignment)
  Q_PROPERTY(bool checkVisible READ checkIsVisible WRITE setCheckVisible)
  Q_PROPERTY(bool closeVisible READ closeIsVisible WRITE setCloseVisible)
  Q_PROPERTY(bool tipsEnabled READ tipsEnabled WRITE setTipsEnabled)
public:
  QwwTipWidget(const QStringList &list, QWidget *parent=0);
  int currentTip() const;
  const QTextBrowser *tipCanvas() const;
  const QStringList tips() const;
  QIcon defaultIcon() const;
  QSize iconSize() const;
  Qt::Alignment iconAlignment() const;
  void setIconAlignment(Qt::Alignment);
  QWidget *headerWidget() const;
  void setHeaderWidget(QWidget *);
  const QPushButton *nextButton() const;
  const QPushButton *prevButton() const;
  const QPushButton *closeButton() const;
  bool checkIsVisible() const;
  bool closeIsVisible() const;

  QFrame::Shape canvasFrameShape() const;
  void setCanvasFrameShape(QFrame::Shape s);
  bool tipsEnabled() const;

public slots:
  void setTipsEnabled(bool v);
  void nextTip();
  void prevTip();
  void setTips(const QStringList &);
  void setCurrentTip(int);
  void setDefaultIcon(const QIcon &);
  void setIconSize(const QSize &);
  void setCheckVisible(bool);
  void setCheckHidden(bool);
  void setCloseVisible(bool);
  void setCloseHidden(bool);
signals:
  void tipChanged(int);
protected:
  void changeEvent(QEvent *);
private:
  WW_DECLARE_PRIVATE(QwwTipWidget);
  Q_PRIVATE_SLOT(d_func(), void showTip())
  Q_DISABLE_COPY(QwwTipWidget);
};


#endif
#endif
