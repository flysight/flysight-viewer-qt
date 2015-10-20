#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
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

MainWindow::MainWindow(
        QWidget *parent):

    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    mMarkActive(false),
    m_viewDataRotation(0),
    m_units(PlotValue::Imperial),
    m_dtWind(30),
    m_temperature(288.15),
    m_mass(70),
    m_planformArea(2),
    m_wingSpan(1.4)
{
    m_ui->setupUi(this);

    // Ensure that closeEvent is called
    QObject::connect(m_ui->actionExit, SIGNAL(triggered()),
                     this, SLOT(close()));

    // Intitialize plot area
    initPlot();

    // Initialize 3D views
    initViews();

    // Initialize map view
    initMapView();

    // Initialize wind view
    initWindView();

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

        qint64 start = dp0.dateTime.toMSecsSinceEpoch();
        qint64 end = dp.dateTime.toMSecsSinceEpoch();

        dp.t = (double) (end - start) / 1000;
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

void MainWindow::on_actionOptimize()
{
    int start = findIndexBelowT(mRangeLower) + 1;
    int end   = findIndexAboveT(mRangeUpper);

    double s10 = 0, s01 = 0, s20 = 0, s11 = 0;
    double s21 = 0, s30 = 0, s40 = 0;

    for (int i = start; i < end; ++i)
    {
        const DataPoint &dp = m_data[i];

        s10 += dp.lift;
        s01 += dp.drag;
        s20 += dp.lift * dp.lift;
        s11 += dp.lift * dp.drag;
        s21 += dp.lift * dp.lift * dp.drag;
        s30 += dp.lift * dp.lift * dp.lift;
        s40 += dp.lift * dp.lift * dp.lift * dp.lift;
    }

    const double s00 = end - start + 1;
    const double det = s00 * s40 - s20 * s20;

    if (det == 0) return;

    // y = ax^2 + c
    const double a = (-s20 * s01 + s00 * s21) / det;
    const double c = ( s40 * s01 - s20 * s21) / det;

    initialize(start, end, s10, a * s10 * s10 + c);
    simulate(start, end);
}

void MainWindow::initialize(
        int start,
        int end,
        double cl,
        double cd)
{
    for (int i = start; i < end; ++i)
    {
        DataPoint &dp = m_data[i];
        dp.optimal_lift = cl;
        dp.optimal_drag = cd;
    }
}

void MainWindow::simulate(
        int start,
        int end)
{
    const double velH = sqrt(m_data[start].velE * m_data[start].velE + m_data[start].velN * m_data[start].velN);
    double theta = atan2(-m_data[start].velD, velH);
    double v = sqrt(m_data[start].velD * m_data[start].velD + velH * velH);
    double x = 0;
    double y = m_data[start].hMSL;

    const double y0 = m_data[start].hMSL;
    const double h0 = m_data[start].alt;

    for (int i = start; i < end - 1; ++i)
    {
        double t = m_data[i].t;
        const double h = m_data[i + 1].t - m_data[i].t;

        const double lift_prev = m_data[i].optimal_lift;
        const double drag_prev = m_data[i].optimal_drag;

        const double lift_next = m_data[i + 1].optimal_lift;
        const double drag_next = m_data[i + 1].optimal_drag;

        // Runge-Kutta integration
        // See https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods
        const double k0 = h * dtheta(t, theta, v, x, y, lift_prev);
        const double l0 = h *     dv(t, theta, v, x, y, drag_prev);
        const double m0 = h *     dx(t, theta, v, x, y);
        const double n0 = h *     dy(t, theta, v, x, y);

        const double k1 = h * dtheta(t + h/2, theta + k0/2, v + l0/2, x + m0/2, y + n0/2, (lift_prev + lift_next) / 2);
        const double l1 = h *     dv(t + h/2, theta + k0/2, v + l0/2, x + m0/2, y + n0/2, (drag_prev + drag_next) / 2);
        const double m1 = h *     dx(t + h/2, theta + k0/2, v + l0/2, x + m0/2, y + n0/2);
        const double n1 = h *     dy(t + h/2, theta + k0/2, v + l0/2, x + m0/2, y + n0/2);

        const double k2 = h * dtheta(t + h/2, theta + k1/2, v + l1/2, x + m1/2, y + n1/2, (lift_prev + lift_next) / 2);
        const double l2 = h *     dv(t + h/2, theta + k1/2, v + l1/2, x + m1/2, y + n1/2, (drag_prev + drag_next) / 2);
        const double m2 = h *     dx(t + h/2, theta + k1/2, v + l1/2, x + m1/2, y + n1/2);
        const double n2 = h *     dy(t + h/2, theta + k1/2, v + l1/2, x + m1/2, y + n1/2);

        const double k3 = h * dtheta(t + h, theta + k2, v + l2, x + m2, y + n2, lift_next);
        const double l3 = h *     dv(t + h, theta + k2, v + l2, x + m2, y + n2, drag_next);
        const double m3 = h *     dx(t + h, theta + k2, v + l2, x + m2, y + n2);
        const double n3 = h *     dy(t + h, theta + k2, v + l2, x + m2, y + n2);

        theta += (k0 + 2 * k1 + 2 * k2 + k3) / 6;
        v     += (l0 + 2 * l1 + 2 * l2 + l3) / 6;
        x     += (m0 + 2 * m1 + 2 * m2 + m3) / 6;
        y     += (n0 + 2 * n1 + 2 * n2 + n3) / 6;

        // Update data
        m_data[i + 1].optimal_velH =  v * cos(theta);
        m_data[i + 1].optimal_velD = -v * sin(theta);
        m_data[i + 1].optimal_hMSL = y;
        m_data[i + 1].optimal_alt  = y - y0 + h0;
    }
}

double MainWindow::dtheta(
        double t,
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

double MainWindow::dv(
        double t,
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

double MainWindow::dx(
        double t,
        double theta,
        double v,
        double x,
        double y)
{
    return v * cos(theta);
}

double MainWindow::dy(
        double t,
        double theta,
        double v,
        double x,
        double y)
{
    return v * sin(theta);
}
