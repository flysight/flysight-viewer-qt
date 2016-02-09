#ifndef DATAPLOT_H
#define DATAPLOT_H

#include "datapoint.h"
#include "plotvalue.h"
#include "qcustomplot.h"

class MainWindow;

class DataPlot : public QCustomPlot
{
    Q_OBJECT

public:
    typedef enum {
        Time = 0,
        Distance2D,
        Distance3D
    } XAxisType;

    typedef enum {
        Elevation = 0,
        VerticalSpeed,
        HorizontalSpeed,
        TotalSpeed,
        DiveAngle,
        Curvature,
        GlideRatio,
        HorizontalAccuracy,
        VerticalAccuracy,
        SpeedAccuracy,
        NumberOfSatellites,
        Acceleration,
        TotalEnergy,
        EnergyRate,
        Lift,
        Drag,
        Course,
        CourseRate,
        CourseAccuracy,
        yaLast
    } YAxisType;

    explicit DataPlot(QWidget *parent = 0);
    ~DataPlot();

    void setMainWindow(MainWindow *mainWindow) { mMainWindow = mainWindow; }

    PlotValue *xValue() const { return m_xValues[m_xAxisType]; }
    PlotValue *yValue(int i) const { return m_yValues[i]; }

    void togglePlot(YAxisType plot);
    bool plotVisible(YAxisType plot) const { return m_yValues[plot]->visible(); }

    void setXAxisType(XAxisType xAxisType);
    XAxisType xAxisType() const { return m_xAxisType ; }

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void wheelEvent(QWheelEvent *event);

    void leaveEvent(QEvent *);

    void paintEvent(QPaintEvent *event);

private:
    QPoint m_cursorPos;
    QPoint m_beginPos;

    bool m_dragging;

    MainWindow *mMainWindow;

    QVector< PlotValue* > m_xValues;
    XAxisType             m_xAxisType;

    QVector< PlotValue* > m_yValues;

    void updateYRanges();
    void setRange(const QCPRange &range);

    void setMark(double start, double end);
    void setMark(double mark);

    DataPoint interpolateDataX(double x);

    int findIndexBelowX(double x);
    int findIndexAboveX(double x);

    void initPlot();

    void readSettings();
    void writeSettings();

public slots:
    void updatePlot();
};

#endif // DATAPLOT_H
