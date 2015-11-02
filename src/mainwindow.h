#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QVector>

#include "dataplot.h"
#include "datapoint.h"
#include "dataview.h"

class QCPRange;
class QCustomPlot;
class WingsuitView;

namespace Ui {
class MainWindow;
}

typedef QVector< double > Genome;
typedef QPair< double, Genome > Score;
typedef QVector< Score > GenePool;

static bool operator<(const Score &s1, const Score &s2)
{
    return s1.first > s2.first;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    typedef enum {
        Pan, Zoom, Measure, Zero, Ground
    } Tool;

    typedef enum {
        Actual, Optimal
    } WindowMode;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    int dataSize() const { return m_data.size(); }
    const DataPoint &dataPoint(int i) const { return m_data[i]; }

    PlotValue::Units units() const { return m_units; }
    double dtWind() const { return m_dtWind; }

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

    DataPoint interpolateDataT(double t);

    int findIndexBelowT(double t);
    int findIndexAboveT(double t);

    void setWindow(double windowBottom, double windowTop);
    double windowTop(void) const { return mWindowTop; }
    double windowBottom(void) const { return mWindowBottom; }

    bool isWindowValid(void) const;
    const DataPoint &windowTopDP(void) const { return mWindowTopDP; }
    const DataPoint &windowBottomDP(void) const { return mWindowBottomDP; }

    void setWindowMode(WindowMode mode);
    WindowMode windowMode() const { return mWindowMode; }

    double planformArea() const { return m_planformArea; }
    double wingSpan() const { return m_wingSpan; }

    double minDrag() const { return m_minDrag; }
    double maxLift() const { return m_maxLift; }
    double efficiency() const { return m_efficiency; }

    void setMinDrag(double minDrag);
    void setEfficiency(double efficiency);

    int optimalSize() const { return m_optimal.size(); }
    const DataPoint &optimalPoint(int i) const { return m_optimal[i]; }

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
    void on_actionWindSpeed_triggered();
    void on_actionWindDirection_triggered();
    void on_actionAircraftSpeed_triggered();
    void on_actionWindError_triggered();
    void on_actionAcceleration_triggered();
    void on_actionTotalEnergy_triggered();
    void on_actionEnergyRate_triggered();
    void on_actionLift_triggered();
    void on_actionDrag_triggered();

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

    void on_actionImportVideo_triggered();
    void on_actionExportKML_triggered();
    void on_actionExportPlot_triggered();

    void on_actionOptimize_triggered();

private:
    Ui::MainWindow       *m_ui;
    QVector< DataPoint >  m_data;
    QVector< DataPoint >  m_optimal;

    double                mMarkStart;
    double                mMarkEnd;
    bool                  mMarkActive;

    double                m_viewDataRotation;

    PlotValue::Units      m_units;
    double                m_dtWind;

    QVector< DataPoint >  m_waypoints;

    double                m_timeStep;

    Tool                  mTool;
    Tool                  mPrevTool;

    double                mRangeLower;
    double                mRangeUpper;

    double                mWindowBottom;
    double                mWindowTop;

    bool                  mIsWindowValid;

    DataPoint             mWindowBottomDP;
    DataPoint             mWindowTopDP;

    WindowMode            mWindowMode;

    WingsuitView         *mWingsuitView;

    double                m_temperature;
    double                m_mass;
    double                m_planformArea;
    double                m_wingSpan;

    double                m_minDrag;
    double                m_maxLift;
    double                m_efficiency;

    void writeSettings();
    void readSettings();

    void initPlot();
    void initViews();
    void initMapView();
    void initWindView();
    void initWingsuitView();
    void initLiftDragView();

    void initSingleView(const QString &title, const QString &objectName,
                        QAction *actionShow, DataView::Direction direction);

    void initWind();
    void getWind(const int center);

    void initAerodynamics();

    double getSlope(const int center, double (*value)(const DataPoint &)) const;

    void initRange();

    void updateBottomActions();
    void updateLeftActions();

    void iterate(QVector< double > &aoa, int parts);
    double simulate(const QVector< double > &aoa, double h, double a, double c,
                    double t0, double theta0, double v0, double x0, double y0, int start);

    double dtheta_dt(double theta, double v, double x, double y, double lift);
    double dv_dt(double theta, double v, double x, double y, double drag);
    double dx_dt(double theta, double v, double x, double y);
    double dy_dt(double theta, double v, double x, double y);

    double lift(double aoa);
    double drag(double aoa, double a, double c);

signals:
    void dataLoaded();
    void dataChanged();
    void rotationChanged(double rotation);

private slots:
    void updateWindow();
    void setWingsuitVisible(bool visible);
};

#endif // MAINWINDOW_H
