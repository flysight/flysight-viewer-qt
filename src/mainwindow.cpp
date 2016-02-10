#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSettings>
#include <QShortcut>
#include <QTextStream>

#include <math.h>

#include "common.h"
#include "configdialog.h"
#include "dataview.h"
#include "genome.h"
#include "liftdragplot.h"
#include "mapview.h"
#include "orthoview.h"
#include "playbackview.h"
#include "scoringview.h"
#include "videoview.h"
#include "windplot.h"

MainWindow::MainWindow(
        QWidget *parent):

    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    mMarkActive(false),
    m_viewDataRotation(0),
    m_units(PlotValue::Imperial),
    mWindowBottom(2000),
    mWindowTop(3000),
    mIsWindowValid(false),
    mWindowMode(Actual),
    mScoringView(0),
    m_mass(70),
    m_planformArea(2),
    m_minDrag(0.05),
    m_minLift(0.0),
    m_maxLift(0.5),
    m_maxLD(3.0),
    m_simulationTime(120),
    mLineThickness(0),
    mWindE(0),
    mWindN(0),
    mWindAdjustment(false)
{
    m_ui->setupUi(this);

    // Ensure that closeEvent is called
    connect(m_ui->actionExit, SIGNAL(triggered()),
            this, SLOT(close()));

    // Respond to data changed signal
    connect(this, SIGNAL(dataChanged()),
            this, SLOT(updateWindow()));

    // Intitialize plot area
    initPlot();

    // Initialize 3D views
    initViews();
    initOrthoView();

    // Initialize map view
    initMapView();

    // Initialize wind view
    initWindView();

    // Initialize scoring view
    initScoringView();

    // Initialize lift/drag view
    initLiftDragView();

    // Initialize playback controls
    initPlaybackView();

    // Restore window state
    readSettings();

    // Set default tool
    setTool(Pan);

    // Redraw plots
    emit dataChanged();
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::writeSettings()
{
    QSettings settings("FlySight", "Viewer");

    settings.beginGroup("mainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("units", m_units);
    settings.setValue("mass", m_mass);
    settings.setValue("planformArea", m_planformArea);
    settings.setValue("minDrag", m_minDrag);
    settings.setValue("minLift", m_minLift);
    settings.setValue("maxLift", m_maxLift);
    settings.setValue("maxLD", m_maxLD);
    settings.setValue("simulationTime", m_simulationTime);
    settings.setValue("lineThickness", mLineThickness);
    settings.setValue("windE", mWindE);
    settings.setValue("windN", mWindN);
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings("FlySight", "Viewer");

    settings.beginGroup("mainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    m_units = (PlotValue::Units) settings.value("units", m_units).toInt();
    m_mass = settings.value("mass", m_mass).toDouble();
    m_planformArea = settings.value("planformArea", m_planformArea).toDouble();
    m_minDrag = settings.value("minDrag", m_minDrag).toDouble();
    m_minLift = settings.value("minLift", m_minLift).toDouble();
    m_maxLift = settings.value("maxLift", m_maxLift).toDouble();
    m_maxLD = settings.value("maxLD", m_maxLD).toDouble();
    m_simulationTime = settings.value("simulationTime", m_simulationTime).toInt();
    mLineThickness = settings.value("lineThickness", mLineThickness).toDouble();
    mWindE = settings.value("windE", mWindE).toDouble();
    mWindN = settings.value("windN", mWindN).toDouble();
    settings.endGroup();
}

void MainWindow::initPlot()
{
    updateBottomActions();
    updateLeftActions();

    m_ui->plotArea->setMainWindow(this);

    connect(this, SIGNAL(dataChanged()),
            m_ui->plotArea, SLOT(updatePlot()));
}

void MainWindow::initViews()
{
    initSingleView(tr("Top View"), "topView", m_ui->actionShowTopView, DataView::Top);
    initSingleView(tr("Side View"), "leftView", m_ui->actionShowLeftView, DataView::Left);
    initSingleView(tr("Front View"), "frontView", m_ui->actionShowFrontView, DataView::Front);
}

void MainWindow::initSingleView(
        const QString &title,
        const QString &objectName,
        QAction *actionShow,
        DataView::Direction direction)
{
    DataView *dataView = new DataView;
    QDockWidget *dockWidget = new QDockWidget(title);
    dockWidget->setWidget(dataView);
    dockWidget->setObjectName(objectName);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    dataView->setMainWindow(this);
    dataView->setDirection(direction);

    connect(actionShow, SIGNAL(toggled(bool)),
            dockWidget, SLOT(setVisible(bool)));
    connect(dockWidget, SIGNAL(visibilityChanged(bool)),
            actionShow, SLOT(setChecked(bool)));

    connect(this, SIGNAL(dataChanged()),
            dataView, SLOT(updateView()));
    connect(this, SIGNAL(rotationChanged(double)),
            dataView, SLOT(updateView()));
}

void MainWindow::initMapView()
{
    MapView *mapView = new MapView;
    QDockWidget *dockWidget = new QDockWidget(tr("Map View"));
    dockWidget->setWidget(mapView);
    dockWidget->setObjectName("mapView");
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    mapView->setMainWindow(this);

    connect(m_ui->actionShowMapView, SIGNAL(toggled(bool)),
            dockWidget, SLOT(setVisible(bool)));
    connect(dockWidget, SIGNAL(visibilityChanged(bool)),
            m_ui->actionShowMapView, SLOT(setChecked(bool)));

    connect(this, SIGNAL(dataLoaded()),
            mapView, SLOT(initView()));
    connect(this, SIGNAL(dataChanged()),
            mapView, SLOT(updateView()));
}

void MainWindow::initWindView()
{
    WindPlot *windPlot = new WindPlot;
    QDockWidget *dockWidget = new QDockWidget(tr("Wind View"));
    dockWidget->setWidget(windPlot);
    dockWidget->setObjectName("windView");
    dockWidget->setVisible(false);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    windPlot->setMainWindow(this);

    connect(m_ui->actionShowWindView, SIGNAL(toggled(bool)),
            dockWidget, SLOT(setVisible(bool)));
    connect(dockWidget, SIGNAL(visibilityChanged(bool)),
            m_ui->actionShowWindView, SLOT(setChecked(bool)));

    connect(this, SIGNAL(dataChanged()),
            windPlot, SLOT(updatePlot()));
}

void MainWindow::initScoringView()
{
    mScoringView = new ScoringView;
    QDockWidget *dockWidget = new QDockWidget(tr("Scoring"));
    dockWidget->setWidget(mScoringView);
    dockWidget->setObjectName("scoringView");
    dockWidget->setVisible(false);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    mScoringView->setMainWindow(this);

    connect(m_ui->actionShowScoringView, SIGNAL(toggled(bool)),
            dockWidget, SLOT(setVisible(bool)));
    connect(m_ui->actionShowScoringView, SIGNAL(toggled(bool)),
            this, SLOT(setScoringVisible(bool)));
    connect(dockWidget, SIGNAL(visibilityChanged(bool)),
            m_ui->actionShowScoringView, SLOT(setChecked(bool)));

    connect(this, SIGNAL(dataChanged()),
            mScoringView, SLOT(updateView()));
}

void MainWindow::initLiftDragView()
{
    LiftDragPlot *liftDragPlot = new LiftDragPlot;
    QDockWidget *dockWidget = new QDockWidget(tr("Drag Polar"));
    dockWidget->setWidget(liftDragPlot);
    dockWidget->setObjectName("liftDragView");
    dockWidget->setVisible(false);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    liftDragPlot->setMainWindow(this);

    connect(m_ui->actionShowLiftDragView, SIGNAL(toggled(bool)),
            dockWidget, SLOT(setVisible(bool)));
    connect(dockWidget, SIGNAL(visibilityChanged(bool)),
            m_ui->actionShowLiftDragView, SLOT(setChecked(bool)));

    connect(this, SIGNAL(dataChanged()),
            liftDragPlot, SLOT(updatePlot()));
}

void MainWindow::initOrthoView()
{
    OrthoView *orthoView = new OrthoView;
    QDockWidget *dockWidget = new QDockWidget(tr("Ortho View"));
    dockWidget->setWidget(orthoView);
    dockWidget->setObjectName("orthoView");
    dockWidget->setVisible(false);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    orthoView->setMainWindow(this);

    connect(m_ui->actionShowOrthoView, SIGNAL(toggled(bool)),
            dockWidget, SLOT(setVisible(bool)));
    connect(dockWidget, SIGNAL(visibilityChanged(bool)),
            m_ui->actionShowOrthoView, SLOT(setChecked(bool)));

    connect(this, SIGNAL(dataChanged()),
            orthoView, SLOT(updateView()));
}

void MainWindow::initPlaybackView()
{
    PlaybackView *playbackView = new PlaybackView;
    QDockWidget *dockWidget = new QDockWidget(tr("Playback View"));
    dockWidget->setWidget(playbackView);
    dockWidget->setObjectName("playbackView");
    dockWidget->setFloating(true);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    playbackView->setMainWindow(this);

    connect(m_ui->actionShowPlaybackView, SIGNAL(toggled(bool)),
            dockWidget, SLOT(setVisible(bool)));
    connect(dockWidget, SIGNAL(visibilityChanged(bool)),
            m_ui->actionShowPlaybackView, SLOT(setChecked(bool)));

    connect(this, SIGNAL(dataChanged()),
            playbackView, SLOT(updateView()));
}

void MainWindow::closeEvent(
        QCloseEvent *event)
{
    // Save window state
    writeSettings();

    // Okay to close
    event->accept();
}

DataPoint MainWindow::interpolateDataT(
        double t)
{
    const int i1 = findIndexBelowT(t);
    const int i2 = findIndexAboveT(t);

    if (i1 < 0)
    {
        return m_data.first();
    }
    else if (i2 >= m_data.size())
    {
        return m_data.last();
    }
    else
    {
        const DataPoint &dp1 = m_data[i1];
        const DataPoint &dp2 = m_data[i2];
        return DataPoint::interpolate(dp1, dp2, (t - dp1.t) / (dp2.t - dp1.t));
    }
}

int MainWindow::findIndexBelowT(
        double t)
{
    int below = -1;
    int above = m_data.size();

    while (below + 1 != above)
    {
        int mid = (below + above) / 2;
        const DataPoint &dp = m_data[mid];

        if (dp.t < t) below = mid;
        else          above = mid;
    }

    return below;
}

int MainWindow::findIndexAboveT(
        double t)
{
    int below = -1;
    int above = m_data.size();

    while (below + 1 != above)
    {
        int mid = (below + above) / 2;
        const DataPoint &dp = m_data[mid];

        if (dp.t > t) above = mid;
        else          below = mid;
    }

    return above;
}

void MainWindow::on_actionImport_triggered()
{
    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get last file read
    QString rootFolder = settings.value("folder").toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Track"), rootFolder, tr("CSV Files (*.csv)"));

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        // TODO: Error message
        return;
    }

    // Remember last file read
    settings.setValue("folder", QFileInfo(fileName).absoluteFilePath());

    QTextStream in(&file);

    // Column enumeration
    typedef enum {
        Time = 0,
        Lat,
        Lon,
        HMSL,
        VelN,
        VelE,
        VelD,
        HAcc,
        VAcc,
        SAcc,
        Heading,
        CAcc,
        NumSV
    } Columns;

    // Read column labels
    QMap< int, int > colMap;
    if (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList cols = line.split(",");

        for (int i = 0; i < cols.size(); ++i)
        {
            const QString &s = cols[i];

            if (s == "time")    colMap[Time]    = i;
            if (s == "lat")     colMap[Lat]     = i;
            if (s == "lon")     colMap[Lon]     = i;
            if (s == "hMSL")    colMap[HMSL]    = i;
            if (s == "velN")    colMap[VelN]    = i;
            if (s == "velE")    colMap[VelE]    = i;
            if (s == "velD")    colMap[VelD]    = i;
            if (s == "hAcc")    colMap[HAcc]    = i;
            if (s == "vAcc")    colMap[VAcc]    = i;
            if (s == "sAcc")    colMap[SAcc]    = i;
            if (s == "numSV")   colMap[NumSV]   = i;
        }
    }

    // Skip next row
    if (!in.atEnd()) in.readLine();

    m_data.clear();

    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList cols = line.split(",");

        DataPoint pt;

        pt.dateTime = QDateTime::fromString(cols[colMap[Time]], Qt::ISODate);

        pt.hasGeodetic = true;

        pt.lat   = cols[colMap[Lat]].toDouble();
        pt.lon   = cols[colMap[Lon]].toDouble();
        pt.hMSL  = cols[colMap[HMSL]].toDouble();

        pt.velN  = cols[colMap[VelN]].toDouble();
        pt.velE  = cols[colMap[VelE]].toDouble();
        pt.velD  = cols[colMap[VelD]].toDouble();

        pt.hAcc  = cols[colMap[HAcc]].toDouble();
        pt.vAcc  = cols[colMap[VAcc]].toDouble();
        pt.sAcc  = cols[colMap[SAcc]].toDouble();

        pt.numSV = cols[colMap[NumSV]].toDouble();

        m_data.append(pt);
    }

    double dist2D = 0, dist3D = 0;

    QVector< double > dt;

    for (int i = 0; i < m_data.size(); ++i)
    {
        const DataPoint &dp0 = m_data[m_data.size() - 1];
        DataPoint &dp = m_data[i];

        double distance = getDistance(dp0, dp);
        double bearing = getBearing(dp0, dp);

        dp.x = distance * sin(bearing);
        dp.y = distance * cos(bearing);
        dp.z = dp.alt = dp.hMSL - dp0.hMSL;

        qint64 start = dp0.dateTime.toMSecsSinceEpoch();
        qint64 end = dp.dateTime.toMSecsSinceEpoch();

        dp.t = (double) (end - start) / 1000;

        if (i > 0)
        {
            const DataPoint &dpPrev = m_data[i - 1];

            double dh = getDistance(dpPrev, dp);
            double dz = dp.hMSL - dpPrev.hMSL;

            dist2D += dh;
            dist3D += sqrt(dh * dh + dz * dz);

            dt.append(dp.t - dpPrev.t);
        }

        dp.dist2D = dist2D;
        dp.dist3D = dist3D;
    }

    // Wind-adjusted velocity
    updateVelocity();

    if (dt.size() > 0)
    {
        qSort(dt.begin(), dt.end());
        m_timeStep = dt.at(dt.size() / 2);
    }
    else
    {
        m_timeStep = 1.0;
    }

    // Clear optimum
    m_optimal.clear();

    initRange();

    emit dataLoaded();
}

void MainWindow::updateVelocity()
{
    if (mWindAdjustment)
    {
        // Wind-adjusted velocity
        for (int i = 0; i < m_data.size(); ++i)
        {
            DataPoint &dp = m_data[i];

            dp.vx = dp.velE - mWindE;
            dp.vy = dp.velN - mWindN;
        }
    }
    else
    {
        // Unadjusted velocity
        for (int i = 0; i < m_data.size(); ++i)
        {
            DataPoint &dp = m_data[i];

            dp.vx = dp.velE;
            dp.vy = dp.velN;
        }
    }

    // Cumulative heading
    double prevHeading;
    bool firstHeading = true;

    for (int i = 0; i < m_data.size(); ++i)
    {
        DataPoint &dp = m_data[i];

        // Calculate heading
        dp.heading = atan2(dp.vx, dp.vy) / PI * 180;

        // Calculate heading accuracy
        const double s = DataPoint::totalSpeed(dp);
        if (s != 0) dp.cAcc = dp.sAcc / s;
        else        dp.cAcc = 0;

        // Adjust heading
        if (!firstHeading)
        {
            while (dp.heading <  prevHeading - 180) dp.heading += 360;
            while (dp.heading >= prevHeading + 180) dp.heading -= 360;
        }

        firstHeading = false;
        prevHeading = dp.heading;
    }

    // Parameters depending on velocity
    for (int i = 0; i < m_data.size(); ++i)
    {
        DataPoint &dp = m_data[i];

        dp.curv = getSlope(i, DataPoint::diveAngle);
        dp.accel = getSlope(i, DataPoint::totalSpeed);
        dp.omega = getSlope(i, DataPoint::course);
    }

    // Initialize aerodynamics
    initAerodynamics();
}

void MainWindow::initAerodynamics()
{
    for (int i = 0; i < m_data.size(); ++i)
    {
        DataPoint &dp = m_data[i];

        // Acceleration
        double accelN = getSlope(i, DataPoint::northSpeed);
        double accelE = getSlope(i, DataPoint::eastSpeed);
        double accelD = getSlope(i, DataPoint::verticalSpeed);

        // Subtract acceleration due to gravity
        accelD -= A_GRAVITY;

        // Calculate acceleration due to drag
        const double vel = DataPoint::totalSpeed(dp);
        const double proj = (accelN * dp.vy + accelE * dp.vx + accelD * dp.velD) / vel;

        const double dragN = proj * dp.vy / vel;
        const double dragE = proj * dp.vx / vel;
        const double dragD = proj * dp.velD / vel;

        const double accelDrag = sqrt(dragN * dragN + dragE * dragE + dragD * dragD);

        // Calculate acceleration due to lift
        const double liftN = accelN - dragN;
        const double liftE = accelE - dragE;
        const double liftD = accelD - dragD;

        const double accelLift = sqrt(liftN * liftN + liftE * liftE + liftD * liftD);

        // From https://en.wikipedia.org/wiki/Atmospheric_pressure#Altitude_variation
        const double airPressure = SL_PRESSURE * pow(1 - LAPSE_RATE * dp.hMSL / SL_TEMP, A_GRAVITY * MM_AIR / GAS_CONST / LAPSE_RATE);

        // From https://en.wikipedia.org/wiki/Lapse_rate
        const double temperature = SL_TEMP - LAPSE_RATE * dp.hMSL;

        // From https://en.wikipedia.org/wiki/Density_of_air
        const double airDensity = airPressure / (GAS_CONST / MM_AIR) / temperature;

        // From https://en.wikipedia.org/wiki/Dynamic_pressure
        const double dynamicPressure = airDensity * vel * vel / 2;

        // Calculate lift and drag coefficients
        dp.lift = m_mass * accelLift / dynamicPressure / m_planformArea;
        dp.drag = m_mass * accelDrag / dynamicPressure / m_planformArea;
    }
}

double MainWindow::getSlope(
        const int center,
        double (*value)(const DataPoint &)) const
{
    int iMin = qMax (0, center - 2);
    int iMax = qMin (m_data.size () - 1, center + 2);

    double sumx = 0, sumy = 0, sumxx = 0, sumxy = 0;

    for (int i = iMin; i <= iMax; ++i)
    {
        const DataPoint &dp = m_data[i];
        double y = value(dp);

        sumx += dp.t;
        sumy += y;
        sumxx += dp.t * dp.t;
        sumxy += dp.t * y;
    }

    int n = iMax - iMin + 1;
    return (sumxy - sumx * sumy / n) / (sumxx - sumx * sumx / n);
}

double MainWindow::getDistance(
        const DataPoint &dp1,
        const DataPoint &dp2) const
{
    if (dp1.hasGeodetic && dp2.hasGeodetic)
    {
        const double R = 6371009;
        const double pi = 3.14159265359;

        double lat1 = dp1.lat / 180 * pi;
        double lon1 = dp1.lon / 180 * pi;

        double lat2 = dp2.lat / 180 * pi;
        double lon2 = dp2.lon / 180 * pi;

        double dLat = lat2 - lat1;
        double dLon = lon2 - lon1;

        double a = sin(dLat / 2) * sin(dLat / 2) +
                sin(dLon / 2) * sin(dLon / 2) * cos(lat1) * cos(lat2);
        double c = 2 * atan2(sqrt(a), sqrt(1 - a));

        return R * c;
    }
    else
    {
        const double dx = dp2.x - dp1.x;
        const double dy = dp2.y - dp1.y;

        return sqrt(dx * dx + dy * dy);
    }
}

double MainWindow::getBearing(
        const DataPoint &dp1,
        const DataPoint &dp2) const
{
    const double pi = 3.14159265359;

    double lat1 = dp1.lat / 180 * pi;
    double lon1 = dp1.lon / 180 * pi;

    double lat2 = dp2.lat / 180 * pi;
    double lon2 = dp2.lon / 180 * pi;

    double dLon = lon2 - lon1;

    double y = sin(dLon) * cos(lat2);
    double x = cos(lat1) * sin(lat2) -
            sin(lat1) * cos(lat2) * cos(dLon);

    return atan2(y, x);
}

void MainWindow::setMark(
        double start,
        double end)
{
    if (m_data.isEmpty()) return;

    if (start >= m_data.front().t &&
            start <= m_data.back().t &&
            end >= m_data.front().t &&
            end <= m_data.back().t)
    {
        mMarkStart = start;
        mMarkEnd = end;
        mMarkActive = true;
    }
    else
    {
        mMarkActive = false;
    }

    emit dataChanged();
}

void MainWindow::setMark(
        double mark)
{
    if (m_data.isEmpty()) return;

    if (mark >= m_data.front().t &&
            mark <= m_data.back().t)
    {
        mMarkStart = mMarkEnd = mark;
        mMarkActive = true;
    }
    else
    {
        mMarkActive = false;
    }

    emit dataChanged();
}

void MainWindow::clearMark()
{
    mMarkActive = false;

    emit dataChanged();
}

void MainWindow::initRange()
{
    double lower, upper;

    for (int i = 0; i < m_data.size(); ++i)
    {
        DataPoint &dp = m_data[i];

        if (i == 0)
        {
            lower = upper = dp.t;
        }
        else
        {
            if (dp.t < lower) lower = dp.t;
            if (dp.t > upper) upper = dp.t;
        }
    }

    setRange(lower, upper);
}

void MainWindow::on_actionElevation_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::Elevation);
}

void MainWindow::on_actionVerticalSpeed_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::VerticalSpeed);
}

void MainWindow::on_actionHorizontalSpeed_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::HorizontalSpeed);
}

void MainWindow::on_actionTime_triggered()
{
    m_ui->plotArea->setXAxisType(DataPlot::Time);
    updateBottomActions();
}

void MainWindow::on_actionDistance2D_triggered()
{
    m_ui->plotArea->setXAxisType(DataPlot::Distance2D);
    updateBottomActions();
}

void MainWindow::on_actionDistance3D_triggered()
{
    m_ui->plotArea->setXAxisType(DataPlot::Distance3D);
    updateBottomActions();
}

void MainWindow::updateBottomActions()
{
    m_ui->actionTime->setChecked(m_ui->plotArea->xAxisType() == DataPlot::Time);
    m_ui->actionDistance2D->setChecked(m_ui->plotArea->xAxisType() == DataPlot::Distance2D);
    m_ui->actionDistance3D->setChecked(m_ui->plotArea->xAxisType() == DataPlot::Distance3D);
}

void MainWindow::updateLeftActions()
{
    m_ui->actionElevation->setChecked(m_ui->plotArea->plotVisible(DataPlot::Elevation));
    m_ui->actionVerticalSpeed->setChecked(m_ui->plotArea->plotVisible(DataPlot::VerticalSpeed));
    m_ui->actionHorizontalSpeed->setChecked(m_ui->plotArea->plotVisible(DataPlot::HorizontalSpeed));
    m_ui->actionTotalSpeed->setChecked(m_ui->plotArea->plotVisible(DataPlot::TotalSpeed));
    m_ui->actionDiveAngle->setChecked(m_ui->plotArea->plotVisible(DataPlot::DiveAngle));
    m_ui->actionCurvature->setChecked(m_ui->plotArea->plotVisible(DataPlot::Curvature));
    m_ui->actionGlideRatio->setChecked(m_ui->plotArea->plotVisible(DataPlot::GlideRatio));
    m_ui->actionHorizontalAccuracy->setChecked(m_ui->plotArea->plotVisible(DataPlot::HorizontalAccuracy));
    m_ui->actionVerticalAccuracy->setChecked(m_ui->plotArea->plotVisible(DataPlot::VerticalAccuracy));
    m_ui->actionSpeedAccuracy->setChecked(m_ui->plotArea->plotVisible(DataPlot::SpeedAccuracy));
    m_ui->actionNumberOfSatellites->setChecked(m_ui->plotArea->plotVisible(DataPlot::NumberOfSatellites));
    m_ui->actionAcceleration->setChecked(m_ui->plotArea->plotVisible(DataPlot::Acceleration));
    m_ui->actionTotalEnergy->setChecked(m_ui->plotArea->plotVisible(DataPlot::TotalEnergy));
    m_ui->actionEnergyRate->setChecked(m_ui->plotArea->plotVisible(DataPlot::EnergyRate));
    m_ui->actionLift->setChecked(m_ui->plotArea->plotVisible(DataPlot::Lift));
    m_ui->actionDrag->setChecked(m_ui->plotArea->plotVisible(DataPlot::Drag));
    m_ui->actionCourse->setChecked(m_ui->plotArea->plotVisible(DataPlot::Course));
    m_ui->actionCourseRate->setChecked(m_ui->plotArea->plotVisible(DataPlot::CourseRate));
    m_ui->actionCourseAccuracy->setChecked(m_ui->plotArea->plotVisible(DataPlot::CourseAccuracy));
}

void MainWindow::on_actionTotalSpeed_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::TotalSpeed);
}

void MainWindow::on_actionDiveAngle_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::DiveAngle);
}

void MainWindow::on_actionCurvature_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::Curvature);
}

void MainWindow::on_actionGlideRatio_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::GlideRatio);
}

void MainWindow::on_actionHorizontalAccuracy_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::HorizontalAccuracy);
}

void MainWindow::on_actionVerticalAccuracy_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::VerticalAccuracy);
}

void MainWindow::on_actionSpeedAccuracy_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::SpeedAccuracy);
}

void MainWindow::on_actionNumberOfSatellites_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::NumberOfSatellites);
}

void MainWindow::on_actionAcceleration_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::Acceleration);
}

void MainWindow::on_actionTotalEnergy_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::TotalEnergy);
}

void MainWindow::on_actionEnergyRate_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::EnergyRate);
}

void MainWindow::on_actionLift_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::Lift);
}

void MainWindow::on_actionDrag_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::Drag);
}

void MainWindow::on_actionCourse_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::Course);
}

void MainWindow::on_actionCourseRate_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::CourseRate);
}

void MainWindow::on_actionCourseAccuracy_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::CourseAccuracy);
}

void MainWindow::on_actionPan_triggered()
{
    setTool(Pan);
}

void MainWindow::on_actionZoom_triggered()
{
    setTool(Zoom);
}

void MainWindow::on_actionMeasure_triggered()
{
    setTool(Measure);
}

void MainWindow::on_actionZero_triggered()
{
    setTool(Zero);
}

void MainWindow::on_actionGround_triggered()
{
    setTool(Ground);
}

void MainWindow::on_actionWind_triggered()
{
    mWindAdjustment = !mWindAdjustment;
    m_ui->actionWind->setChecked(mWindAdjustment);

    updateVelocity();

    emit dataChanged();
}

void MainWindow::on_actionImportGates_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Import Gates"), "", tr("CSV Files (*.csv)"));

    for (int i = 0; i < fileNames.size(); ++i)
    {
        QFile file(fileNames[i]);
        if (!file.open(QIODevice::ReadOnly))
        {
            // TODO: Error message
            continue;
        }

        QTextStream in(&file);

        // skip first 2 rows
        if (!in.atEnd()) in.readLine();
        if (!in.atEnd()) in.readLine();

        QVector< double > lat, lon, hMSL;

        while (!in.atEnd())
        {
            QString line = in.readLine();
            QStringList cols = line.split(",");

            lat.append(cols[1].toDouble());
            lon.append(cols[2].toDouble());
            hMSL.append(cols[3].toDouble());
        }

        if (lat.size() > 0)
        {
            qSort(lat.begin(), lat.end());
            qSort(lon.begin(), lon.end());
            qSort(hMSL.begin(), hMSL.end());

            int mid = lat.size() / 2;

            DataPoint dp ;

            dp.lat  = lat[mid];
            dp.lon  = lon[mid];
            dp.hMSL = hMSL[mid];

            m_waypoints.append(dp);
        }
    }

    emit dataChanged();
}

void MainWindow::on_actionPreferences_triggered()
{
    ConfigDialog dlg(this);

    dlg.setUnits(m_units);
    dlg.setMass(m_mass);
    dlg.setPlanformArea(m_planformArea);
    dlg.setMinDrag(m_minDrag);
    dlg.setMinLift(m_minLift);
    dlg.setMaxLift(m_maxLift);
    dlg.setMaxLD(m_maxLD);
    dlg.setSimulationTime(m_simulationTime);
    dlg.setLineThickness(mLineThickness);

    const double factor = (m_units == PlotValue::Metric) ? MPS_TO_KMH : MPS_TO_MPH;
    const QString unitText = (m_units == PlotValue::Metric) ? "km/h" : "mph";

    double windSpeed = sqrt(mWindE * mWindE + mWindN * mWindN) * factor;
    double windDirection = atan2(-mWindE, -mWindN) / M_PI * 180;
    if (windDirection < 0) windDirection += 360;

    dlg.setWindSpeed(windSpeed);
    dlg.setWindUnits(unitText);
    dlg.setWindDirection(windDirection);

    if (dlg.exec() == QDialog::Accepted)
    {
        if (m_units != dlg.units())
        {
            m_units = dlg.units();
            initRange();
        }

        if (m_mass != dlg.mass() ||
            m_planformArea != dlg.planformArea())
        {
            m_mass = dlg.mass();
            m_planformArea = dlg.planformArea();

            initAerodynamics();

            emit dataChanged();
        }

        if (m_minDrag != dlg.minDrag() ||
            m_minLift != dlg.minLift() ||
            m_maxLift != dlg.maxLift() ||
            m_maxLD != dlg.maxLD())
        {
            m_minDrag = dlg.minDrag();
            m_minLift = dlg.minLift();
            m_maxLift = dlg.maxLift();
            m_maxLD = dlg.maxLD();

            emit dataChanged();
        }

        m_simulationTime = dlg.simulationTime();

        bool plotChanged = false;
        for (int i = 0; i < plotArea()->yaLast; ++i)
        {
            PlotValue *yValue = plotArea()->yValue(i);

            if (yValue->color() != dlg.plotColor(i))
            {
                yValue->setColor(dlg.plotColor(i));
                plotChanged = true;
            }

            if (yValue->minimum() != dlg.plotMinimum(i))
            {
                yValue->setMinimum(dlg.plotMinimum(i) / yValue->factor(units()));
                plotChanged = true;
            }

            if (yValue->maximum() != dlg.plotMaximum(i))
            {
                yValue->setMaximum(dlg.plotMaximum(i) / yValue->factor(units()));
                plotChanged = true;
            }

            if (yValue->useMinimum() != dlg.plotUseMinimum(i))
            {
                yValue->setUseMinimum(dlg.plotUseMinimum(i));
                plotChanged = true;
            }

            if (yValue->useMaximum() != dlg.plotUseMaximum(i))
            {
                yValue->setUseMaximum(dlg.plotUseMaximum(i));
                plotChanged = true;
            }
        }

        if (mLineThickness != dlg.lineThickness())
        {
            mLineThickness = dlg.lineThickness();
            plotChanged = true;
        }

        if (plotChanged)
        {
            emit dataChanged();
        }

        if (mWindE != -dlg.windSpeed() * sin(dlg.windDirection() / 180 * PI) / factor ||
            mWindN != -dlg.windSpeed() * cos(dlg.windDirection() / 180 * PI) / factor)
        {
            mWindE = -dlg.windSpeed() * sin(dlg.windDirection() / 180 * PI) / factor;
            mWindN = -dlg.windSpeed() * cos(dlg.windDirection() / 180 * PI) / factor;

            emit dataChanged();
        }
    }
}

void MainWindow::on_actionImportVideo_triggered()
{
    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get last file read
    QString rootFolder = settings.value("videoFolder").toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Video"), rootFolder);

    if (!fileName.isEmpty())
    {
        // Remember last file read
        settings.setValue("videoFolder", QFileInfo(fileName).absoluteFilePath());

        // Create video view
        VideoView *videoView = new VideoView;
        QDockWidget *dockWidget = new QDockWidget(tr("Video View"));
        dockWidget->setWidget(videoView);
        dockWidget->setObjectName("videoView");
        addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

        // Default to floating and delete when closed
        dockWidget->setFloating(true);
        dockWidget->setAttribute(Qt::WA_DeleteOnClose);

        // Associate the view with the main window
        videoView->setMainWindow(this);

        // Set up notifications for video view
        connect(this, SIGNAL(dataChanged()),
                videoView, SLOT(updateView()));

        // Associate view with this file
        videoView->setMedia(fileName);
    }
}

void MainWindow::on_actionExportKML_triggered()
{
    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get last file read
    QString rootFolder = settings.value("kmlFolder").toString();

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export KML"),
                                                    rootFolder,
                                                    tr("KML Files (*.kml)"));

    if (!fileName.isEmpty())
    {
        // Remember last file read
        settings.setValue("kmlFolder", QFileInfo(fileName).absoluteFilePath());

        // Open output file
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            // TODO: Error message
            return;
        }

        QTextStream stream(&file);

        // Write headers
        stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
        stream << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << endl;
        stream << "  <Placemark>" << endl;
        stream << "    <name>" << QFileInfo(fileName).baseName() << "</name>" << endl;
        stream << "    <LineString>" << endl;
        stream << "      <altitudeMode>absolute</altitudeMode>" << endl;
        stream << "      <coordinates>" << endl;

        double lower = rangeLower();
        double upper = rangeUpper();

        bool first = true;
        for (int i = 0; i < dataSize(); ++i)
        {
            const DataPoint &dp = dataPoint(i);

            if (lower <= dp.t && dp.t <= upper)
            {
                if (first)
                {
                    stream << "        ";
                    first = false;
                }
                else
                {
                    stream << " ";
                }

                stream << QString("%1,%2,%3").arg(dp.lon, 0, 'f').arg(dp.lat, 0, 'f').arg(dp.hMSL, 0, 'f');
            }
        }

        if (!first)
        {
            stream << endl;
        }


        // Write footers
        stream << "      </coordinates>" << endl;
        stream << "    </LineString>" << endl;
        stream << "  </Placemark>" << endl;
        stream << "</kml>" << endl;
    }
}

void MainWindow::on_actionExportPlot_triggered()
{
    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get last file read
    QString rootFolder = settings.value("plotFolder").toString();

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export Plot"),
                                                    rootFolder,
                                                    tr("CSV Files (*.csv)"));

    if (!fileName.isEmpty())
    {
        // Remember last file read
        settings.setValue("plotFolder", QFileInfo(fileName).absoluteFilePath());

        // Open output file
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            // TODO: Error message
            return;
        }

        QTextStream stream(&file);

        // Write header
        stream << m_ui->plotArea->xValue()->title(m_units);
        for (int j = 0; j < DataPlot::yaLast; ++j)
        {
            if (!m_ui->plotArea->yValue(j)->visible()) continue;
            stream << "," << m_ui->plotArea->yValue(j)->title(m_units);
        }
        stream << endl;

        double lower = rangeLower();
        double upper = rangeUpper();

        for (int i = 0; i < dataSize(); ++i)
        {
            const DataPoint &dp = dataPoint(i);

            if (lower <= dp.t && dp.t <= upper)
            {
                stream << m_ui->plotArea->xValue()->value(dp, m_units);
                for (int j = 0; j < DataPlot::yaLast; ++j)
                {
                    if (!m_ui->plotArea->yValue(j)->visible()) continue;
                    stream << QString(",%1").arg(m_ui->plotArea->yValue(j)->value(dp, m_units), 0, 'f');
                }
                stream << endl;
            }
        }
    }
}

void MainWindow::on_actionExportTrack_triggered()
{
    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get last file read
    QString rootFolder = settings.value("trackFolder").toString();

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export Track"),
                                                    rootFolder,
                                                    tr("CSV Files (*.csv)"));

    if (!fileName.isEmpty())
    {
        // Remember last file read
        settings.setValue("trackFolder", QFileInfo(fileName).absoluteFilePath());

        // Open output file
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            // TODO: Error message
            return;
        }

        QTextStream stream(&file);

        // Write header
        stream << "time,lat,lon,hMSL,velN,velE,velD,hAcc,vAcc,sAcc,heading,cAcc,gpsFix,numSV" << endl;
        stream << ",(deg),(deg),(m),(m/s),(m/s),(m/s),(m),(m),(m/s),(deg),(deg),," << endl;

        double lower = rangeLower();
        double upper = rangeUpper();

        for (int i = 0; i < dataSize(); ++i)
        {
            const DataPoint &dp = dataPoint(i);

            if (lower <= dp.t && dp.t <= upper)
            {
                stream << dp.dateTime.date().toString(Qt::ISODate) << "T";
                stream << dp.dateTime.time().toString(Qt::ISODate) << ".";
                stream << QString("%1").arg(dp.dateTime.time().msec(), 3, 10, QChar('0')) << "Z,";

                stream << QString::number(dp.lat, 'f', 7) << ",";
                stream << QString::number(dp.lon, 'f', 7) << ",";
                stream << QString::number(dp.hMSL, 'f', 3) << ",";

                stream << QString::number(dp.velN, 'f', 2) << ",";
                stream << QString::number(dp.velE, 'f', 2) << ",";
                stream << QString::number(dp.velD, 'f', 2) << ",";

                stream << QString::number(dp.hAcc, 'f', 3) << ",";
                stream << QString::number(dp.vAcc, 'f', 3) << ",";
                stream << QString::number(dp.sAcc, 'f', 2) << ",";

                // Get adjusted heading
                double heading = dp.heading;
                while (heading <  0)   heading += 360;
                while (heading >= 360) heading -= 360;

                stream << QString::number(heading, 'f', 5) << ",";
                stream << QString::number(dp.cAcc, 'f', 5) << ",";

                stream << ",";  // gpsFix

                stream << QString::number(dp.numSV) << endl;
            }
        }
    }
}

void MainWindow::setRange(
        double lower,
        double upper)
{
    mRangeLower = qMin(lower, upper);
    mRangeUpper = qMax(lower, upper);

    emit dataChanged();
}

void MainWindow::setRotation(
        double rotation)
{
    m_viewDataRotation = rotation;
    emit rotationChanged(m_viewDataRotation);
}

void MainWindow::setZero(
        double t)
{
    if (m_data.isEmpty()) return;

    DataPoint dp0 = interpolateDataT(t);

    for (int i = 0; i < m_data.size(); ++i)
    {
        DataPoint &dp = m_data[i];

        dp.t -= dp0.t;
        dp.x -= dp0.x;
        dp.y -= dp0.y;
        dp.z -= dp0.z;

        dp.dist2D -= dp0.dist2D;
        dp.dist3D -= dp0.dist3D;
    }

    mMarkStart -= dp0.t;
    mMarkEnd -= dp0.t;

    setRange(mRangeLower - dp0.t, mRangeUpper - dp0.t);
    setTool(mPrevTool);
}

void MainWindow::setGround(
        double t)
{
    if (m_data.isEmpty()) return;

    DataPoint dp0 = interpolateDataT(t);

    for (int i = 0; i < m_data.size(); ++i)
    {
        DataPoint &dp = m_data[i];
        dp.alt -= dp0.alt;
    }

    setRange(mRangeLower, mRangeUpper);
    setTool(mPrevTool);
}

void MainWindow::setWindow(
        double windowBottom,
        double windowTop)
{
    mWindowBottom = windowBottom;
    mWindowTop = windowTop;

    emit dataChanged();
}

void MainWindow::setScoringVisible(
        bool visible)
{
    emit dataChanged();
}

void MainWindow::setMinDrag(
        double minDrag)
{
    m_minDrag = minDrag;

    emit dataChanged();
}

void MainWindow::setMaxLift(
        double maxLift)
{
    m_maxLift = maxLift;

    emit dataChanged();
}

void MainWindow::setMaxLD(
        double maxLD)
{
    m_maxLD = maxLD;

    emit dataChanged();
}

void MainWindow::updateWindow(void)
{
    switch (mWindowMode)
    {
    case Actual:
        mIsWindowValid = getWindowBounds(m_data, mWindowBottomDP, mWindowTopDP);
        break;
    case Optimal:
        mIsWindowValid = getWindowBounds(m_optimal, mWindowBottomDP, mWindowTopDP);
        break;
    }
}

bool MainWindow::isWindowValid() const
{
    return mIsWindowValid && mScoringView && m_ui->actionShowScoringView->isChecked();
}

void MainWindow::setWindowMode(
        WindowMode mode)
{
    mWindowMode = mode;

    emit dataChanged();
}

void MainWindow::setTool(
        Tool tool)
{
    if (tool != Ground && tool != Zero)
    {
        mPrevTool = tool;
    }
    mTool = tool;

    m_ui->actionPan->setChecked(tool == Pan);
    m_ui->actionZoom->setChecked(tool == Zoom);
    m_ui->actionMeasure->setChecked(tool == Measure);
    m_ui->actionZero->setChecked(tool == Zero);
    m_ui->actionGround->setChecked(tool == Ground);
}

void MainWindow::optimize(
        OptimizationMode mode)
{
    const int start = findIndexBelowT(0) + 1;

    // y = ax^2 + c
    const double m = 1 / m_maxLD;
    const double c = m_minDrag;
    const double a = m * m / (4 * c);

    const int workingSize    = 100;     // Working population
    const int keepSize       = 10;      // Number of elites to keep
    const int newSize        = 10;      // New genomes in first level
    const int numGenerations = 250;     // Generations per level of detail
    const int tournamentSize = 5;       // Number of individuals in a tournament
    const int mutationRate   = 100;     // Frequency of mutations
    const int truncationRate = 10;      // Frequency of truncations

    qsrand(QTime::currentTime().msec());

    const double dt = 0.25; // Time step (s)

    int kLim = 0;
    while (dt * (1 << kLim) < m_simulationTime)
    {
        ++kLim;
    }

    const int genomeSize = (1 << kLim) + 1;
    const int kMin = kLim - 4;
    const int kMax = kLim - 2;

    GenePool genePool;

    QProgressDialog progress("Initializing...",
                             "Abort",
                             0,
                             (kMax - kMin + 1) * numGenerations * workingSize + workingSize,
                             this);
    progress.setWindowModality(Qt::WindowModal);

    double maxScore = 0;
    bool abort = false;

    // Add new individuals
    for (int i = 0; i < workingSize; ++i)
    {
        progress.setValue(progress.value() + 1);
        if (progress.wasCanceled())
        {
            abort = true;
            break;
        }

        Genome g(genomeSize, kMin, m_minLift, m_maxLift);
        const QVector< DataPoint > result = g.simulate(dt, a, c, m_planformArea, m_mass, m_data[start], mWindowBottom);
        const double s = score(result, mode);
        genePool.append(Score(s, g));

        maxScore = qMax(maxScore, s);
    }

    // Increasing levels of detail
    for (int k = kMin; k <= kMax && !abort; ++k)
    {
        // Generations
        for (int j = 0; j < numGenerations && !abort; ++j)
        {
            progress.setValue(progress.value() + keepSize);
            if (progress.wasCanceled())
            {
                abort = true;
                break;
            }

            // Sort gene pool by score
            qSort(genePool);

            // Elitism
            GenePool newGenePool = genePool.mid(0, keepSize);

            // Initialize score
            maxScore = 0;
            for (int i = 0; i < keepSize; ++i)
            {
                maxScore = qMax(maxScore, newGenePool[i].first);
            }

            // Add new individuals in first level
            for (int i = 0; k == kMin && i < newSize; ++i)
            {
                progress.setValue(progress.value() + 1);
                if (progress.wasCanceled())
                {
                    abort = true;
                    break;
                }

                Genome g(genomeSize, kMin, m_minLift, m_maxLift);
                const QVector< DataPoint > result = g.simulate(dt, a, c, m_planformArea, m_mass, m_data[start], mWindowBottom);
                const double s = score(result, mode);
                newGenePool.append(Score(s, g));

                maxScore = qMax(maxScore, s);
            }

            // Tournament selection
            while (newGenePool.size() < workingSize)
            {
                progress.setValue(progress.value() + 1);
                if (progress.wasCanceled())
                {
                    abort = true;
                    break;
                }

                const Genome &p1 = selectGenome(genePool, tournamentSize);
                const Genome &p2 = selectGenome(genePool, tournamentSize);
                Genome g(p1, p2, k);

                if (qrand() % 100 < truncationRate)
                {
                    g.truncate(k);
                }
                if (qrand() % 100 < mutationRate)
                {
                    g.mutate(k, kMin, m_minLift, m_maxLift);
                }

                const QVector< DataPoint > result = g.simulate(dt, a, c, m_planformArea, m_mass, m_data[start], mWindowBottom);
                const double s = score(result, mode);
                newGenePool.append(Score(s, g));

                maxScore = qMax(maxScore, s);
            }

            genePool = newGenePool;

            // Show best score in progress dialog
            QString labelText;
            switch (mode)
            {
            case Time:
                labelText = QString::number(maxScore) + QString(" s");
                break;
            case Distance:
                labelText = QString::number(maxScore / 1000) + QString(" km");
                break;
            case HorizontalSpeed:
            case VerticalSpeed:
                labelText = QString::number(maxScore * MPS_TO_KMH) + QString(" km/h");
                break;
            }
            progress.setLabelText(QString("Optimizing (best score is ") +
                                  labelText +
                                  QString(")..."));
        }
    }

    progress.setValue((kMax - kMin + 1) * numGenerations * workingSize + workingSize);

    // Sort gene pool by score
    qSort(genePool);

    // Keep most fit individual
    m_optimal.clear();
    m_optimal = genePool[0].second.simulate(dt, a, c, m_planformArea, m_mass, m_data[start], mWindowBottom);

    emit dataChanged();
}

const Genome &MainWindow::selectGenome(
        const GenePool &genePool,
        const int tournamentSize)
{
    int jMax;
    double sMax;
    bool first = true;

    for (int i = 0; i < tournamentSize; ++i)
    {
        const int j = qrand() % genePool.size();
        if (first || genePool[j].first > sMax)
        {
            jMax = j;
            sMax = genePool[j].first;
            first = false;
        }
    }

    return genePool[jMax].second;
}

bool MainWindow::getWindowBounds(
        const QVector< DataPoint > result,
        DataPoint &dpBottom,
        DataPoint &dpTop)
{
    bool foundBottom = false;
    bool foundTop = false;
    int bottom, top;

    for (int i = result.size() - 1; i >= 0; --i)
    {
        const DataPoint &dp = result[i];

        if (dp.alt < mWindowBottom)
        {
            bottom = i;
            foundBottom = true;
        }

        if (dp.alt < mWindowTop)
        {
            top = i;
            foundTop = false;
        }

        if (dp.alt > mWindowTop)
        {
            foundTop = true;
        }

        if (dp.t < 0) break;
    }

    if (foundBottom && foundTop)
    {
        // Calculate bottom of window
        const DataPoint &dp1 = result[bottom - 1];
        const DataPoint &dp2 = result[bottom];
        dpBottom = DataPoint::interpolate(dp1, dp2, (mWindowBottom - dp1.alt) / (dp2.alt - dp1.alt));

        // Calculate top of window
        const DataPoint &dp3 = result[top - 1];
        const DataPoint &dp4 = result[top];
        dpTop = DataPoint::interpolate(dp3, dp4, (mWindowTop - dp3.alt) / (dp4.alt - dp3.alt));

        return true;
    }
    else
    {
        return false;
    }
}

double MainWindow::score(
        const QVector< DataPoint > &result,
        OptimizationMode mode)
{
    DataPoint dpBottom, dpTop;
    if (getWindowBounds(result, dpBottom, dpTop))
    {
        switch (mode)
        {
        case Time:
            return dpBottom.t - dpTop.t;
        case Distance:
            return dpBottom.x - dpTop.x;
        case HorizontalSpeed:
            return (dpBottom.x - dpTop.x) / (dpBottom.t - dpTop.t);
        case VerticalSpeed:
            return (mWindowTop - mWindowBottom) / (dpBottom.t - dpTop.t);
        }
    }
    else
    {
        return 0;
    }
}

DataPlot *MainWindow::plotArea() const
{
    return m_ui->plotArea;
}

void MainWindow::setLineThickness(
        double width)
{
    mLineThickness = width;
    emit dataChanged();
}

void MainWindow::setWind(
        double windE,
        double windN)
{
    mWindE = windE;
    mWindN = windN;

    updateVelocity();

    emit dataChanged();
}
