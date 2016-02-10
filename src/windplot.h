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

    double mWindE, mWindN;
    double mVelAircraft;

    void setViewRange(double xMin, double xMax,
                      double yMin, double yMax);

    void updateWind(const int start, const int end);

public slots:
    void updatePlot();
    void save();
};

#endif // WINDPLOT_H
