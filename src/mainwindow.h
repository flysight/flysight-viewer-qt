#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QVector>

#include "dataplot.h"
#include "datapoint.h"
#include "dataview.h"
#include "plotvalue.h"

class QCPRange;
class QCustomPlot;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
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
        yaLast
    } YAxisType;

    typedef enum {
        Pan, Zoom, Measure, Zero, Ground
    } Tool;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    int dataSize() const { return m_data.size(); }
    const DataPoint &dataPoint(int i) const { return m_data[i]; }

    const PlotValue *xValue() const { return m_xValues[m_xAxis]; }
    const PlotValue *yValue(int i) const { return m_yValues[i]; }

    PlotValue::Units units() const { return m_units; }

    void setRange(double lower, double upper);
    double rangeLower() const { return mRangeLower; }
    double rangeUpper() const { return mRangeUpper; }

    void setZero(double t);
    void setGround(double t);

    void setTool(Tool tool);
    Tool tool() const { return mTool; }

    double markStart() const { return mMarkStart; }
    double markEnd() const { return mMarkEnd; }
    bool markActive() const { return mMarkActive; }

    void setRotation(double rotation);
    double rotation() const { return m_viewDataRotation; }

    int waypointSize() const { return m_waypoints.size(); }
    const DataPoint &waypoint(int i) const { return m_waypoints[i]; }

    double getDistance(const DataPoint &dp1, const DataPoint &dp2) const;
    double getBearing(const DataPoint &dp1, const DataPoint &dp2) const;

    void setMark(double start, double end);
    void setMark(double mark);
    void clearMark();

    DataPoint interpolateDataX(double x);
    DataPoint interpolateDataT(double t);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_actionImport_triggered();

    void on_actionElevation_triggered();
    void on_actionVerticalSpeed_triggered();
    void on_actionHorizontalSpeed_triggered();
    void on_actionTotalSpeed_triggered();
    void on_actionDiveAngle_triggered();
    void on_actionCurvature_triggered();
    void on_actionGlideRatio_triggered();
    void on_actionHorizontalAccuracy_triggered();
    void on_actionVerticalAccuracy_triggered();
    void on_actionSpeedAccuracy_triggered();
    void on_actionNumberOfSatellites_triggered();

    void on_actionPan_triggered();
    void on_actionZoom_triggered();
    void on_actionMeasure_triggered();
    void on_actionZero_triggered();
    void on_actionGround_triggered();

    void on_actionTime_triggered();
    void on_actionDistance2D_triggered();
    void on_actionDistance3D_triggered();

    void on_actionImportGates_triggered();
    void on_actionPreferences_triggered();

private:
    Ui::MainWindow       *m_ui;
    QVector< DataPoint >  m_data;

    QVector< PlotValue* > m_xValues;
    XAxisType             m_xAxis;

    QVector< PlotValue* > m_yValues;

    double                m_xPlot, m_yPlot[yaLast];
    double                m_xView, m_yView, m_zView;

    double                mMarkStart;
    double                mMarkEnd;
    bool                  mMarkActive;

    double                m_viewDataRotation;

    PlotValue::Units      m_units;

    QVector< DataPoint >  m_waypoints;

    double                m_timeStep;

    Tool                  mTool;
    Tool                  mPrevTool;

    double                mRangeLower;
    double                mRangeUpper;

    void writeSettings();
    void readSettings();

    void initPlot();
    void initViews();
    void initSingleView(const QString &title, const QString &objectName,
                        QAction *actionShow, DataView::Direction direction);

    double getSlope(const int center, YAxisType yAxis) const;

    void initPlotData();

    double getXValue(const DataPoint &dp, XAxisType axis);

    int findIndexBelowX(double x);
    int findIndexAboveX(double x);

    int findIndexBelowT(double t);
    int findIndexAboveT(double t);

    void updateTool();
    void updateBottom(XAxisType xAxis);

    void updateBottomActions();
    void updateLeftActions();

signals:
    void rangeChanged(double lower, double upper);
    void dataChanged();
    void plotChanged();
    void rotationChanged(double rotation);
};

#endif // MAINWINDOW_H
