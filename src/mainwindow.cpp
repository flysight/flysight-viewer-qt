#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QToolTip>

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

    m_xAxisTitlesMetric.append(tr("Time (s)"));
    m_xAxisTitlesMetric.append(tr("Horizontal Distance (m)"));
    m_xAxisTitlesMetric.append(tr("Total Distance (m)"));

    m_xAxisTitlesImperial.append(tr("Time (s)"));
    m_xAxisTitlesImperial.append(tr("Horizontal Distance (ft)"));
    m_xAxisTitlesImperial.append(tr("Total Distance (ft)"));

    m_plotValues.append(new PlotElevation);
    m_plotValues.append(new PlotVerticalSpeed);
    m_plotValues.append(new PlotHorizontalSpeed);
    m_plotValues.append(new PlotTotalSpeed);
    m_plotValues.append(new PlotDiveAngle);
    m_plotValues.append(new PlotCurvature);
    m_plotValues.append(new PlotGlideRatio);

    foreach (PlotValue *v, m_plotValues)
    {
        v->readSettings();
    }

    m_ui->actionElevation->setChecked(true);
    m_ui->actionVerticalSpeed->setChecked(true);

    m_ui->vSplitter->setSizes(QList< int > () << 100 << 100);

    m_ui->topView->setMouseTracking(true);
    m_ui->leftView->setMouseTracking(true);
    m_ui->frontView->setMouseTracking(true);

    connect(m_ui->plotArea, SIGNAL(zoom(const QCPRange &)),
            this, SLOT(onDataPlot_zoom(const QCPRange &)));
    connect(m_ui->plotArea, SIGNAL(pan(double, double)),
            this, SLOT(onDataPlot_pan(double, double)));
    connect(m_ui->plotArea, SIGNAL(measure(double, double)),
            this, SLOT(onDataPlot_measure(double, double)));

    connect(m_ui->plotArea, SIGNAL(mark(double)),
            this, SLOT(onDataPlot_mark(double)));

    connect(m_ui->plotArea, SIGNAL(clear()),
            this, SLOT(onDataPlot_clear()));

    connect(m_ui->topView, SIGNAL(mousePress(QMouseEvent *)),
            this, SLOT(onTopView_mousePress(QMouseEvent *)));
    connect(m_ui->topView, SIGNAL(mouseRelease(QMouseEvent *)),
            this, SLOT(onTopView_mouseRelease(QMouseEvent *)));
    connect(m_ui->topView, SIGNAL(mouseMove(QMouseEvent *)),
            this, SLOT(onTopView_mouseMove(QMouseEvent *)));

    connect(m_ui->leftView, SIGNAL(mouseMove(QMouseEvent *)),
            this, SLOT(onLeftView_mouseMove(QMouseEvent *)));
    connect(m_ui->frontView, SIGNAL(mouseMove(QMouseEvent *)),
            this, SLOT(onFrontView_mouseMove(QMouseEvent *)));

    connect(m_ui->plotArea, SIGNAL(expand(QPoint, QPoint)),
            this, SLOT(onPlotArea_expand(QPoint, QPoint)));

    m_ui->plotArea->setTool(DataPlot::Pan);
    updateTool();
}

MainWindow::~MainWindow()
{
    while (!m_plotValues.isEmpty())
    {
        delete m_plotValues.takeLast();
    }

    delete m_ui;
}

void MainWindow::closeEvent(
        QCloseEvent *event)
{
    foreach (PlotValue *v, m_plotValues)
    {
        v->writeSettings();
    }

    event->accept();
}

void MainWindow::onDataPlot_zoom(const QCPRange &range)
{
    m_ui->plotArea->xAxis->setRange(range);

    updateYRanges();
    updateViewData();
}

void MainWindow::onDataPlot_pan(
        double xStart,
        double xEnd)
{
    QCPRange range = m_ui->plotArea->xAxis->range();

    double diff = xStart - xEnd;
    range = QCPRange(range.lower + diff, range.upper + diff);

    m_ui->plotArea->xAxis->setRange(range);

    updateYRanges();
    updateViewData();
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

    if (m_units == PlotValue::Metric)
    {
        const double val = getXValue(dpEnd, m_xAxis)
                - getXValue(dpStart, m_xAxis);
        status += QString("<tr style='color:black;'><td>%1</td><td>%2</td><td>(%3%4)</td></tr>")
                .arg(m_xAxisTitlesMetric[m_xAxis])
                .arg(getXValue(dpEnd, m_xAxis))
                .arg(val < 0 ? "" : "+")
                .arg(val);
    }
    else
    {
        const double val = getXValue(dpEnd, m_xAxis)
                - getXValue(dpStart, m_xAxis);
        status += QString("<tr style='color:black;'><td>%1</td><td>%2</td><td>(%3%4)</td></tr>")
                .arg(m_xAxisTitlesImperial[m_xAxis])
                .arg(getXValue(dpEnd, m_xAxis))
                .arg(val < 0 ? "" : "+")
                .arg(val);
    }

    for (int i = 0; i < yaLast; ++i)
    {
        if (m_plotValues[i]->visible())
        {
            const double val = m_plotValues[i]->value(dpEnd, m_units)
                    - m_plotValues[i]->value(dpStart, m_units);
            status += QString("<tr style='color:%5;'><td>%1</td><td>%2</td><td>(%3%4)</td></tr>")
                    .arg(m_plotValues[(YAxisType) i]->title(m_units))
                    .arg(m_plotValues[i]->value(dpEnd, m_units))
                    .arg(val < 0 ? "" : "+")
                    .arg(val)
                    .arg(m_plotValues[i]->color().name());
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

    mark(interpolateData(xMark));

    QString status;

    status = QString("<table width='200'>");

    if (m_units == PlotValue::Metric)
    {
        status += QString("<tr style='color:black;'><td>%1</td><td>%2</td></tr>")
                .arg(m_xAxisTitlesMetric[m_xAxis])
                .arg(m_xPlot);
    }
    else
    {
        status += QString("<tr style='color:black;'><td>%1</td><td>%2</td></tr>")
                .arg(m_xAxisTitlesImperial[m_xAxis])
                .arg(m_xPlot);
    }

    for (int i = 0; i < yaLast; ++i)
    {
        if (m_plotValues[i]->visible())
        {
            status += QString("<tr style='color:%3;'><td>%1</td><td>%2</td></tr>")
                    .arg(m_plotValues[(YAxisType) i]->title(m_units))
                    .arg(m_yPlot[i])
                    .arg(m_plotValues[i]->color().name());
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
        if (m_plotValues[i]->visible())
        {
            m_yPlot[i] = m_plotValues[i]->value(dp, m_units);
        }
    }

    m_xView = dp.x;
    m_yView = dp.y;
    m_zView = dp.hMSL;

    m_markActive = true;

    updatePlotData();
    updateViewData();
}

void MainWindow::onDataPlot_clear()
{
    m_markActive = false;

    QToolTip::hideText();

    updatePlotData();
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

        if (getXValue(dp, m_xAxis) < x)
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

        if (dp.t < t)
            return i + 1;
    }

    return 0;
}

void MainWindow::updateYRanges()
{
    const QCPRange &range = m_ui->plotArea->xAxis->range();

    int k = 0;
    for (int j = 0; j < yaLast; ++j)
    {
        if (!m_plotValues[j]->visible()) continue;

        double yMin, yMax;
        int iMin, iMax;

        bool first = true;

        for (int i = 0; i < m_data.size(); ++i)
        {
            DataPoint &dp = m_data[i];

            if (range.contains(getXValue(dp, m_xAxis)))
            {
                double y = m_plotValues[j]->value(dp, m_units);

                if (first)
                {
                    yMin = yMax = y;
                    iMin = iMax = i;
                    first = false;
                }
                else
                {
                    if (y < yMin) yMin = y;
                    if (y > yMax) yMax = y;
                    iMax = i;
                }
            }
        }

        if (!first)
            m_ui->plotArea->axisRect()->axis(QCPAxis::atLeft, k++)->setRange(yMin, yMax);
    }

    m_ui->plotArea->replot();
}

void MainWindow::on_actionImport_triggered()
{
    const double pi = 3.14159265359;

    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get root folder
    QString rootFolder = settings.value("folder").toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import"), rootFolder, tr("CSV Files (*.csv)"));

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        // TODO: Error message
        return;
    }

    // Remember root folder
    settings.setValue("folder", QFileInfo(fileName).absolutePath());

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
        pt.lat  = cols[1].toDouble();
        pt.lon  = cols[2].toDouble();
        pt.hMSL = cols[3].toDouble();
        pt.velN = cols[4].toDouble();
        pt.velE = cols[5].toDouble();
        pt.velD = cols[6].toDouble();

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
        dp.z = dp.hMSL - dp0.hMSL;

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

        if (i - 1 >= 0 && i + 1 < m_data.size ())
        {
            const DataPoint &dpPrev = m_data[i - 1];
            const DataPoint &dpNext = m_data[i + 1];

            double t0 = dpPrev.t;
            double y0 = atan2(dpPrev.velD, sqrt(dpPrev.velE * dpPrev.velE + dpPrev.velN * dpPrev.velN)) / pi * 180;

            double t1 = dp.t;
            double y1 = atan2(dp.velD, sqrt(dp.velE * dp.velE + dp.velN * dp.velN)) / pi * 180;

            double t2 = dpNext.t;
            double y2 = atan2(dpNext.velD, sqrt(dpNext.velE * dpNext.velE + dpNext.velN * dpNext.velN)) / pi * 180;

            double sumx = t0 + t1 + t2;
            double sumy = y0 + y1 + y2;
            double sumxx = t0 * t0 + t1 * t1 + t2 * t2;
            double sumxy = t0 * y0 + t1 * y1 + t2 * y2;

            dp.curv = (3 * sumxy - sumx * sumy) /
                    (3 * sumxx - sumx * sumx);
        }
        if (i - 1 >= 0)
        {
            const DataPoint &dpPrev = m_data[i - 1];

            double t0 = dpPrev.t;
            double y0 = atan2(dpPrev.velD, sqrt(dpPrev.velE * dpPrev.velE + dpPrev.velN * dpPrev.velN)) / pi * 180;

            double t1 = dp.t;
            double y1 = atan2(dp.velD, sqrt(dp.velE * dp.velE + dp.velN * dp.velN)) / pi * 180;

            dp.curv = (y1 - y0) / (t1 - t0);
        }
        else
        {
            const DataPoint &dpNext = m_data[i + 1];

            double t1 = dp.t;
            double y1 = atan2(dp.velD, sqrt(dp.velE * dp.velE + dp.velN * dp.velN)) / pi * 180;

            double t2 = dpNext.t;
            double y2 = atan2(dpNext.velD, sqrt(dpNext.velE * dpNext.velE + dpNext.velN * dpNext.velN)) / pi * 180;

            dp.curv = (y2 - y1) / (t2 - t1);
        }
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

    if (m_units == PlotValue::Metric)
    {
        m_ui->plotArea->xAxis->setLabel(m_xAxisTitlesMetric[m_xAxis]);
    }
    else
    {
        m_ui->plotArea->xAxis->setLabel(m_xAxisTitlesImperial[m_xAxis]);
    }

    updatePlotData();
}

void MainWindow::updatePlotData()
{
    QVector< double > x;
    for (int i = 0; i < m_data.size(); ++i)
    {
        DataPoint &dp = m_data[i];
        x.append(getXValue(dp, m_xAxis));
    }

    m_ui->plotArea->clearPlottables();
    while (m_ui->plotArea->axisRect()->axisCount(QCPAxis::atLeft) > 0)
    {
        m_ui->plotArea->axisRect()->removeAxis(
                    m_ui->plotArea->axisRect()->axis(QCPAxis::atLeft, 0));
    }

    for (int j = 0; j < yaLast; ++j)
    {
        if (!m_plotValues[j]->visible()) continue;

        QVector< double > y;
        for (int i = 0; i < m_data.size(); ++i)
        {
            DataPoint &dp = m_data[i];
            y.append(m_plotValues[j]->value(dp, m_units));
        }

        QCPGraph *graph = m_ui->plotArea->addGraph(
                    m_ui->plotArea->axisRect()->axis(QCPAxis::atBottom),
                    m_plotValues[j]->addAxis(m_ui->plotArea, m_units));
        graph->setData(x, y);
        graph->setPen(QPen(m_plotValues[j]->color()));
    }

    if (m_markActive)
    {
        QVector< double > xMark, yMark;

        xMark.append(m_xPlot);

        int k = 0;
        for (int i = 0; i < yaLast; ++i)
        {
            if (m_plotValues[i]->visible())
            {
                yMark.clear();
                yMark.append(m_yPlot[i]);

                QCPGraph *graph = m_ui->plotArea->addGraph(
                            m_ui->plotArea->xAxis,
                            m_ui->plotArea->axisRect()->axis(QCPAxis::atLeft, k++));

                graph->setData(xMark, yMark);
                graph->setPen(QPen(Qt::black));
                graph->setLineStyle(QCPGraph::lsNone);
                graph->setScatterStyle(QCPScatterStyle::ssDisc);
            }
        }
    }

    updateYRanges();
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
                z.append(dp.hMSL - m_data.back().hMSL);
            }
            else
            {
                x.append((dp.x *  cos(m_viewDataRotation) + dp.y * sin(m_viewDataRotation)) * METERS_TO_FEET);
                y.append((dp.x * -sin(m_viewDataRotation) + dp.y * cos(m_viewDataRotation)) * METERS_TO_FEET);
                z.append((dp.hMSL - m_data.back().hMSL) * METERS_TO_FEET);
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

    m_ui->leftView->clearPlottables();
    QCPCurve *left = new QCPCurve(m_ui->leftView->xAxis, m_ui->leftView->yAxis);
    left->setData(t, x, z);
    left->setPen(QPen(Qt::blue));
    m_ui->leftView->addPlottable(left);
    setViewRange(m_ui->leftView,
                 xMin, xMax,
                 zMin, zMax);

    m_ui->frontView->clearPlottables();
    QCPCurve *front = new QCPCurve(m_ui->frontView->xAxis, m_ui->frontView->yAxis);
    front->setData(t, y, z);
    front->setPen(QPen(Qt::red));
    m_ui->frontView->addPlottable(front);
    setViewRange(m_ui->frontView,
                 yMin, yMax,
                 zMin, zMax);

    m_ui->topView->clearPlottables();
    QCPCurve *top = new QCPCurve(m_ui->topView->xAxis, m_ui->topView->yAxis);
    top->setData(t, x, y);
    top->setPen(QPen(Qt::black));
    m_ui->topView->addPlottable(top);
    setViewRange(m_ui->topView,
                 xMin, xMax,
                 yMin, yMax);

    m_ui->topView->addGraph();
    m_ui->topView->graph(0)->addData(m_ui->topView->xAxis->range().upper, (yMin + yMax) / 2);
    m_ui->topView->graph(0)->setPen(QPen(Qt::red));
    m_ui->topView->graph(0)->setLineStyle(QCPGraph::lsNone);
    m_ui->topView->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 12));

    m_ui->topView->addGraph();
    m_ui->topView->graph(1)->addData((xMin + xMax) / 2, m_ui->topView->yAxis->range().lower);
    m_ui->topView->graph(1)->setPen(QPen(Qt::blue));
    m_ui->topView->graph(1)->setLineStyle(QCPGraph::lsNone);
    m_ui->topView->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 12));

    if (m_markActive)
    {
        QVector< double > xMark, yMark, zMark;

        if (m_units == PlotValue::Metric)
        {
            xMark.append(m_xView *  cos(m_viewDataRotation) + m_yView * sin(m_viewDataRotation));
            yMark.append(m_xView * -sin(m_viewDataRotation) + m_yView * cos(m_viewDataRotation));
            zMark.append(m_zView - m_data.back().hMSL);
        }
        else
        {
            xMark.append((m_xView *  cos(m_viewDataRotation) + m_yView * sin(m_viewDataRotation)) * METERS_TO_FEET);
            yMark.append((m_xView * -sin(m_viewDataRotation) + m_yView * cos(m_viewDataRotation)) * METERS_TO_FEET);
            zMark.append((m_zView - m_data.back().hMSL) * METERS_TO_FEET);
        }

        m_ui->leftView->addGraph();
        m_ui->leftView->graph(0)->setData(xMark, zMark);
        m_ui->leftView->graph(0)->setPen(QPen(Qt::black));
        m_ui->leftView->graph(0)->setLineStyle(QCPGraph::lsNone);
        m_ui->leftView->graph(0)->setScatterStyle(QCPScatterStyle::ssDisc);

        m_ui->frontView->addGraph();
        m_ui->frontView->graph(0)->setData(yMark, zMark);
        m_ui->frontView->graph(0)->setPen(QPen(Qt::black));
        m_ui->frontView->graph(0)->setLineStyle(QCPGraph::lsNone);
        m_ui->frontView->graph(0)->setScatterStyle(QCPScatterStyle::ssDisc);

        m_ui->topView->addGraph();
        m_ui->topView->graph(2)->setData(xMark, yMark);
        m_ui->topView->graph(2)->setPen(QPen(Qt::black));
        m_ui->topView->graph(2)->setLineStyle(QCPGraph::lsNone);
        m_ui->topView->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
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
                zMark.append(dp.hMSL - m_data.back().hMSL);
            }
            else
            {
                xMark.append((dp.x *  cos(m_viewDataRotation) + dp.y * sin(m_viewDataRotation)) * METERS_TO_FEET);
                yMark.append((dp.x * -sin(m_viewDataRotation) + dp.y * cos(m_viewDataRotation)) * METERS_TO_FEET);
                zMark.append((dp.hMSL - m_data.back().hMSL) * METERS_TO_FEET);
            }
        }

        QCPGraph *cur = m_ui->leftView->addGraph();
        cur->setData(xMark, zMark);
        cur->setPen(QPen(Qt::black));
        cur->setLineStyle(QCPGraph::lsNone);
        cur->setScatterStyle(QCPScatterStyle::ssPlus);

        cur = m_ui->frontView->addGraph();
        cur->setData(yMark, zMark);
        cur->setPen(QPen(Qt::black));
        cur->setLineStyle(QCPGraph::lsNone);
        cur->setScatterStyle(QCPScatterStyle::ssPlus);

        cur = m_ui->topView->addGraph();
        cur->setData(xMark, yMark);
        cur->setPen(QPen(Qt::black));
        cur->setLineStyle(QCPGraph::lsNone);
        cur->setScatterStyle(QCPScatterStyle::ssPlus);
    }

    addNorthArrow(m_ui->topView);

    m_ui->leftView->replot();
    m_ui->frontView->replot();
    m_ui->topView->replot();
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
    switch (axis)
    {
    case Time:
        return dp.t;
    case Distance2D:
        if (m_units == PlotValue::Metric) return dp.dist2D;
        else                   return dp.dist2D * METERS_TO_FEET;
    case Distance3D:
        if (m_units == PlotValue::Metric) return dp.dist3D;
        else                   return dp.dist3D * METERS_TO_FEET;
    default:
        return 0;
    }
}

void MainWindow::on_actionElevation_triggered()
{
    m_plotValues[Elevation]->setVisible(
                !m_plotValues[Elevation]->visible());
    updatePlotData();
}

void MainWindow::on_actionVerticalSpeed_triggered()
{
    m_plotValues[VerticalSpeed]->setVisible(
                !m_plotValues[VerticalSpeed]->visible());
    updatePlotData();
}

void MainWindow::on_actionHorizontalSpeed_triggered()
{
    m_plotValues[HorizontalSpeed]->setVisible(
                !m_plotValues[HorizontalSpeed]->visible());
    updatePlotData();
}

void MainWindow::on_actionMetric_triggered()
{
    m_units = PlotValue::Metric;
    initPlotData();
    updateViewData();
}

void MainWindow::on_actionImperial_triggered()
{
    m_units = PlotValue::Imperial;
    initPlotData();
    updateViewData();
}

void MainWindow::on_actionTime_triggered()
{
    m_xAxis = Time;
    initPlotData();
}

void MainWindow::on_actionDistance2D_triggered()
{
    m_xAxis = Distance2D;
    initPlotData();
}

void MainWindow::on_actionDistance3D_triggered()
{
    m_xAxis = Distance3D;
    initPlotData();
}

void MainWindow::on_actionTotalSpeed_triggered()
{
    m_plotValues[TotalSpeed]->setVisible(
                !m_plotValues[TotalSpeed]->visible());
    updatePlotData();
}

void MainWindow::on_actionDiveAngle_triggered()
{
    m_plotValues[DiveAngle]->setVisible(
                !m_plotValues[DiveAngle]->visible());
    updatePlotData();
}

void MainWindow::on_actionCurvature_triggered()
{
    m_plotValues[Curvature]->setVisible(
                !m_plotValues[Curvature]->visible());
    updatePlotData();
}

void MainWindow::on_actionGlideRatio_triggered()
{
    m_plotValues[GlideRatio]->setVisible(
                !m_plotValues[GlideRatio]->visible());
    updatePlotData();
}

void MainWindow::on_actionPan_triggered()
{
    m_ui->plotArea->setTool(DataPlot::Pan);
    updateTool();
}

void MainWindow::on_actionZoom_triggered()
{
    m_ui->plotArea->setTool(DataPlot::Zoom);
    updateTool();
}

void MainWindow::on_actionMeasure_triggered()
{
    m_ui->plotArea->setTool(DataPlot::Measure);
    updateTool();
}

void MainWindow::updateTool()
{
    m_ui->actionPan->setChecked(m_ui->plotArea->tool() == DataPlot::Pan);
    m_ui->actionZoom->setChecked(m_ui->plotArea->tool() == DataPlot::Zoom);
    m_ui->actionMeasure->setChecked(m_ui->plotArea->tool() == DataPlot::Measure);
}

void MainWindow::onTopView_mousePress(
        QMouseEvent *event)
{
    if (m_ui->topView->axisRect()->rect().contains(event->pos()))
    {
        m_topViewBeginPos = event->pos() - m_ui->topView->axisRect()->center();
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

        QRect axisRect = m_ui->topView->axisRect()->rect();
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

    onView_mouseMove(m_ui->topView, event);
}

void MainWindow::onLeftView_mouseMove(
        QMouseEvent *event)
{
    onView_mouseMove(m_ui->leftView, event);
}

void MainWindow::onFrontView_mouseMove(
        QMouseEvent *event)
{
    onView_mouseMove(m_ui->frontView, event);
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

void MainWindow::onPlotArea_expand(
        QPoint pos,
        QPoint angleDelta)
{
    QRect windowRect = m_ui->plotArea->rect();
    QRect axisRect = m_ui->plotArea->axisRect()->rect();

    qDebug() << windowRect;
    qDebug() << axisRect;

    double multiplier = exp((double) angleDelta.y() / 500);

    qDebug() << multiplier;

    QRect newAxisRect(pos.x() + (axisRect.x() - pos.x()) * multiplier,
                      pos.y() + (axisRect.y() - pos.y()) * multiplier,
                      axisRect.width() * multiplier,
                      axisRect.height() * multiplier);

    qDebug() << newAxisRect;

    QRect newWindowRect(newAxisRect.x() + windowRect.x() - axisRect.x(),
                        newAxisRect.y() + windowRect.y() - axisRect.y(),
                        newAxisRect.width() + windowRect.width() - axisRect.width(),
                        newAxisRect.height() + windowRect.height() - axisRect.height());

    qDebug() << newWindowRect;

    QRect finalWindowRect(QPoint(m_ui->vSplitter->rect().left(),
                                 m_ui->vSplitter->rect().top()),
                          QPoint(m_ui->vSplitter->rect().right(),
                                 qMin(m_ui->vSplitter->rect().bottom(), newWindowRect.bottom())));

    qDebug() << finalWindowRect;

    QRect finalAxisRect(finalWindowRect.x() + axisRect.x() - windowRect.x(),
                        finalWindowRect.y() + axisRect.y() - windowRect.y(),
                        finalWindowRect.width() + axisRect.width() - windowRect.width(),
                        finalWindowRect.height() + axisRect.height() - windowRect.height());

    qDebug() << finalAxisRect;

    QCPRange xRange = m_ui->plotArea->xAxis->range();
    QCPRange y1Range = m_ui->plotArea->yAxis->range();
    QCPRange y2Range = m_ui->plotArea->yAxis2->range();

    qDebug() << xRange.lower << xRange.upper;
    qDebug() << y1Range.lower << y1Range.upper;
    qDebug() << y2Range.lower << y2Range.upper;

    double xScale = xRange.size() / newAxisRect.width();

    qDebug() << xScale;

    QCPRange newXRange(xRange.lower + (finalAxisRect.left() - newAxisRect.left()) * xScale,
                       xRange.lower + (finalAxisRect.right() - newAxisRect.left()) * xScale);

    qDebug() << newXRange.lower << newXRange.upper;

    m_ui->plotArea->xAxis->setRange(newXRange);

    m_ui->vSplitter->setSizes(QList< int > () << finalWindowRect.height() << m_ui->vSplitter->rect().height() - finalWindowRect.height());

    updateYRanges();
    updateViewData();
}
