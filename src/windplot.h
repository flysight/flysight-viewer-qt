#ifndef WINDPLOT_H
#define WINDPLOT_H

#include "qcustomplot.h"

class MainWindow;

class WindPlot : public QCustomPlot
{
    Q_OBJECT

public:
    explicit WindPlot(QWidget *parent = 0);

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow) { mMainWindow = mainWindow; }

protected:
    void mouseMoveEvent(QMouseEvent *event);

private:
    MainWindow *mMainWindow;

    void setViewRange(double xMin, double xMax,
                      double yMin, double yMax);

public slots:
    void updatePlot();
};

#endif // WINDPLOT_H
