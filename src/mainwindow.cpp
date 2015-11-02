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
#include "liftdragplot.h"
#include "mapview.h"
#include "videoview.h"
#include "windplot.h"
#include "wingsuitview.h"

MainWindow::MainWindow(
        QWidget *parent):

    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    mMarkActive(false),
    m_viewDataRotation(0),
    m_units(PlotValue::Imperial),
    m_dtWind(30),
    mWindowBottom(2000),
    mWindowTop(3000),
    mIsWindowValid(false),
    mWindowMode(Actual),
    mWingsuitView(0),
    m_temperature(288.15),
    m_mass(70),
    m_planformArea(2),
    m_wingSpan(1.4),
    m_minDrag(0.041),
    m_maxLift(0.52),
    m_efficiency(0.53)
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

    // Initialize map view
    initMapView();

    // Initialize wind view
    initWindView();

    // Initialize wingsuit view
    initWingsuitView();

    // Initialize lift/drag view
    initLiftDragView();

    // Restore window state
    readSettings();

    // Set default tool
    setTool(Pan);

    // Redraw plots
    emit dataChanged();

#ifdef Q_OS_MAC
    // Fix for single-key shortcuts on Mac
    // http://thebreakfastpost.com/2014/06/03/single-key-menu-shortcuts-with-qt5-on-osx/
    foreach (QAction *a, m_ui->menuPlots->actions())
    {
        QObject::connect(new QShortcut(a->shortcut(), a->parentWidget()),
                         SIGNAL(activated()), a, SLOT(trigger()));
    }
    foreach (QAction *a, m_ui->menu_Tools->actions())
    {
        QObject::connect(new QShortcut(a->shortcut(), a->parentWidget()),
                         SIGNAL(activated()), a, SLOT(trigger()));
    }
#endif
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
    settings.setValue("dtWind", m_dtWind);
    settings.setValue("temperature", m_temperature);
    settings.setValue("mass", m_mass);
    settings.setValue("planformArea", m_planformArea);
    settings.setValue("wingSpan", m_wingSpan);
    settings.setValue("minDrag", m_minDrag);
    settings.setValue("maxLift", m_maxLift);
    settings.setValue("efficiency", m_efficiency);
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings("FlySight", "Viewer");

    settings.beginGroup("mainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    m_units = (PlotValue::Units) settings.value("units", m_units).toInt();
    m_dtWind = settings.value("dtWind", m_dtWind).toDouble();
    m_temperature = settings.value("temperature", m_temperature).toDouble();
    m_mass = settings.value("mass", m_mass).toDouble();
    m_planformArea = settings.value("planformArea", m_planformArea).toDouble();
    m_wingSpan = settings.value("wingSpan", m_wingSpan).toDouble();
    m_minDrag = settings.value("minDrag", m_minDrag).toDouble();
    m_maxLift = settings.value("maxLift", m_maxLift).toDouble();
    m_efficiency = settings.value("efficiency", m_efficiency).toDouble();
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

void MainWindow::initWingsuitView()
{
    mWingsuitView = new WingsuitView;
    QDockWidget *dockWidget = new QDockWidget(tr("Wingsuit View"));
    dockWidget->setWidget(mWingsuitView);
    dockWidget->setObjectName("wingsuitView");
    dockWidget->setVisible(false);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    mWingsuitView->setMainWindow(this);

    connect(m_ui->actionShowWingsuitView, SIGNAL(toggled(bool)),
            dockWidget, SLOT(setVisible(bool)));
    connect(m_ui->actionShowWingsuitView, SIGNAL(toggled(bool)),
            this, SLOT(setWingsuitVisible(bool)));
    connect(dockWidget, SIGNAL(visibilityChanged(bool)),
            m_ui->actionShowWingsuitView, SLOT(setChecked(bool)));

    connect(this, SIGNAL(dataChanged()),
            mWingsuitView, SLOT(updateView()));
}

void MainWindow::initLiftDragView()
{
    LiftDragPlot *liftDragPlot = new LiftDragPlot;
    QDockWidget *dockWidget = new QDockWidget(tr("Lift/Drag View"));
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

            if (s == "time")  colMap[Time]  = i;
            if (s == "lat")   colMap[Lat]   = i;
            if (s == "lon")   colMap[Lon]   = i;
            if (s == "hMSL")  colMap[HMSL]  = i;
            if (s == "velN")  colMap[VelN]  = i;
            if (s == "velE")  colMap[VelE]  = i;
            if (s == "velD")  colMap[VelD]  = i;
            if (s == "hAcc")  colMap[HAcc]  = i;
            if (s == "vAcc")  colMap[VAcc]  = i;
            if (s == "sAcc")  colMap[SAcc]  = i;
            if (s == "numSV") colMap[NumSV] = i;
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

    for (int i = 0; i < m_data.size(); ++i)
    {
        DataPoint &dp = m_data[i];
        dp.curv = getSlope(i, DataPoint::diveAngle);
    }

    initWind();

    for (int i = 0; i < m_data.size(); ++i)
    {
        DataPoint &dp = m_data[i];
        dp.accel = getSlope(i, DataPoint::totalSpeed);
    }

    initAerodynamics();

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

void MainWindow::initWind()
{
    for (int i = 0; i < m_data.size(); ++i)
    {
        getWind(i);
    }
}

void MainWindow::getWind(
        const int center)
{
    // Weighted least-squares circle fit based on this:
    //   http://www.dtcenter.org/met/users/docs/write_ups/circle_fit.pdf

    DataPoint &dp0 = m_data[center];

    int start = findIndexBelowT(dp0.t - m_dtWind) + 1;
    int end   = findIndexAboveT(dp0.t + m_dtWind);

    double xbar = 0, ybar = 0, N = 0;
    for (int i = start; i < end; ++i)
    {
        DataPoint &dp = m_data[i];

        const double dt = dp.t - dp0.t;
        const double wi = 0.5 * (1 + cos(M_PI * dt / m_dtWind));

        const double xi = dp.velE;
        const double yi = dp.velN;

        xbar += wi * xi;
        ybar += wi * yi;

        N += wi;
    }

    if (N == 0)
    {
        dp0.windE = 0;
        dp0.windN = 0;
        dp0.velAircraft = 0;
        dp0.windErr = 0;
        return;
    }

    xbar /= N;
    ybar /= N;

    double suu = 0, suv = 0, svv = 0;
    double suuu = 0, suvv = 0, svuu = 0, svvv = 0;
    for (int i = start; i < end; ++i)
    {
        DataPoint &dp = m_data[i];

        const double dt = dp.t - dp0.t;
        const double wi = 0.5 * (1 + cos(M_PI * dt / m_dtWind));

        const double xi = dp.velE;
        const double yi = dp.velN;

        const double ui = xi - xbar;
        const double vi = yi - ybar;

        suu += wi * ui * ui;
        suv += wi * ui * vi;
        svv += wi * vi * vi;

        suuu += wi * ui * ui * ui;
        suvv += wi * ui * vi * vi;
        svuu += wi * vi * ui * ui;
        svvv += wi * vi * vi * vi;
    }

    const double det = suu * svv - suv * suv;

    if (det == 0)
    {
        dp0.windE = 0;
        dp0.windN = 0;
        dp0.velAircraft = 0;
        dp0.windErr = 0;
        return;
    }

    const double uc = 1 / det * (0.5 * svv * (suuu + suvv) - 0.5 * suv * (svvv + svuu));
    const double vc = 1 / det * (0.5 * suu * (svvv + svuu) - 0.5 * suv * (suuu + suvv));

    const double xc = uc + xbar;
    const double yc = vc + ybar;

    const double alpha = uc * uc + vc * vc + (suu + svv) / N;
    const double R = sqrt(alpha);

    dp0.windE = xc;
    dp0.windN = yc;
    dp0.velAircraft = R;

    double err = 0;
    for (int i = start; i < end; ++i)
    {
        DataPoint &dp = m_data[i];

        const double dt = dp.t - dp0.t;
        const double wi = 0.5 * (1 + cos(M_PI * dt / m_dtWind));

        const double xi = dp.velE;
        const double yi = dp.velN;

        const double dx = xi - xc;
        const double dy = yi - yc;

        const double term = sqrt(dx * dx + dy * dy) - R;
        err += wi * term * term;
    }

    dp0.windErr = sqrt(err / N);
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
        const double proj = (accelN * dp.velN + accelE * dp.velE + accelD * dp.velD) / vel;

        const double dragN = proj * dp.velN / vel;
        const double dragE = proj * dp.velE / vel;
        const double dragD = proj * dp.velD / vel;

        const double accelDrag = sqrt(dragN * dragN + dragE * dragE + dragD * dragD);

        // Calculate acceleration due to lift
        const double liftN = accelN - dragN;
        const double liftE = accelE - dragE;
        const double liftD = accelD - dragD;

        const double accelLift = sqrt(liftN * liftN + liftE * liftE + liftD * liftD);

        // From https://en.wikipedia.org/wiki/Atmospheric_pressure#Altitude_variation
        const double airPressure = SL_PRESSURE * pow(1 - LAPSE_RATE * dp.hMSL / SL_TEMP, A_GRAVITY * MM_AIR / GAS_CONST / LAPSE_RATE);

        // From https://en.wikipedia.org/wiki/Density_of_air
        const double airDensity = airPressure / (GAS_CONST / MM_AIR) / m_temperature;

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
    m_ui->actionWindSpeed->setChecked(m_ui->plotArea->plotVisible(DataPlot::WindSpeed));
    m_ui->actionWindDirection->setChecked(m_ui->plotArea->plotVisible(DataPlot::WindDirection));
    m_ui->actionAircraftSpeed->setChecked(m_ui->plotArea->plotVisible(DataPlot::AircraftSpeed));
    m_ui->actionAcceleration->setChecked(m_ui->plotArea->plotVisible(DataPlot::Acceleration));
    m_ui->actionTotalEnergy->setChecked(m_ui->plotArea->plotVisible(DataPlot::TotalEnergy));
    m_ui->actionEnergyRate->setChecked(m_ui->plotArea->plotVisible(DataPlot::EnergyRate));
    m_ui->actionLift->setChecked(m_ui->plotArea->plotVisible(DataPlot::Lift));
    m_ui->actionDrag->setChecked(m_ui->plotArea->plotVisible(DataPlot::Drag));
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

void MainWindow::on_actionWindSpeed_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::WindSpeed);
}

void MainWindow::on_actionWindDirection_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::WindDirection);
}

void MainWindow::on_actionAircraftSpeed_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::AircraftSpeed);
}

void MainWindow::on_actionWindError_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::WindError);
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
    ConfigDialog dlg;

    dlg.setUnits(m_units);

    dlg.setDtWind(m_dtWind);

    dlg.setTemperature(m_temperature);
    dlg.setMass(m_mass);
    dlg.setPlanformArea(m_planformArea);
    dlg.setWingSpan(m_wingSpan);
    dlg.setMinDrag(m_minDrag);
    dlg.setMaxLift(m_maxLift);
    dlg.setEfficiency(m_efficiency);

    dlg.exec();

    if (m_units != dlg.units())
    {
        m_units = dlg.units();
        initRange();
    }

    if (m_dtWind != dlg.dtWind())
    {
        m_dtWind = dlg.dtWind();
        initWind();

        emit dataChanged();
    }

    if (m_temperature != dlg.temperature() ||
        m_mass != dlg.mass() ||
        m_planformArea != dlg.planformArea() ||
        m_wingSpan != dlg.wingSpan())
    {
        m_temperature = dlg.temperature();
        m_mass = dlg.mass();
        m_planformArea = dlg.planformArea();
        m_wingSpan = dlg.wingSpan();

        initAerodynamics();

        emit dataChanged();
    }

    if (m_minDrag != dlg.minDrag() ||
        m_maxLift != dlg.maxLift() ||
        m_efficiency != dlg.efficiency())
    {
        m_minDrag = dlg.minDrag();
        m_maxLift = dlg.maxLift();
        m_efficiency = dlg.efficiency();

        emit dataChanged();
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

void MainWindow::setWingsuitVisible(
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

void MainWindow::setEfficiency(
        double efficiency)
{
    m_efficiency = efficiency;

    emit dataChanged();
}

void MainWindow::updateWindow(void)
{
    const QVector< DataPoint > &data =
            (mWindowMode == Actual) ? m_data : m_optimal;

    mIsWindowValid = false;

    int iBottom = data.size() - 1, iTop = data.size() - 1;

    // Find end of window
    for (int i = data.size() - 1; i >= 0; --i)
    {
        const DataPoint &dp = data[i];

        if (dp.alt < mWindowBottom)
        {
            iBottom = i;
        }

        if (dp.alt < mWindowTop)
        {
            iTop = i;
        }
        else
        {
            mIsWindowValid = true;
        }

        if (mIsWindowValid && dp.t < 0) break;
    }

    if (mIsWindowValid)
    {
        // Calculate bottom of window
        const DataPoint &dp1 = data[iBottom - 1];
        const DataPoint &dp2 = data[iBottom];
        mWindowBottomDP = DataPoint::interpolate(dp1, dp2, (mWindowBottom - dp1.alt) / (dp2.alt - dp1.alt));

        // Calculate top of window
        const DataPoint &dp3 = data[iTop - 1];
        const DataPoint &dp4 = data[iTop];
        mWindowTopDP = DataPoint::interpolate(dp3, dp4, (mWindowTop - dp3.alt) / (dp4.alt - dp3.alt));
    }
}

bool MainWindow::isWindowValid() const
{
    return mIsWindowValid && mWingsuitView && m_ui->actionShowWingsuitView->isChecked();
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

void MainWindow::on_actionOptimize_triggered()
{
    int start = findIndexBelowT(mRangeLower) + 1;
    int end   = findIndexAboveT(mRangeUpper);

    const double bb = m_wingSpan;
    const double ss = m_planformArea;
    const double ar = bb * bb / ss;

    // y = ax^2 + c
    const double a = 1 / (M_PI * m_efficiency * ar);
    const double c = m_minDrag;

    const double t0 = m_data[start].t;
    const double velH = sqrt(m_data[start].velE * m_data[start].velE + m_data[start].velN * m_data[start].velN);
    const double theta0 = atan2(-m_data[start].velD, velH);
    const double v0 = sqrt(m_data[start].velD * m_data[start].velD + velH * velH);
    const double x0 = 0;
    const double y0 = m_data[start].hMSL;

    qsrand(QTime::currentTime().msec());

    int aoaSize = 1, kMax = 0;
    while (aoaSize < end - start)
    {
        aoaSize *= 2;
        ++kMax;
    }

    GenePool genePool;

    double smoothedChange = 1;
    double maxScore = 0;

    const double max_aoa = m_maxLift / (2 * M_PI);

    // Initialize the gene pool
    for (int i = 0; i < 100; ++i)
    {
        double aoa = (double) qrand() / RAND_MAX * max_aoa;
        Genome g(aoaSize, aoa);

        const double s = simulate(g, m_timeStep, a, c, t0, theta0, v0, x0, y0, -1);
        genePool.append(Score(s, g));
        maxScore = qMax(maxScore, s);
    }

    QProgressDialog progress("Optimizing...", "Abort", 0, kMax * 100, this);
    progress.setWindowModality(Qt::WindowModal);

    // Increasing levels of detail
    for (int k = 0; k < kMax; ++k)
    {
        const int parts = (1 << k);

        // Generations
        for (int j = 0; j < 100; ++j)
        {
            progress.setValue(k * 100 + j);
            if (progress.wasCanceled())
            {
                k = kMax;
                break;
            }

            // Sort gene pool by score
            qSort(genePool);

            const double prevScore = maxScore;

            // Replace unfit individuals
            for (int i = 10; i < 100; ++i)
            {
                const int iParent = i / 10 - 1;
                Genome g = genePool[iParent].second;
                iterate(g, parts);

                const double s = simulate(g, m_timeStep, a, c, t0, theta0, v0, x0, y0, -1);
                genePool[i] = Score(s, g);
                maxScore = qMax(maxScore, s);
            }

            // Stop if we aren't seeing enough change
            const double change = (maxScore - prevScore) / maxScore;
            smoothedChange = (9 * smoothedChange + change) / 10;
            if (smoothedChange < 1e-6) break;
        }
    }

    progress.setValue(kMax * 100);

    // Keep most fit individual
    m_optimal.clear();
    simulate(genePool[0].second, m_timeStep, a, c, t0, theta0, v0, x0, y0, start);

    emit dataChanged();

    // Next: - Start simulation at exit and proceed to bottom of competition window.
}

void MainWindow::iterate(
        QVector< double > &aoa,
        int parts)
{
    const int i = qrand() % (parts + 1);
    const double max_aoa = m_maxLift / (2 * M_PI);
    const double minr = -max_aoa / parts, maxr = max_aoa / parts;
    const double r = minr + (double) qrand() / RAND_MAX * (maxr - minr);

    if (i > 0)
    {
        const int jPrev = aoa.size() * (i - 1) / parts;
        const int jNext = aoa.size() * i / parts;

        const double rPrev = 0.0;
        const double rNext = r;

        for (int j = jPrev; j < jNext; ++j)
        {
            const double r = rPrev + (rNext - rPrev) * (j - jPrev) / (jNext - jPrev);
            aoa[j] += r;
        }
    }

    if (i < parts)
    {
        const int jPrev = aoa.size() * i / parts;
        const int jNext = aoa.size() * (i + 1) / parts;

        const double rPrev = r;
        const double rNext = 0.0;

        for (int j = jPrev; j < jNext; ++j)
        {
            const double r = rPrev + (rNext - rPrev) * (j - jPrev) / (jNext - jPrev);
            aoa[j] += r;
        }
    }
}

double MainWindow::simulate(
        const QVector< double > &aoa,
        double h,
        double a,
        double c,
        double t0,
        double theta0,
        double v0,
        double x0,
        double y0,
        int start)
{
    double t = t0;
    double theta = theta0;
    double v = v0;
    double x = x0;
    double y = y0;

    double tStart, tEnd;
    double xStart, xEnd;
    int armed = 0;

    double dist2D, dist3D;

    if (start >= 0)
    {
        dist2D = m_data[start].dist2D;
        dist3D = m_data[start].dist3D;
    }

    QVector< double >::ConstIterator i;
    for (i = aoa.constBegin();
         i != aoa.constEnd() && i + 1 != aoa.constEnd();
         ++i)
    {
        const double lift_prev = lift(*i);
        const double drag_prev = drag(*i, a, c);

        const double lift_next = lift(*(i + 1));
        const double drag_next = drag(*(i + 1), a, c);

        // Runge-Kutta integration
        // See https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods
        const double k0 = h * dtheta_dt(theta, v, x, y, lift_prev);
        const double l0 = h *     dv_dt(theta, v, x, y, drag_prev);
        const double m0 = h *     dx_dt(theta, v, x, y);
        const double n0 = h *     dy_dt(theta, v, x, y);

        const double k1 = h * dtheta_dt(theta + k0/2, v + l0/2, x + m0/2, y + n0/2, (lift_prev + lift_next) / 2);
        const double l1 = h *     dv_dt(theta + k0/2, v + l0/2, x + m0/2, y + n0/2, (drag_prev + drag_next) / 2);
        const double m1 = h *     dx_dt(theta + k0/2, v + l0/2, x + m0/2, y + n0/2);
        const double n1 = h *     dy_dt(theta + k0/2, v + l0/2, x + m0/2, y + n0/2);

        const double k2 = h * dtheta_dt(theta + k1/2, v + l1/2, x + m1/2, y + n1/2, (lift_prev + lift_next) / 2);
        const double l2 = h *     dv_dt(theta + k1/2, v + l1/2, x + m1/2, y + n1/2, (drag_prev + drag_next) / 2);
        const double m2 = h *     dx_dt(theta + k1/2, v + l1/2, x + m1/2, y + n1/2);
        const double n2 = h *     dy_dt(theta + k1/2, v + l1/2, x + m1/2, y + n1/2);

        const double k3 = h * dtheta_dt(theta + k2, v + l2, x + m2, y + n2, lift_next);
        const double l3 = h *     dv_dt(theta + k2, v + l2, x + m2, y + n2, drag_next);
        const double m3 = h *     dx_dt(theta + k2, v + l2, x + m2, y + n2);
        const double n3 = h *     dy_dt(theta + k2, v + l2, x + m2, y + n2);

        const double dtheta = (k0 + 2 * k1 + 2 * k2 + k3) / 6;
        const double dv     = (l0 + 2 * l1 + 2 * l2 + l3) / 6;
        const double dx     = (m0 + 2 * m1 + 2 * m2 + m3) / 6;
        const double dy     = (n0 + 2 * n1 + 2 * n2 + n3) / 6;

        const double tNext     = t + h;
        const double thetaNext = theta + dtheta;
        const double vNext     = v + dv;
        const double xNext     = x + dx;
        const double yNext     = y + dy;

        const double alt = y + m_data[0].alt - m_data[0].hMSL;
        const double altNext = yNext + m_data[0].alt - m_data[0].hMSL;

        if (armed == 0 && alt >= 3000 && altNext < 3000)
        {
            tStart = t + (3000 - alt) / (altNext - alt) * (tNext - t);
            xStart = x + (3000 - alt) / (altNext - alt) * (xNext - x);
            ++armed;
        }
        if (armed == 1 && alt >= 2000 && altNext < 2000)
        {
            tEnd = t + (2000 - alt) / (altNext - alt) * (tNext - t);
            xEnd = x + (2000 - alt) / (altNext - alt) * (xNext - x);
            ++armed;
            break;
        }

        t      = tNext;
        theta  = thetaNext;
        v      = vNext;
        x      = xNext;
        y      = yNext;

        // Update data
        if (start >= 0)
        {
            DataPoint pt;

            pt.hMSL  = yNext;

            pt.velN  = 0;
            pt.velE  = v * cos(theta);
            pt.velD  = -v * sin(theta);

            pt.t = tNext;
            pt.x = xNext;
            pt.y = 0;
            pt.z = pt.alt = altNext;

            dist2D += dx;
            dist3D += sqrt(dx * dx + dy * dy);

            pt.dist2D = dist2D;
            pt.dist3D = dist3D;

            // curv
            // accel

            pt.lift = lift_next;
            pt.drag = drag_next;

            m_optimal.append(pt);
        }
    }

    if (armed == 2)
    {
//        return (xEnd - xStart) / (tEnd - tStart);
        return xEnd - xStart;
//        return tEnd - tStart;
    }
    else
    {
        return 0;
    }
}

double MainWindow::dtheta_dt(
        double theta,
        double v,
        double x,
        double y,
        double lift)
{
    // From https://en.wikipedia.org/wiki/Atmospheric_pressure#Altitude_variation
    const double airPressure = SL_PRESSURE * pow(1 - LAPSE_RATE * y / SL_TEMP, A_GRAVITY * MM_AIR / GAS_CONST / LAPSE_RATE);

    // From https://en.wikipedia.org/wiki/Density_of_air
    const double airDensity = airPressure / (GAS_CONST / MM_AIR) / m_temperature;

    // From https://en.wikipedia.org/wiki/Dynamic_pressure
    const double dynamicPressure = airDensity * v * v / 2;

    // Calculate acceleration due to drag and lift
    const double accelLift = dynamicPressure * m_planformArea * lift / m_mass;

    return (accelLift - A_GRAVITY * cos(theta)) / v;
}

double MainWindow::dv_dt(
        double theta,
        double v,
        double x,
        double y,
        double drag)
{
    // From https://en.wikipedia.org/wiki/Atmospheric_pressure#Altitude_variation
    const double airPressure = SL_PRESSURE * pow(1 - LAPSE_RATE * y / SL_TEMP, A_GRAVITY * MM_AIR / GAS_CONST / LAPSE_RATE);

    // From https://en.wikipedia.org/wiki/Density_of_air
    const double airDensity = airPressure / (GAS_CONST / MM_AIR) / m_temperature;

    // From https://en.wikipedia.org/wiki/Dynamic_pressure
    const double dynamicPressure = airDensity * v * v / 2;

    // Calculate acceleration due to drag and lift
    const double accelDrag = dynamicPressure * m_planformArea * drag / m_mass;

    return -accelDrag - A_GRAVITY * sin(theta);
}

double MainWindow::dx_dt(
        double theta,
        double v,
        double x,
        double y)
{
    return v * cos(theta);
}

double MainWindow::dy_dt(
        double theta,
        double v,
        double x,
        double y)
{
    return v * sin(theta);
}

double MainWindow::lift(
        double aoa)
{
    const double max_aoa = m_maxLift / (2 * M_PI);
    const double width = 0.001;
    const double w = 1 / (1 + exp(-(aoa - max_aoa) / width));
    return w * (2 * sin(aoa) * sin(2 * aoa)) + (1 - w) * (2 * M_PI * aoa);
}

double MainWindow::drag(
        double aoa,
        double a,
        double c)
{
    const double cl = 2 * M_PI * aoa;
    return a * cl * cl + c;
}
