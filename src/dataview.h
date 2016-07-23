#ifndef DATAVIEW_H
#define DATAVIEW_H

#include "qcustomplot.h"

class MainWindow;

class DataView : public QCustomPlot
{
    Q_OBJECT

public:
    typedef enum {
        Top = 0,
        Left,
        Front
    } Direction;

    explicit DataView(QWidget *parent = 0);

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow) { mMainWindow = mainWindow; }
    void setDirection(Direction direction) { mDirection = direction; }

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    MainWindow *mMainWindow;

    Direction   mDirection;

    QPoint      m_topViewBeginPos;
    bool        m_topViewPan;

    QVector< QCPGraph* >  m_cursors;

    void setViewRange(double xMin, double xMax,
                      double yMin, double yMax);
    void addNorthArrow();

public slots:
    void updateView();
    void updateCursor();
};

#endif // DATAVIEW_H
