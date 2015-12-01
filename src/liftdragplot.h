#ifndef LIFTDRAGPLOT_H
#define LIFTDRAGPLOT_H

#include "qcustomplot.h"

class MainWindow;

class LiftDragPlot : public QCustomPlot
{
    Q_OBJECT

public:
    explicit LiftDragPlot(QWidget *parent = 0);

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow) { mMainWindow = mainWindow; }

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    MainWindow *mMainWindow;

    QPoint      mBeginPos;
    bool        mDragging;

    void setMark(double mark);
    void setViewRange(double xMax, double yMax);

public slots:
    void updatePlot();
};

#endif // LIFTDRAGPLOT_H
