#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QMap>
#include <QSqlDatabase>
#include <QStack>
#include <QVector>

#include "dataplot.h"
#include "datapoint.h"
#include "dataview.h"

class MapView;
class QCPRange;
class QCustomPlot;
class ScoringMethod;
class ScoringView;

namespace Ui {
class MainWindow;
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
        PPC, Speed, Performance, WideOpenSpeed, WideOpenDistance, Flare, smLast
    } ScoringMode;

    typedef enum {
        Automatic, Fixed
    } GroundReference;

    typedef QVector< DataPoint > DataPoints;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    const DataPoints &data() const { return m_data; }
    int dataSize() const { return m_data.size(); }
    const DataPoint &dataPoint(int i) const { return m_data[i]; }

    PlotValue::Units units() const { return m_units; }

    void setRange(double lower, double upper, bool immediate = false);
    double rangeLower() const { return mZoomLevel.rangeLower; }
    double rangeUpper() const { return mZoomLevel.rangeUpper; }

    void setZero(double t);
    void setGround(double t);
    void setCourse(double t);

    void setTrackGround(QString trackName, double ground);
    void setTrackWindSpeed(QString trackName, double windSpeed);
    void setTrackWindDir(QString trackName, double windDIr);

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
    int findIndexForLanding();

    void setWindowMode(WindowMode mode);
    WindowMode windowMode() const { return mWindowMode; }

    double mass() const { return m_mass; }
    double planformArea() const { return m_planformArea; }

    double minDrag() const { return m_minDrag; }
    double minLift() const { return m_minLift; }
    double maxLift() const { return m_maxLift; }
    double maxLD() const { return m_maxLD; }

    int simulationTime() const { return m_simulationTime; }

    void setMinDrag(double minDrag);
    void setMaxLift(double maxLift);
    void setMaxLD(double maxLD);    

    const DataPoints &optimal() const { return m_optimal; }
    void setOptimal(const DataPoints &result);

    int optimalSize() const { return m_optimal.size(); }
    const DataPoint &optimalPoint(int i) const { return m_optimal[i]; }

    DataPlot *plotArea() const;

    void setLineThickness(double width);
    double lineThickness() const { return mLineThickness; }

    void setWind(double windE, double windN);
    void getWind(QString trackName, double *windE, double *windN);
    void getWindSpeedDirection(QString trackName, double *windSpeed, double *windDirection);
    bool windAdjustment() const { return mWindAdjustment; }

    double getQNE(void) const { return mFixedReference;}

    void setScoringMode(ScoringMode mode);
    ScoringMode scoringMode() const { return mScoringMode; }
    ScoringMethod *scoringMethod(int i) const { return mScoringMethods[i]; }

    void prepareDataPlot(DataPlot *plot);
    void prepareMapView(MapView *plot);

    bool updateReference(double lat, double lon);
    void closeReference();

    void importFromDatabase(const QString &uniqueName);
    void importFromCheckedTrack(const QString &uniqueName);

    void setTrackName(const QString &trackName);
    QString trackName() const { return mTrackName; }

    void setSelectedTracks(QVector< QString > tracks);
    void setTrackDescription(const QString &trackName, const QString &description);

    void setTrackChecked(const QString &trackName, bool checked);
    bool trackChecked(const QString &trackName) const;

    QString databasePath() const { return mDatabasePath; }

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_actionImport_triggered();
    void on_actionImportFolder_triggered();

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
    void on_actionZoomToExtent_triggered();

    void on_actionDeleteTrack_triggered();

private:
    typedef struct {
        double rangeLower;
        double rangeUpper;
    } ZoomLevel;

    Ui::MainWindow       *m_ui;
    DataPoints            m_data;
    DataPoints            m_optimal;

    double                mMarkStart;
    double                mMarkEnd;
    bool                  mMarkActive;

    double                m_viewDataRotation;

    PlotValue::Units      m_units;

    DataPoints            m_waypoints;

    Tool                  mTool;
    Tool                  mPrevTool;

    ZoomLevel             mZoomLevel;
    ZoomLevel             mZoomLevelPrev;
    QStack< ZoomLevel >   mZoomLevelUndo;
    QStack< ZoomLevel >   mZoomLevelRedo;

    double                mRangeLower;
    double                mRangeUpper;

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

    QString               mDatabasePath;
    QSqlDatabase          mDatabase;

    QString               mTrackName;
    QVector< QString >    mSelectedTracks;
    QMap< QString, DataPoints > mCheckedTracks;

    QTimer               *zoomTimer;

    void writeSettings();
    void readSettings();

    void initDatabase();
    bool setDatabaseValue(QString trackName, QString column, QString value);
    bool getDatabaseValue(QString trackName, QString column, QString &value);
    void saveZoomToDatabase();

    void initPlot();
    void initViews();
    void initMapView();
    void initWindView();
    void initScoringView();
    void initLiftDragView();
    void initOrthoView();
    void initPlaybackView();
    void initLogbookView();

    void initSingleView(const QString &title, const QString &objectName,
                        QAction *actionShow, DataView::Direction direction);

    void import(QIODevice *device, DataPoints &data, QString trackName, bool initDatabase);
    void initTime(DataPoints &data, QString trackName, bool initDatabase);
    void initAltitude(DataPoints &data, QString trackName, bool initDatabase);
    void updateVelocity(DataPoints &data, QString trackName, bool initDatabase);
    void initAerodynamics(DataPoints &data);

    double getSlope(const int center, double (*value)(const DataPoint &)) const;

    void initRange(QString trackName);

    void updateBottomActions();
    void updateLeftActions();

    void updateGround(DataPoints &data, double ground);
    QString dateTimeToUTC(const QDateTime &dt);

signals:
    void dataLoaded();
    void dataChanged();
    void cursorChanged();
    void aeroChanged();
    void rotationChanged(double rotation);
    void databaseChanged();

public slots:
    void importFolder(QString folderName);
    void importFile(QString fileName);

private slots:
    void setScoringVisible(bool visible);
    void saveZoom();
};

#endif // MAINWINDOW_H
