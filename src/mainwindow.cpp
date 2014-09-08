#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QShortcut>
#include <QToolTip>

#include "configdialog.h"
#include "dataview.h"

MainWindow::MainWindow(
        QWidget *parent):

    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_xAxis(Time),
    m_markActive(false),
    m_topViewPan(false),
    m_viewDataRotation(0),
    m_units(PlotValue::Imperial)
{
    m_ui->setupUi(this);

#ifdef Q_OS_MAC
    // Ensure that closeEvent is called
    QObject::connect(m_ui->actionExit, SIGNAL(triggered()),
                     this, SLOT(close()));
#endif

    // Intitialize plot area
    initPlot();

    // Initialize 3D views
    initViews();

    // Restore window state
    readSettings();

    // Set default tool
    setTool(Pan);

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
    while (!m_xValues.isEmpty())
    {
        delete m_xValues.takeLast();
    }

    while (!m_yValues.isEmpty())
    {
        delete m_yValues.takeLast();
    }

    delete m_ui;
}

void MainWindow::writeSettings()
{
    QSettings settings("FlySight", "Viewer");

    settings.beginGroup("mainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("units", m_units);
    settings.setValue("xAxis", m_xAxis);
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings("FlySight", "Viewer");

    settings.beginGroup("mainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    m_units = (PlotValue::Units) settings.value("units", m_units).toInt();
    m_xAxis = (XAxisType) settings.value("xAxis", m_xAxis).toInt();
    settings.endGroup();
}

void MainWindow::initPlot()
{
    m_xValues.append(new PlotTime);
    m_xValues.append(new PlotDistance2D);
    m_xValues.append(new PlotDistance3D);

    foreach (PlotValue *v, m_xValues)
    {
        v->readSettings();
    }

    updateBottomActions();

    m_yValues.append(new PlotElevation);
    m_yValues.append(new PlotVerticalSpeed);
    m_yValues.append(new PlotHorizontalSpeed);
    m_yValues.append(new PlotTotalSpeed);
    m_yValues.append(new PlotDiveAngle);
    m_yValues.append(new PlotCurvature);
    m_yValues.append(new PlotGlideRatio);
    m_yValues.append(new PlotHorizontalAccuracy);
    m_yValues.append(new PlotVerticalAccuracy);
    m_yValues.append(new PlotSpeedAccuracy);
    m_yValues.append(new PlotNumberOfSatellites);

    foreach (PlotValue *v, m_yValues)
    {
        v->readSettings();
    }

    m_ui->actionElevation->setChecked(m_yValues[Elevation]->visible());
    m_ui->actionVerticalSpeed->setChecked(m_yValues[VerticalSpeed]->visible());
    m_ui->actionHorizontalSpeed->setChecked(m_yValues[HorizontalSpeed]->visible());
    m_ui->actionTotalSpeed->setChecked(m_yValues[TotalSpeed]->visible());
    m_ui->actionDiveAngle->setChecked(m_yValues[DiveAngle]->visible());
    m_ui->actionCurvature->setChecked(m_yValues[Curvature]->visible());
    m_ui->actionGlideRatio->setChecked(m_yValues[GlideRatio]->visible());
    m_ui->actionHorizontalAccuracy->setChecked(m_yValues[HorizontalAccuracy]->visible());
    m_ui->actionVerticalAccuracy->setChecked(m_yValues[VerticalAccuracy]->visible());
    m_ui->actionSpeedAccuracy->setChecked(m_yValues[SpeedAccuracy]->visible());
    m_ui->actionNumberOfSatellites->setChecked(m_yValues[NumberOfSatellites]->visible());

    connect(m_ui->plotArea, SIGNAL(zoom(const QCPRange &)),
            this, SLOT(onDataPlot_zoom(const QCPRange &)));
    connect(m_ui->plotArea, SIGNAL(pan(double, double)),
            this, SLOT(onDataPlot_pan(double, double)));
    connect(m_ui->plotArea, SIGNAL(measure(double, double)),
            this, SLOT(onDataPlot_measure(double, double)));
    connect(m_ui->plotArea, SIGNAL(zero(double)),
            this, SLOT(onDataPlot_zero(double)));
    connect(m_ui->plotArea, SIGNAL(ground(double)),
            this, SLOT(onDataPlot_ground(double)));

    connect(m_ui->plotArea, SIGNAL(mark(double)),
            this, SLOT(onDataPlot_mark(double)));

    connect(m_ui->plotArea, SIGNAL(clear()),
            this, SLOT(onDataPlot_clear()));

    m_ui->plotArea->setMainWindow(this);

    connect(this, SIGNAL(rangeChanged(const QCPRange &)),
            m_ui->plotArea, SLOT(setRange(const QCPRange &)));
    connect(this, SIGNAL(dataChanged()),
            m_ui->plotArea, SLOT(updateData()));
}

void MainWindow::initViews()
{
    // Add side view
    mLeftView = new DataView;
    QDockWidget *leftDock = new QDockWidget(tr("Side View"));
    leftDock->setWidget(mLeftView);
    leftDock->setObjectName("leftView");
    addDockWidget(Qt::BottomDockWidgetArea, leftDock);
    connect(m_ui->actionShowLeftView, SIGNAL(toggled(bool)),
            leftDock, SLOT(setVisible(bool)));
    connect(leftDock, SIGNAL(visibilityChanged(bool)),
            m_ui->actionShowLeftView, SLOT(setChecked(bool)));

    // Add top view
    mTopView = new DataView;
    QDockWidget *topDock = new QDockWidget(tr("Top View"));
    topDock->setWidget(mTopView);
    topDock->setObjectName("topView");
    addDockWidget(Qt::BottomDockWidgetArea, topDock);
    connect(m_ui->actionShowTopView, SIGNAL(toggled(bool)),
            topDock, SLOT(setVisible(bool)));
    connect(topDock, SIGNAL(visibilityChanged(bool)),
            m_ui->actionShowTopView, SLOT(setChecked(bool)));

    // Add front view
    mFrontView = new DataView;
    QDockWidget *frontDock = new QDockWidget(tr("Front View"));
    frontDock->setWidget(mFrontView);
    frontDock->setObjectName("frontView");
    addDockWidget(Qt::BottomDockWidgetArea, frontDock);
    connect(m_ui->actionShowFrontView, SIGNAL(toggled(bool)),
            frontDock, SLOT(setVisible(bool)));
    connect(frontDock, SIGNAL(visibilityChanged(bool)),
            m_ui->actionShowFrontView, SLOT(setChecked(bool)));

    mTopView->setMouseTracking(true);
    mLeftView->setMouseTracking(true);
    mFrontView->setMouseTracking(true);

    connect(mTopView, SIGNAL(mousePress(QMouseEvent *)),
            this, SLOT(onTopView_mousePress(QMouseEvent *)));
    connect(mTopView, SIGNAL(mouseRelease(QMouseEvent *)),
            this, SLOT(onTopView_mouseRelease(QMouseEvent *)));
    connect(mTopView, SIGNAL(mouseMove(QMouseEvent *)),
            this, SLOT(onTopView_mouseMove(QMouseEvent *)));

    connect(mLeftView, SIGNAL(mouseMove(QMouseEvent *)),
            this, SLOT(onLeftView_mouseMove(QMouseEvent *)));
    connect(mFrontView, SIGNAL(mouseMove(QMouseEvent *)),
            this, SLOT(onFrontView_mouseMove(QMouseEvent *)));
}

void MainWindow::closeEvent(
        QCloseEvent *event)
{
    // Save window state
    writeSettings();

    // Save plot state
    foreach (PlotValue *v, m_yValues)
    {
        v->writeSettings();
    }

    event->accept();
}

void MainWindow::onDataPlot_measure(
        double xStart,
        double xEnd)
{
    if (m_data.isEmpty()) return;

    DataPoint dpStart = interpolateData(xStart);
    DataPoint dpEnd = interpolateData(xEnd);

    QString status;

    status = QString("<table width='300'>");

    const double val = getXValue(dpEnd, m_xAxis)
            - getXValue(dpStart, m_xAxis);
    status += QString("<tr style='color:black;'><td>%1</td><td>%2</td><td>(%3%4)</td></tr>")
            .arg(m_xValues[m_xAxis]->title(m_units))
            .arg(m_xValues[m_xAxis]->value(dpEnd, m_units))
            .arg(val < 0 ? "" : "+")
            .arg(val);

    for (int i = 0; i < yaLast; ++i)
    {
        if (m_yValues[i]->visible())
        {
            const double val = m_yValues[i]->value(dpEnd, m_units)
                    - m_yValues[i]->value(dpStart, m_units);
            status += QString("<tr style='color:%5;'><td>%1</td><td>%2</td><td>(%3%4)</td></tr>")
                    .arg(m_yValues[(YAxisType) i]->title(m_units))
                    .arg(m_yValues[i]->value(dpEnd, m_units))
                    .arg(val < 0 ? "" : "+")
                    .arg(val)
                    .arg(m_yValues[i]->color().name());
        }
    }

    status += QString("</table>");

    QToolTip::showText(QCursor::pos(), status);

    mark(dpEnd);
}

DataPoint MainWindow::interpolateData(
        double x)
{
    const int i1 = findIndexBelowX(x);
    const int i2 = findIndexAboveX(x);

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
        const double x1 = getXValue(dp1, m_xAxis);
        const double x2 = getXValue(dp2, m_xAxis);
        return interpolate(dp1, dp2, (x - x1) / (x2 - x1));
    }
}


void MainWindow::onDataPlot_mark(
        double xMark)
{
    if (m_data.isEmpty()) return;

    const DataPoint dp = interpolateData(xMark);
    mark(dp);

    QString status;

    status = QString("<table width='200'>");

    status += QString("<tr style='color:black;'><td>%1</td><td>%2</td></tr>")
            .arg(m_xValues[m_xAxis]->title(m_units))
            .arg(m_xValues[m_xAxis]->value(dp, m_units));

    for (int i = 0; i < yaLast; ++i)
    {
        if (m_yValues[i]->visible())
        {
            status += QString("<tr style='color:%3;'><td>%1</td><td>%2</td></tr>")
                    .arg(m_yValues[(YAxisType) i]->title(m_units))
                    .arg(m_yPlot[i])
                    .arg(m_yValues[i]->color().name());
        }
    }

    status += QString("</table>");

    QToolTip::showText(QCursor::pos(), status);
}

void MainWindow::mark(
        const DataPoint &dp)
{
    m_xPlot = getXValue(dp, m_xAxis);
    for (int i = 0; i < yaLast; ++i)
    {
        if (m_yValues[i]->visible())
        {
            m_yPlot[i] = m_yValues[i]->value(dp, m_units);
        }
    }

    m_xView = dp.x;
    m_yView = dp.y;
    m_zView = dp.z;

    m_markActive = true;

    emit dataChanged();
    updateViewData();
}

void MainWindow::onDataPlot_clear()
{
    m_markActive = false;

    QToolTip::hideText();

    emit dataChanged();
    updateViewData();
}

int MainWindow::findIndexBelowX(
        double x)
{
    for (int i = 0; i < m_data.size(); ++i)
    {
        DataPoint &dp = m_data[i];

        if (getXValue(dp, m_xAxis) > x)
            return i - 1;
    }

    return m_data.size() - 1;
}

int MainWindow::findIndexAboveX(
        double x)
{
    for (int i = m_data.size() - 1; i >= 0; --i)
    {
        DataPoint &dp = m_data[i];

        if (getXValue(dp, m_xAxis) <= x)
            return i + 1;
    }

    return 0;
}

int MainWindow::findIndexBelowT(
        double t)
{
    for (int i = 0; i < m_data.size(); ++i)
    {
        DataPoint &dp = m_data[i];

        if (dp.t > t)
            return i - 1;
    }

    return m_data.size() - 1;
}

int MainWindow::findIndexAboveT(
        double t)
{
    for (int i = m_data.size() - 1; i >= 0; --i)
    {
        DataPoint &dp = m_data[i];

        if (dp.t <= t)
            return i + 1;
    }

    return 0;
}

void MainWindow::on_actionImport_triggered()
{
    const double pi = 3.14159265359;

    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get last file read
    QString rootFolder = settings.value("folder").toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import"), rootFolder, tr("CSV Files (*.csv)"));

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        // TODO: Error message
        return;
    }

    // Remember last file read
    settings.setValue("folder", QFileInfo(fileName).absoluteFilePath());

    QTextStream in(&file);

    // skip first 2 rows
    if (!in.atEnd()) in.readLine();
    if (!in.atEnd()) in.readLine();

    m_data.clear();

    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList cols = line.split(",");

        DataPoint pt;

        pt.dateTime = QDateTime::fromString(cols[0], Qt::ISODate);

        pt.lat   = cols[1].toDouble();
        pt.lon   = cols[2].toDouble();
        pt.hMSL  = cols[3].toDouble();

        pt.velN  = cols[4].toDouble();
        pt.velE  = cols[5].toDouble();
        pt.velD  = cols[6].toDouble();

        pt.hAcc  = cols[7].toDouble();
        pt.vAcc  = cols[8].toDouble();
        pt.sAcc  = cols[9].toDouble();

        pt.numSV = cols[11].toDouble();

        //
        // TODO: Handle newer CSV version
        //

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
        dp.curv = getSlope(i, DiveAngle);
    }

    if (dt.size() > 0)
    {
        qSort(dt.begin(), dt.end());
        m_timeStep = dt.at(dt.size() / 2);
    }
    else
    {
        m_timeStep = 1.0;
    }

    initPlotData();
    updateViewData();
}

double MainWindow::getSlope(
        const int center,
        YAxisType yAxis) const
{
    int iMin = qMax (0, center - 2);
    int iMax = qMin (m_data.size () - 1, center + 2);

    double sumx = 0, sumy = 0, sumxx = 0, sumxy = 0;

    for (int i = iMin; i <= iMax; ++i)
    {
        const DataPoint &dp = m_data[i];
        double yValue = m_yValues[yAxis]->value(dp, m_units);

        sumx += dp.t;
        sumy += yValue;
        sumxx += dp.t * dp.t;
        sumxy += dp.t * yValue;
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

void MainWindow::on_actionExit_triggered()
{
    qApp->quit();
}

void MainWindow::initPlotData()
{
    double xMin, xMax;

    for (int i = 0; i < m_data.size(); ++i)
    {
        DataPoint &dp = m_data[i];

        double x = getXValue(dp, m_xAxis);

        if (i == 0)
        {
            xMin = xMax = x;
        }
        else
        {
            if (x < xMin) xMin = x;
            if (x > xMax) xMax = x;
        }
    }

    m_ui->plotArea->xAxis->setRange(xMin, xMax);
    m_ui->plotArea->xAxis->setLabel(m_xValues[m_xAxis]->title(m_units));

    emit dataChanged();
}

void MainWindow::updateViewData()
{
    const QCPRange &range = m_ui->plotArea->xAxis->range();

    QVector< double > t, x, y, z;

    double xMin, xMax;
    double yMin, yMax;
    double zMin, zMax;

    bool first = true;

    for (int i = 0; i < m_data.size(); ++i)
    {
        DataPoint &dp = m_data[i];

        if (range.contains(getXValue(dp, m_xAxis)))
        {
            t.append(dp.t);

            if (m_units == PlotValue::Metric)
            {
                x.append(dp.x *  cos(m_viewDataRotation) + dp.y * sin(m_viewDataRotation));
                y.append(dp.x * -sin(m_viewDataRotation) + dp.y * cos(m_viewDataRotation));
                z.append(dp.z);
            }
            else
            {
                x.append((dp.x *  cos(m_viewDataRotation) + dp.y * sin(m_viewDataRotation)) * METERS_TO_FEET);
                y.append((dp.x * -sin(m_viewDataRotation) + dp.y * cos(m_viewDataRotation)) * METERS_TO_FEET);
                z.append((dp.z) * METERS_TO_FEET);
            }

            if (first)
            {
                xMin = xMax = x.back();
                yMin = yMax = y.back();
                zMin = zMax = z.back();

                first = false;
            }
            else
            {
                if (x.back() < xMin) xMin = x.back();
                if (x.back() > xMax) xMax = x.back();

                if (y.back() < yMin) yMin = y.back();
                if (y.back() > yMax) yMax = y.back();

                if (z.back() < zMin) zMin = z.back();
                if (z.back() > zMax) zMax = z.back();
            }
        }
    }

    mLeftView->clearPlottables();
    QCPCurve *left = new QCPCurve(mLeftView->xAxis, mLeftView->yAxis);
    left->setData(t, x, z);
    left->setPen(QPen(Qt::blue));
    mLeftView->addPlottable(left);
    setViewRange(mLeftView,
                 xMin, xMax,
                 zMin, zMax);

    mFrontView->clearPlottables();
    QCPCurve *front = new QCPCurve(mFrontView->xAxis, mFrontView->yAxis);
    front->setData(t, y, z);
    front->setPen(QPen(Qt::red));
    mFrontView->addPlottable(front);
    setViewRange(mFrontView,
                 yMin, yMax,
                 zMin, zMax);

    mTopView->clearPlottables();
    QCPCurve *top = new QCPCurve(mTopView->xAxis, mTopView->yAxis);
    top->setData(t, x, y);
    top->setPen(QPen(Qt::black));
    mTopView->addPlottable(top);
    setViewRange(mTopView,
                 xMin, xMax,
                 yMin, yMax);

    mTopView->addGraph();
    mTopView->graph(0)->addData(mTopView->xAxis->range().upper, (yMin + yMax) / 2);
    mTopView->graph(0)->setPen(QPen(Qt::red));
    mTopView->graph(0)->setLineStyle(QCPGraph::lsNone);
    mTopView->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 12));

    mTopView->addGraph();
    mTopView->graph(1)->addData((xMin + xMax) / 2, mTopView->yAxis->range().lower);
    mTopView->graph(1)->setPen(QPen(Qt::blue));
    mTopView->graph(1)->setLineStyle(QCPGraph::lsNone);
    mTopView->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 12));

    if (m_markActive)
    {
        QVector< double > xMark, yMark, zMark;

        if (m_units == PlotValue::Metric)
        {
            xMark.append(m_xView *  cos(m_viewDataRotation) + m_yView * sin(m_viewDataRotation));
            yMark.append(m_xView * -sin(m_viewDataRotation) + m_yView * cos(m_viewDataRotation));
            zMark.append(m_zView);
        }
        else
        {
            xMark.append((m_xView *  cos(m_viewDataRotation) + m_yView * sin(m_viewDataRotation)) * METERS_TO_FEET);
            yMark.append((m_xView * -sin(m_viewDataRotation) + m_yView * cos(m_viewDataRotation)) * METERS_TO_FEET);
            zMark.append((m_zView) * METERS_TO_FEET);
        }

        mLeftView->addGraph();
        mLeftView->graph(0)->setData(xMark, zMark);
        mLeftView->graph(0)->setPen(QPen(Qt::black));
        mLeftView->graph(0)->setLineStyle(QCPGraph::lsNone);
        mLeftView->graph(0)->setScatterStyle(QCPScatterStyle::ssDisc);

        mFrontView->addGraph();
        mFrontView->graph(0)->setData(yMark, zMark);
        mFrontView->graph(0)->setPen(QPen(Qt::black));
        mFrontView->graph(0)->setLineStyle(QCPGraph::lsNone);
        mFrontView->graph(0)->setScatterStyle(QCPScatterStyle::ssDisc);

        mTopView->addGraph();
        mTopView->graph(2)->setData(xMark, yMark);
        mTopView->graph(2)->setPen(QPen(Qt::black));
        mTopView->graph(2)->setLineStyle(QCPGraph::lsNone);
        mTopView->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
    }

    if (!m_data.empty())
    {
        QVector< double > xMark, yMark, zMark;

        for (int i = 0; i < m_waypoints.size(); ++i)
        {
            const DataPoint &dp0 = m_data[m_data.size() - 1];
            DataPoint &dp = m_waypoints[i];

            double distance = getDistance(dp0, dp);
            double bearing = getBearing(dp0, dp);

            dp.x = distance * sin(bearing);
            dp.y = distance * cos(bearing);

            if (m_units == PlotValue::Metric)
            {
                xMark.append(dp.x *  cos(m_viewDataRotation) + dp.y * sin(m_viewDataRotation));
                yMark.append(dp.x * -sin(m_viewDataRotation) + dp.y * cos(m_viewDataRotation));
                zMark.append(dp.z);
            }
            else
            {
                xMark.append((dp.x *  cos(m_viewDataRotation) + dp.y * sin(m_viewDataRotation)) * METERS_TO_FEET);
                yMark.append((dp.x * -sin(m_viewDataRotation) + dp.y * cos(m_viewDataRotation)) * METERS_TO_FEET);
                zMark.append((dp.z) * METERS_TO_FEET);
            }
        }

        QCPGraph *cur = mLeftView->addGraph();
        cur->setData(xMark, zMark);
        cur->setPen(QPen(Qt::black));
        cur->setLineStyle(QCPGraph::lsNone);
        cur->setScatterStyle(QCPScatterStyle::ssPlus);

        cur = mFrontView->addGraph();
        cur->setData(yMark, zMark);
        cur->setPen(QPen(Qt::black));
        cur->setLineStyle(QCPGraph::lsNone);
        cur->setScatterStyle(QCPScatterStyle::ssPlus);

        cur = mTopView->addGraph();
        cur->setData(xMark, yMark);
        cur->setPen(QPen(Qt::black));
        cur->setLineStyle(QCPGraph::lsNone);
        cur->setScatterStyle(QCPScatterStyle::ssPlus);
    }

    addNorthArrow(mTopView);

    mLeftView->replot();
    mFrontView->replot();
    mTopView->replot();
}

void MainWindow::addNorthArrow(
        QCustomPlot *plot)
{
    QPainter painter(plot);

    double mmPerPix = (double) painter.device()->widthMM() / painter.device()->width();
    double valPerPix = plot->xAxis->range().size() / plot->axisRect()->width();
    double valPerMM = valPerPix / mmPerPix;

    // rotated arrow
    QPointF north( 5 * sin(m_viewDataRotation),  5 * cos(m_viewDataRotation));
    QPointF south(-5 * sin(m_viewDataRotation), -5 * cos(m_viewDataRotation));

    // offset from corner
    north -= QPointF(10, 10);
    south -= QPointF(10, 10);

    // convert from mm to values
    north *= valPerMM;
    south *= valPerMM;

    QPointF corner (plot->xAxis->range().upper, plot->yAxis->range().upper);

    north += corner;
    south += corner;

    plot->removeItem(0);

    QCPItemLine *arrow = new QCPItemLine(plot);
    plot->addItem(arrow);
    arrow->start->setCoords(south);
    arrow->end->setCoords(north);
    arrow->setHead(QCPLineEnding::esSpikeArrow);
}

void MainWindow::setViewRange(
        QCustomPlot *plot,
        double xMin,
        double xMax,
        double yMin,
        double yMax)
{
    QPainter painter(plot);

    double xMMperPix = (double) painter.device()->widthMM() / painter.device()->width();
    double yMMperPix = (double) painter.device()->heightMM() / painter.device()->height();

    QRect rect = plot->axisRect()->rect();

    double xSpan = (xMax - xMin) * 1.2;
    double ySpan = (yMax - yMin) * 1.2;

    double xScale = xSpan / rect.width() / xMMperPix;
    double yScale = ySpan / rect.height() / yMMperPix;

    double scale = qMax(xScale, yScale);

    double xMid = (xMin + xMax) / 2;
    double yMid = (yMin + yMax) / 2;

    xMin = xMid - rect.width() * xMMperPix * scale / 2;
    xMax = xMid + rect.width() * xMMperPix * scale / 2;

    yMin = yMid - rect.height() * yMMperPix * scale / 2;
    yMax = yMid + rect.height() * yMMperPix * scale / 2;

    plot->xAxis->setRange(xMin, xMax);
    plot->yAxis->setRange(yMin, yMax);
}

double MainWindow::getXValue(
        const DataPoint &dp,
        XAxisType axis)
{
    return m_xValues[axis]->value(dp, m_units);
}

void MainWindow::on_actionElevation_triggered()
{
    m_yValues[Elevation]->setVisible(
                !m_yValues[Elevation]->visible());
    emit dataChanged();
}

void MainWindow::on_actionVerticalSpeed_triggered()
{
    m_yValues[VerticalSpeed]->setVisible(
                !m_yValues[VerticalSpeed]->visible());
    emit dataChanged();
}

void MainWindow::on_actionHorizontalSpeed_triggered()
{
    m_yValues[HorizontalSpeed]->setVisible(
                !m_yValues[HorizontalSpeed]->visible());
    emit dataChanged();
}

void MainWindow::on_actionTime_triggered()
{
    updateBottom(Time);
}

void MainWindow::on_actionDistance2D_triggered()
{
    updateBottom(Distance2D);
}

void MainWindow::on_actionDistance3D_triggered()
{
    updateBottom(Distance3D);
}

void MainWindow::updateBottom(
        XAxisType xAxis)
{
    QCPRange range = m_ui->plotArea->xAxis->range();

    DataPoint dpStart = interpolateData(range.lower);
    DataPoint dpEnd = interpolateData(range.upper);

    m_xAxis = xAxis;
    initPlotData();

    emit rangeChanged(QCPRange(getXValue(dpStart, m_xAxis),
                               getXValue(dpEnd, m_xAxis)));
    m_ui->plotArea->replot();

    updateBottomActions();
}

void MainWindow::updateBottomActions()
{
    m_ui->actionTime->setChecked(m_xAxis == Time);
    m_ui->actionDistance2D->setChecked(m_xAxis == Distance2D);
    m_ui->actionDistance3D->setChecked(m_xAxis == Distance3D);
}

void MainWindow::on_actionTotalSpeed_triggered()
{
    m_yValues[TotalSpeed]->setVisible(
                !m_yValues[TotalSpeed]->visible());
    emit dataChanged();
}

void MainWindow::on_actionDiveAngle_triggered()
{
    m_yValues[DiveAngle]->setVisible(
                !m_yValues[DiveAngle]->visible());
    emit dataChanged();
}

void MainWindow::on_actionCurvature_triggered()
{
    m_yValues[Curvature]->setVisible(
                !m_yValues[Curvature]->visible());
    emit dataChanged();
}

void MainWindow::on_actionGlideRatio_triggered()
{
    m_yValues[GlideRatio]->setVisible(
                !m_yValues[GlideRatio]->visible());
    emit dataChanged();
}

void MainWindow::on_actionHorizontalAccuracy_triggered()
{
    m_yValues[HorizontalAccuracy]->setVisible(
                !m_yValues[HorizontalAccuracy]->visible());
    emit dataChanged();
}

void MainWindow::on_actionVerticalAccuracy_triggered()
{
    m_yValues[VerticalAccuracy]->setVisible(
                !m_yValues[VerticalAccuracy]->visible());
    emit dataChanged();
}

void MainWindow::on_actionSpeedAccuracy_triggered()
{
    m_yValues[SpeedAccuracy]->setVisible(
                !m_yValues[SpeedAccuracy]->visible());
    emit dataChanged();
}

void MainWindow::on_actionNumberOfSatellites_triggered()
{
    m_yValues[NumberOfSatellites]->setVisible(
                !m_yValues[NumberOfSatellites]->visible());
    emit dataChanged();
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

void MainWindow::onTopView_mousePress(
        QMouseEvent *event)
{
    if (mTopView->axisRect()->rect().contains(event->pos()))
    {
        m_topViewBeginPos = event->pos() - mTopView->axisRect()->center();
        m_topViewPan = true;
    }
}

void MainWindow::onTopView_mouseRelease(
        QMouseEvent *)
{
    m_topViewPan = false;
}

void MainWindow::onTopView_mouseMove(
        QMouseEvent *event)
{
    if (m_topViewPan)
    {
        const double pi = 3.14159265359;

        QRect axisRect = mTopView->axisRect()->rect();
        QPoint endPos = event->pos() - axisRect.center();
/*
        double a1 = atan2((double) m_topViewBeginPos.x(), m_topViewBeginPos.y());
        double a2 = atan2((double) endPos.x(), endPos.y());
        double a = a2 - a1;
*/
        double a1 = (double) (m_topViewBeginPos.x() - axisRect.left()) / axisRect.width();
        double a2 = (double) (endPos.x() - axisRect.left()) / axisRect.width();
        double a = a2 - a1;

        while (a < -pi) a += 2 * pi;
        while (a >  pi) a -= 2 * pi;

        m_viewDataRotation -= a;

        updateViewData();

        m_topViewBeginPos = endPos;
    }

    onView_mouseMove(mTopView, event);
}

void MainWindow::onLeftView_mouseMove(
        QMouseEvent *event)
{
    onView_mouseMove(mLeftView, event);
}

void MainWindow::onFrontView_mouseMove(
        QMouseEvent *event)
{
    onView_mouseMove(mFrontView, event);
}

void MainWindow::onView_mouseMove(
        DataView *view,
        QMouseEvent *event)
{
    if (QCPCurve *curve = qobject_cast<QCPCurve *>(view->plottable(0)))
    {
        const QCPCurveDataMap *data = curve->data();

        double resultTime;
        double resultDistance = std::numeric_limits<double>::max();

        for (QCPCurveDataMap::const_iterator it = data->constBegin();
             it != data->constEnd() && (it + 1) != data->constEnd();
             ++ it)
        {
            QPointF pt1 = QPointF(view->xAxis->coordToPixel(it.value().key),
                                  view->yAxis->coordToPixel(it.value().value));
            QPointF pt2 = QPointF(view->xAxis->coordToPixel((it + 1).value().key),
                                  view->yAxis->coordToPixel((it + 1).value().value));

            double mu;
            double dist = sqrt(distSqrToLine(pt1, pt2, event->pos(), mu));

            if (dist < resultDistance)
            {
                double t1 = it.value().t;
                double t2 = (it + 1).value().t;

                resultTime = t1 + mu * (t2 - t1);
                resultDistance = dist;
            }
        }

        if (resultDistance < view->selectionTolerance())
        {
            int below = findIndexBelowT(resultTime);
            int above = findIndexAboveT(resultTime);

            if (below >= 0 && above < m_data.size())
            {
                const DataPoint &dp1 = m_data[below];
                const DataPoint &dp2 = m_data[above];

                double x1 = getXValue(dp1, m_xAxis);
                double x2 = getXValue(dp2, m_xAxis);

                double x = x1 + (resultTime - dp1.t) / (dp2.t - dp1.t) * (x2 - x1);

                onDataPlot_mark(x);
            }
            else
            {
                onDataPlot_clear();
            }
        }
        else
        {
            onDataPlot_clear();
        }
    }
}

double MainWindow::distSqrToLine(
        const QPointF &start,
        const QPointF &end,
        const QPointF &point,
        double &mu)
{
    QVector2D a(start);
    QVector2D b(end);
    QVector2D p(point);
    QVector2D v(b - a);

    double vLengthSqr = v.lengthSquared();
    if (!qFuzzyIsNull(vLengthSqr))
    {
        mu = QVector2D::dotProduct(p - a, v)/vLengthSqr;
        if (mu < 0)
        {
            mu = 0;
            return (a - p).lengthSquared();
        }
        else if (mu > 1)
        {
            mu = 1;
            return (b - p).lengthSquared();
        }
        else
        {
            return ((a + mu * v) - p).lengthSquared();
        }
    }
    else
    {
        mu = 0;
        return (a - p).lengthSquared();
    }
}

void MainWindow::on_actionImportGates_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Import"), "", tr("CSV Files (*.csv)"));

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

    updateViewData();
}

void MainWindow::on_actionPreferences_triggered()
{
    ConfigDialog dlg;

    dlg.setUnits(m_units);

    dlg.exec();

    if (m_units != dlg.units())
    {
        m_units = dlg.units();
        initPlotData();
        updateViewData();
    }
}


void MainWindow::setRange(
        const QCPRange &range)
{
    emit rangeChanged(range);
    updateViewData();
}

void MainWindow::setZero(
        double x)
{
    if (m_data.isEmpty()) return;

    DataPoint dp0 = interpolateData(x);

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

    double x0 = getXValue(dp0, m_xAxis);
    QCPRange range = m_ui->plotArea->xAxis->range();
    range = QCPRange (range.lower - x0, range.upper - x0);

    initPlotData();
    updateViewData();

    m_ui->plotArea->xAxis->setRange(range);
    m_ui->plotArea->replot();

    setTool(mPrevTool);
}

void MainWindow::setGround(
        double x)
{
    if (m_data.isEmpty()) return;

    DataPoint dp0 = interpolateData(x);

    for (int i = 0; i < m_data.size(); ++i)
    {
        DataPoint &dp = m_data[i];
        dp.alt -= dp0.alt;
    }

    QCPRange range = m_ui->plotArea->xAxis->range();

    initPlotData();
    updateViewData();

    m_ui->plotArea->xAxis->setRange(range);
    m_ui->plotArea->replot();

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

    emit toolChanged(tool);

    m_ui->actionPan->setChecked(tool == Pan);
    m_ui->actionZoom->setChecked(tool == Zoom);
    m_ui->actionMeasure->setChecked(tool == Measure);
    m_ui->actionZero->setChecked(tool == Zero);
    m_ui->actionGround->setChecked(tool == Ground);
}
