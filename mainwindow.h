#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QVector>

#include "datapoint.h"

class DataView;
class QCPRange;
class QCustomPlot;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_actionImport_triggered();
    void on_actionExit_triggered();

    void on_actionLeftElevation_triggered();
    void on_actionLeftVerticalSpeed_triggered();
    void on_actionLeftHorizontalSpeed_triggered();
    void on_actionLeftTotalSpeed_triggered();
    void on_actionLeftDiveAngle_triggered();
    void on_actionLeftCurvature_triggered();
    void on_actionLeftGlideRatio_triggered();

    void on_actionRightElevation_triggered();
    void on_actionRightVerticalSpeed_triggered();
    void on_actionRightHorizontalSpeed_triggered();
    void on_actionRightTotalSpeed_triggered();
    void on_actionRightDiveAngle_triggered();
    void on_actionRightCurvature_triggered();
    void on_actionRightGlideRatio_triggered();

    void on_actionMetric_triggered();
    void on_actionImperial_triggered();

    void on_actionTime_triggered();
    void on_actionDistance2D_triggered();
    void on_actionDistance3D_triggered();

    void onDataPlot_zoom(const QCPRange &range);
    void onDataPlot_pan(double xBegin, double xEnd);

    void onDataPlot_mark(double xMark);
    void onDataPlot_clear();    

    void onTopView_mousePress(QMouseEvent *event);
    void onTopView_mouseRelease(QMouseEvent *event);
    void onTopView_mouseMove(QMouseEvent *event);

    void onLeftView_mouseMove(QMouseEvent *event);
    void onFrontView_mouseMove(QMouseEvent *event);

    void on_actionImportGates_triggered();

    void onPlotArea_expand(QPoint pos, QPoint angleDelta);

private:
    typedef enum {
        Time,
        Distance2D,
        Distance3D
    } XAxisType;

    typedef enum {
        Elevation,
        VerticalSpeed,
        HorizontalSpeed,
        TotalSpeed,
        DiveAngle,
        Curvature,
        GlideRatio
    } YAxisType;

    typedef enum
    {
        Metric,
        Imperial
    } Units;

    Ui::MainWindow       *m_ui;
    QVector< DataPoint >  m_data;

    XAxisType             m_xAxis;
    YAxisType             m_yAxis1, m_yAxis2;

    QVector< QString >    m_xAxisTitlesMetric, m_xAxisTitlesImperial;
    QVector< QString >    m_yAxisTitlesMetric, m_yAxisTitlesImperial;

    double                m_xPlot, m_y1Plot, m_y2Plot;
    double                m_xView, m_yView, m_zView;
    bool                  m_markActive;

    QPoint                m_topViewBeginPos;
    bool                  m_topViewPan;
    double                m_viewDataRotation;

    Units                 m_units;

    QLabel                *m_statusLabel;

    QVector< DataPoint >  m_waypoints;

    double                m_timeStep;

    double getDistance(const DataPoint &dp1, const DataPoint &dp2) const;
    double getBearing(const DataPoint &dp1, const DataPoint &dp2) const;

    void initPlotData();
    void updatePlotData();
    void updateYRanges();

    void updateViewData();
    void setViewRange(QCustomPlot *plot,
                      double xMin, double xMax,
                      double yMin, double yMax);
    void addNorthArrow(QCustomPlot *plot);

    double getXValue(const DataPoint &dp, XAxisType axis);
    double getYValue(const DataPoint &dp, YAxisType axis);

    int findIndexBelowX(double x);
    int findIndexAboveX(double x);

    int findIndexBelowT(double t);
    int findIndexAboveT(double t);

    void onView_mouseMove(DataView *view, QMouseEvent *event);

    static double distSqrToLine(const QPointF &start, const QPointF &end,
                                const QPointF &point, double &mu);
};

#endif // MAINWINDOW_H
