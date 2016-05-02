#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QStack>
#include <QVector>

#include "dataplot.h"
#include "datapoint.h"
#include "dataview.h"
#include "genome.h"

class QCPRange;
class QCustomPlot;
class ScoringMethod;
class ScoringView;

namespace Ui {
class MainWindow;
}

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
        Pan, Zoom, Measure, Zero, Ground, Course
    } Tool;

    typedef enum {
        Actual, Optimal
    } WindowMode;

    typedef enum {
        Time, Distance, HorizontalSpeed, VerticalSpeed
    } OptimizationMode;

    typedef enum {
        PPC, Speed, smLast
    } ScoringMode;

    typedef enum {
        Automatic, Fixed
    } GroundReference;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    const QVector< DataPoint > &data() const { return m_data; }
    int dataSize() const { return m_data.size(); }
    const DataPoint &dataPoint(int i) const { return m_data[i]; }

    PlotValue::Units units() const { return m_units; }

    void setRange(double lower, double upper);
    double rangeLower() const { return mZoomLevel.rangeLower; }
    double rangeUpper() const { return mZoomLevel.rangeUpper; }

    void setZero(double t);
    void setGround(double t);
    void setCourse(double t);

    void setTool(Tool tool);
    Tool tool() const { return mTool; }

    double markStart() const { return mMarkStart; }
    double markEnd() const { return mMarkEnd; }
    bool markActive() const { return mMarkActive; }

    void setRotation(double rotation);
    double rotation() const { return m_viewDataRotation; }

    int waypointSize() const { return m_waypoints.size(); }
    const DataPoint &waypoint(int i) const { return m_waypoints[i]; }

    static double getDistance(const DataPoint &dp1, const DataPoint &dp2);
    static double getBearing(const DataPoint &dp1, const DataPoint &dp2);

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

    double minDrag() const { return m_minDrag; }
    double maxLift() const { return m_maxLift; }
    double maxLD() const { return m_maxLD; }

    void setMinDrag(double minDrag);
    void setMaxLift(double maxLift);
    void setMaxLD(double maxLD);

    const QVector< DataPoint > &optimal() const { return m_optimal; }
    int optimalSize() const { return m_optimal.size(); }
    const DataPoint &optimalPoint(int i) const { return m_optimal[i]; }

    void optimize();

    DataPlot *plotArea() const;

    void setLineThickness(double width);
    double lineThickness() const { return mLineThickness; }

    void setWind(double windE, double windN);
    bool windAdjustment() const { return mWindAdjustment; }

    void setScoringMode(ScoringMode mode);
    ScoringMode scoringMode() const { return mScoringMode; }
    ScoringMethod *scoringMethod(int i) const { return mScoringMethods[i]; }

    bool getWindowBounds(const QVector< DataPoint > result, DataPoint &dpBottom, DataPoint &dpTop);

    void prepareDataPlot(DataPlot *plot);

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
    void on_actionAcceleration_triggered();
    void on_actionTotalEnergy_triggered();
    void on_actionEnergyRate_triggered();
    void on_actionLift_triggered();
    void on_actionDrag_triggered();
    void on_actionCourse_triggered();
    void on_actionCourseRate_triggered();
    void on_actionCourseAccuracy_triggered();

    void on_actionPan_triggered();
    void on_actionZoom_triggered();
    void on_actionMeasure_triggered();
    void on_actionZero_triggered();
    void on_actionGround_triggered();
    void on_actionSetCourse_triggered();
    void on_actionWind_triggered();

    void on_actionTime_triggered();
    void on_actionDistance2D_triggered();
    void on_actionDistance3D_triggered();

    void on_actionImportGates_triggered();
    void on_actionPreferences_triggered();

    void on_actionImportVideo_triggered();
    void on_actionExportKML_triggered();
    void on_actionExportPlot_triggered();
    void on_actionExportTrack_triggered();

    void on_actionUndoZoom_triggered();
    void on_actionRedoZoom_triggered();

private:
    typedef struct {
        double rangeLower;
        double rangeUpper;
    } ZoomLevel;

    Ui::MainWindow       *m_ui;
    QVector< DataPoint >  m_data;
    QVector< DataPoint >  m_optimal;

    double                mMarkStart;
    double                mMarkEnd;
    bool                  mMarkActive;

    double                m_viewDataRotation;

    PlotValue::Units      m_units;

    QVector< DataPoint >  m_waypoints;

    Tool                  mTool;
    Tool                  mPrevTool;

    ZoomLevel             mZoomLevel;
    QStack< ZoomLevel >   mZoomLevelUndo;
    QStack< ZoomLevel >   mZoomLevelRedo;

    double                mRangeLower;
    double                mRangeUpper;

    double                mWindowBottom;
    double                mWindowTop;

    bool                  mIsWindowValid;

    DataPoint             mWindowBottomDP;
    DataPoint             mWindowTopDP;

    WindowMode            mWindowMode;

    ScoringView          *mScoringView;

    QVector< ScoringMethod* > mScoringMethods;
    ScoringMode               mScoringMode;

    double                m_mass;
    double                m_planformArea;

    double                m_minDrag;
    double                m_minLift;
    double                m_maxLift;
    double                m_maxLD;

    int                   m_simulationTime;

    double                mLineThickness;

    double                mWindE, mWindN;
    bool                  mWindAdjustment;

    GroundReference       mGroundReference;
    double                mFixedReference;

    void writeSettings();
    void readSettings();

    void initPlot();
    void initViews();
    void initMapView();
    void initWindView();
    void initScoringView();
    void initLiftDragView();
    void initOrthoView();
    void initPlaybackView();

    void initSingleView(const QString &title, const QString &objectName,
                        QAction *actionShow, DataView::Direction direction);

    void initAltitude();
    void updateVelocity();
    void initAerodynamics();

    double getSlope(const int center, double (*value)(const DataPoint &)) const;

    void initRange();

    void updateBottomActions();
    void updateLeftActions();

    const Genome &selectGenome(const GenePool &genePool, const int tournamentSize);

signals:
    void dataLoaded();
    void dataChanged();
    void rotationChanged(double rotation);

private slots:
    void updateWindow();
    void setScoringVisible(bool visible);
};

#endif // MAINWINDOW_H
