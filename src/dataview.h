#ifndef DATAVIEW_H
#define DATAVIEW_H

#include "qcustomplot.h"

class MainWindow;

class DataView : public QCustomPlot
{
    Q_OBJECT

public:
    explicit DataView(QWidget *parent = 0);

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow) { mMainWindow = mainWindow; }

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    MainWindow *mMainWindow;

    static double distSqrToLine(const QPointF &start, const QPointF &end,
                                const QPointF &point, double &mu);

signals:
    void mark(double xMark);
    void clear();

public slots:
    
};

#endif // DATAVIEW_H
