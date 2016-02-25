#include <QToolTip>

#include "dataplot.h"
#include "mainwindow.h"

DataPlot::DataPlot(QWidget *parent) :
    QCustomPlot(parent),
    mMainWindow(0),
    m_dragging(false),
    m_xAxisType(Time)
{
    // Initialize window
    setMouseTracking(true);
    setCursor(QCursor(Qt::ArrowCursor));

    // Intitialize plot area
    initPlot();

    // Read plot settings
    readSettings();
}

DataPlot::~DataPlot()
{
    // Write plot settings
    writeSettings();

    // Save plot state
    foreach (PlotValue *v, m_yValues)
    {
        v->writeSettings();
    }

    // Delete plots
    while (!m_xValues.isEmpty())
    {
        delete m_xValues.takeLast();
    }

    while (!m_yValues.isEmpty())
    {
        delete m_yValues.takeLast();
    }
}

void DataPlot::initPlot()
{
    m_xValues.append(new PlotTime);
    m_xValues.append(new PlotDistance2D);
    m_xValues.append(new PlotDistance3D);

    foreach (PlotValue *v, m_xValues)
    {
        v->readSettings();
    }

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
    m_yValues.append(new PlotAcceleration);
    m_yValues.append(new PlotTotalEnergy);
    m_yValues.append(new PlotEnergyRate);
    m_yValues.append(new PlotLift);
    m_yValues.append(new PlotDrag);
    m_yValues.append(new PlotCourse);
    m_yValues.append(new PlotCourseRate);
    m_yValues.append(new PlotCourseAccuracy);

    foreach (PlotValue *v, m_yValues)
    {
        v->readSettings();
    }
}

void DataPlot::readSettings()
{
    QSettings settings("FlySight", "Viewer");

    settings.beginGroup("mainWindow");
    m_xAxisType = (XAxisType) settings.value("xAxis", m_xAxisType).toInt();
    settings.endGroup();
}

void DataPlot::writeSettings()
{
    QSettings settings("FlySight", "Viewer");

    settings.beginGroup("mainWindow");
    settings.setValue("xAxis", m_xAxisType);
    settings.endGroup();
}

void DataPlot::mousePressEvent(
        QMouseEvent *event)
{
    if (axisRect()->rect().contains(event->pos()))
    {
        m_beginPos = event->pos();
        m_dragging = true;
        update();
    }

    QCustomPlot::mousePressEvent(event);
}

void DataPlot::mouseReleaseEvent(
        QMouseEvent *event)
{
    QPoint endPos = event->pos();

    MainWindow::Tool tool = mMainWindow->tool();
    if (m_dragging && tool == MainWindow::Zoom)
    {
        QCPRange range(qMin(xAxis->pixelToCoord(m_beginPos.x()),
                            xAxis->pixelToCoord(endPos.x())),
                       qMax(xAxis->pixelToCoord(m_beginPos.x()),
                            xAxis->pixelToCoord(endPos.x())));

        setRange(range);
    }
    if (m_dragging && tool == MainWindow::Zero)
    {
        DataPoint dpEnd = interpolateDataX(xAxis->pixelToCoord(endPos.x()));
        mMainWindow->setZero(dpEnd.t);
    }
    if (m_dragging && tool == MainWindow::Ground)
    {
        DataPoint dpEnd = interpolateDataX(xAxis->pixelToCoord(endPos.x()));
        mMainWindow->setGround(dpEnd.t);
    }
    if (m_dragging && tool == MainWindow::Course)
    {
        DataPoint dpEnd = interpolateDataX(xAxis->pixelToCoord(endPos.x()));
        mMainWindow->setCourse(dpEnd.t);
    }

    if (m_dragging)
    {
        m_dragging = false;
        replot();
    }

    QCustomPlot::mouseReleaseEvent(event);
}

void DataPlot::mouseMoveEvent(
        QMouseEvent *event)
{
    m_cursorPos = event->pos();

    MainWindow::Tool tool = mMainWindow->tool();
    if (m_dragging && tool == MainWindow::Pan)
    {
        QCPRange range = xAxis->range();

        double diff = xAxis->pixelToCoord(m_beginPos.x())
                - xAxis->pixelToCoord(m_cursorPos.x());
        range = QCPRange(range.lower + diff, range.upper + diff);

        setRange(range);

        m_beginPos = m_cursorPos;
    }

    if (axisRect()->rect().contains(event->pos()))
    {
        if (m_dragging && tool == MainWindow::Measure)
        {
            setMark(xAxis->pixelToCoord(m_beginPos.x()),
                    xAxis->pixelToCoord(m_cursorPos.x()));
        }
        else
        {
            setMark(xAxis->pixelToCoord(m_cursorPos.x()));
        }
    }
    else
    {
        mMainWindow->clearMark();
        QToolTip::hideText();
    }

    update();

    QCustomPlot::mouseMoveEvent(event);
}

void DataPlot::wheelEvent(
        QWheelEvent *event)
{
    if (axisRect()->rect().contains(event->pos()))
    {
        double multiplier = exp((double) -event->angleDelta().y() / 500);

        double x = xAxis->pixelToCoord(event->pos().x());

        QCPRange range = xAxis->range();

        range = QCPRange(
                    x + (range.lower - x) * multiplier,
                    x + (range.upper - x) * multiplier);

        setRange(range);
    }
}

void DataPlot::leaveEvent(
        QEvent *)
{
    mMainWindow->clearMark();
    m_cursorPos = QPoint();
    update();
}

void DataPlot::paintEvent(
        QPaintEvent *event)
{
    QCustomPlot::paintEvent(event);

    MainWindow::Tool tool = mMainWindow->tool();
    if (m_dragging && (tool == MainWindow::Zoom || tool == MainWindow::Measure))
    {
        QPainter painter(this);

        painter.setPen(QPen(Qt::black));
        painter.drawLine(m_beginPos.x(), axisRect()->rect().top(), m_beginPos.x(), axisRect()->rect().bottom());
        if (axisRect()->rect().left() <= m_cursorPos.x() && m_cursorPos.x() <= axisRect()->rect().right())
        {
            painter.drawLine(m_cursorPos.x(), axisRect()->rect().top(), m_cursorPos.x(), axisRect()->rect().bottom());
        }

        QRect shading(
                    qMin(m_beginPos.x(), m_cursorPos.x()),
                    axisRect()->rect().top(),
                    qAbs(m_beginPos.x() - m_cursorPos.x()),
                    axisRect()->rect().height());

        painter.fillRect(shading & axisRect()->rect(), QColor(181, 217, 42, 64));
    }
    else
    {
        if (axisRect()->rect().contains(m_cursorPos))
        {
            QPainter painter(this);

            painter.setPen(QPen(Qt::black));
            painter.drawLine(m_cursorPos.x(), axisRect()->rect().top(), m_cursorPos.x(), axisRect()->rect().bottom());
            painter.drawLine(axisRect()->rect().left(), m_cursorPos.y(), axisRect()->rect().right(), m_cursorPos.y());
        }
    }
}

void DataPlot::setMark(
        double start,
        double end)
{
    DataPoint dpStart = interpolateDataX(start);
    DataPoint dpEnd = interpolateDataX(end);

    mMainWindow->setMark(dpStart.t, dpEnd.t);

    if (mMainWindow->dataSize() == 0) return;

    const int jStart = findIndexAboveX(start);
    const int jEnd = findIndexBelowX(end);

    QString status;
    status = QString("<u>%1 %2.%3 UTC</u>")
            .arg(dpEnd.dateTime.date().toString(Qt::ISODate))
            .arg(dpEnd.dateTime.time().toString(Qt::ISODate))
            .arg(QString("%1").arg(dpEnd.dateTime.time().msec(), 3, 10, QChar('0')));

    status += QString("<table width='300'>");

    double change = m_xValues[Time]->value(dpEnd, mMainWindow->units())
            - m_xValues[Time]->value(dpStart, mMainWindow->units());
    status += QString("<tr style='color:black;'><td>%1</td><td>%2</td><td>(%3%4)</td></tr>")
            .arg(m_xValues[Time]->title(mMainWindow->units()))
            .arg(m_xValues[Time]->value(dpEnd, mMainWindow->units()))
            .arg(change < 0 ? "" : "+")
            .arg(change);

    if (m_xAxisType != Time)
    {
        change = xValue()->value(dpEnd, mMainWindow->units())
                - xValue()->value(dpStart, mMainWindow->units());
        status += QString("<tr style='color:black;'><td>%1</td><td>%2</td><td>(%3%4)</td></tr>")
                .arg(xValue()->title(mMainWindow->units()))
                .arg(xValue()->value(dpEnd, mMainWindow->units()))
                .arg(change < 0 ? "" : "+")
                .arg(change);
    }

    for (int i = 0; i < yaLast; ++i)
    {
        if (yValue(i)->visible())
        {
            double sum = 0;
            if (jStart > jEnd)
            {
                const double dx = xValue()->value(dpEnd, mMainWindow->units())
                        - xValue()->value(dpStart, mMainWindow->units());
                const double avg = (yValue(i)->value(dpEnd, mMainWindow->units())
                        + yValue(i)->value(dpStart, mMainWindow->units())) / 2;

                sum = avg * dx ;
            }
            else
            {
                const DataPoint &dp0 = mMainWindow->dataPoint(jStart);

                double dx = xValue()->value(dp0, mMainWindow->units())
                        - xValue()->value(dpStart, mMainWindow->units());
                double avg = (yValue(i)->value(dp0, mMainWindow->units())
                        + yValue(i)->value(dpStart, mMainWindow->units())) / 2;

                sum = avg * dx ;

                for (int j = jStart ; j < jEnd ; ++ j)
                {
                    const DataPoint &dp1 = mMainWindow->dataPoint(j);
                    const DataPoint &dp2 = mMainWindow->dataPoint(j + 1);

                    dx = xValue()->value(dp2, mMainWindow->units())
                            - xValue()->value(dp1, mMainWindow->units());
                    avg = (yValue(i)->value(dp2, mMainWindow->units())
                            + yValue(i)->value(dp1, mMainWindow->units())) / 2;

                    sum += avg * dx ;
                }

                const DataPoint &dpN = mMainWindow->dataPoint(jEnd);

                dx = xValue()->value(dpEnd, mMainWindow->units())
                        - xValue()->value(dpN, mMainWindow->units());
                avg = (yValue(i)->value(dpEnd, mMainWindow->units())
                        + yValue(i)->value(dpN, mMainWindow->units())) / 2;

                sum += avg * dx ;
            }

            const double dx = xValue()->value(dpEnd, mMainWindow->units())
                    - xValue()->value(dpStart, mMainWindow->units());

            change = yValue(i)->value(dpEnd, mMainWindow->units())
                    - yValue(i)->value(dpStart, mMainWindow->units());
            status += QString("<tr style='color:%5;'><td>%1</td><td>%2</td><td>(%3%4)</td><td>[%6]</td></tr>")
                    .arg(yValue(i)->title(mMainWindow->units()))
                    .arg(yValue(i)->value(dpEnd, mMainWindow->units()))
                    .arg(change < 0 ? "" : "+")
                    .arg(change)
                    .arg(yValue(i)->color().name())
                    .arg(sum / dx);
        }
    }

    status += QString("</table>");

    QToolTip::showText(QCursor::pos(), status);
}

void DataPlot::setMark(
        double mark)
{
    if (mMainWindow->dataSize() == 0) return;

    DataPoint dp = interpolateDataX(mark);
    mMainWindow->setMark(dp.t);

    QString status;
    status = QString("<u>%1 %2.%3 UTC</u>")
            .arg(dp.dateTime.date().toString(Qt::ISODate))
            .arg(dp.dateTime.time().toString(Qt::ISODate))
            .arg(QString("%1").arg(dp.dateTime.time().msec(), 3, 10, QChar('0')));

    status += QString("<table width='200'>");

    status += QString("<tr style='color:black;'><td>%1</td><td>%2</td></tr>")
            .arg(m_xValues[Time]->title(mMainWindow->units()))
            .arg(m_xValues[Time]->value(dp, mMainWindow->units()));

    if (m_xAxisType != Time)
    {
        status += QString("<tr style='color:black;'><td>%1</td><td>%2</td></tr>")
                .arg(xValue()->title(mMainWindow->units()))
                .arg(xValue()->value(dp, mMainWindow->units()));
    }

    for (int i = 0; i < yaLast; ++i)
    {
        if (yValue(i)->visible())
        {
            status += QString("<tr style='color:%3;'><td>%1</td><td>%2</td></tr>")
                    .arg(yValue(i)->title(mMainWindow->units()))
                    .arg(yValue(i)->value(dp, mMainWindow->units()))
                    .arg(yValue(i)->color().name());
        }
    }

    status += QString("</table>");

    QToolTip::showText(QCursor::pos(), status);
}

void DataPlot::setRange(
        const QCPRange &range)
{
    DataPoint dpLower = interpolateDataX(range.lower);
    DataPoint dpUpper = interpolateDataX(range.upper);

    mMainWindow->setRange(dpLower.t, dpUpper.t);
}

void DataPlot::updateYRanges()
{
    const QCPRange &range = xAxis->range();

    int k = 0;
    for (int j = 0; j < yaLast; ++j)
    {
        if (!yValue(j)->visible()) continue;

        double yMin, yMax;
        bool first = true;

        for (int i = 0; i < mMainWindow->dataSize(); ++i)
        {
            const DataPoint &dp = mMainWindow->dataPoint(i);

            if (range.contains(xValue()->value(dp, mMainWindow->units())))
            {
                double y = yValue(j)->value(dp, mMainWindow->units());

                if (first)
                {
                    yMin = yMax = y;
                    first = false;
                }
                else
                {
                    if (y < yMin) yMin = y;
                    if (y > yMax) yMax = y;
                }
            }
        }

        if (yValue(j)->hasOptimal())
        {
            for (int i = 0; i < mMainWindow->optimalSize(); ++i)
            {
                const DataPoint &dp = mMainWindow->optimalPoint(i);

                if (range.contains(xValue()->value(dp, mMainWindow->units())))
                {
                    double y = yValue(j)->value(dp, mMainWindow->units());

                    if (first)
                    {
                        yMin = yMax = y;
                        first = false;
                    }
                    else
                    {
                        if (y < yMin) yMin = y;
                        if (y > yMax) yMax = y;
                    }
                }
            }
        }

        if (!first)
        {
            const double factor = yValue(j)->factor(mMainWindow->units());
            axisRect()->axis(QCPAxis::atLeft, k++)->setRange(
                        yValue(j)->useMinimum() ? yValue(j)->minimum() * factor : yMin,
                        yValue(j)->useMaximum() ? yValue(j)->maximum() * factor : yMax);
        }
    }

    replot();
}

void DataPlot::updatePlot()
{
    xAxis->setLabel(xValue()->title(mMainWindow->units()));

    QVector< double > x;
    for (int i = 0; i < mMainWindow->dataSize(); ++i)
    {
        const DataPoint &dp = mMainWindow->dataPoint(i);
        x.append(xValue()->value(dp, mMainWindow->units()));
    }

    clearPlottables();
    clearItems();

    // Remove all axes
    while (axisRect()->axisCount(QCPAxis::atLeft) > 0)
    {
        axisRect()->removeAxis(axisRect()->axis(QCPAxis::atLeft, 0));
    }

    // Add axes for visible plots
    for (int j = 0; j < yaLast; ++j)
    {
        if (!yValue(j)->visible()) continue;
        QCPAxis *axis = yValue(j)->addAxis(this, mMainWindow->units());
    }

    DataPoint dpLower = mMainWindow->interpolateDataT(mMainWindow->rangeLower());
    DataPoint dpUpper = mMainWindow->interpolateDataT(mMainWindow->rangeUpper());

    double xMin = xValue()->value(dpLower, mMainWindow->units());
    double xMax = xValue()->value(dpUpper, mMainWindow->units());

    // Add shading for scoring window
    if (yValue(Elevation)->visible() && mMainWindow->isWindowValid())
    {
        const DataPoint &dpTop = mMainWindow->windowTopDP();
        const DataPoint &dpBottom = mMainWindow->windowBottomDP();

        QVector< double > xElev, yElev;

        xElev << xMin << xMax;
        yElev << yValue(Elevation)->value(dpTop, mMainWindow->units())
              << yValue(Elevation)->value(dpTop, mMainWindow->units());

        QCPGraph *graph = addGraph(
                    axisRect()->axis(QCPAxis::atBottom),
                    yValue(Elevation)->axis());
        graph->setData(xElev, yElev);
        graph->setPen(QPen(QBrush(Qt::lightGray), mMainWindow->lineThickness(), Qt::DashLine));

        yElev.clear();
        yElev << yValue(Elevation)->value(dpBottom, mMainWindow->units())
              << yValue(Elevation)->value(dpBottom, mMainWindow->units());

        graph = addGraph(
                    axisRect()->axis(QCPAxis::atBottom),
                    yValue(Elevation)->axis());
        graph->setData(xElev, yElev);
        graph->setPen(QPen(QBrush(Qt::lightGray), mMainWindow->lineThickness(), Qt::DashLine));

        QCPItemRect *rect = new QCPItemRect(this);
        addItem(rect);

        rect->setPen(QPen(QBrush(Qt::lightGray), mMainWindow->lineThickness(), Qt::DashLine));
        rect->setBrush(QColor(0, 0, 0, 8));

        rect->topLeft->setType(QCPItemPosition::ptAxisRectRatio);
        rect->topLeft->setAxes(xAxis, yValue(Elevation)->axis());
        rect->topLeft->setCoords(-0.1, -0.1);

        rect->bottomRight->setType(QCPItemPosition::ptAxisRectRatio);
        rect->bottomRight->setAxes(xAxis, yValue(Elevation)->axis());
        rect->bottomRight->setCoords(
                    (xValue()->value(dpTop, mMainWindow->units()) - xMin) / (xMax - xMin),
                    1.1);

        rect = new QCPItemRect(this);
        addItem(rect);

        rect->setPen(QPen(QBrush(Qt::lightGray), mMainWindow->lineThickness(), Qt::DashLine));
        rect->setBrush(QColor(0, 0, 0, 8));

        rect->topLeft->setType(QCPItemPosition::ptAxisRectRatio);
        rect->topLeft->setAxes(xAxis, yValue(Elevation)->axis());
        rect->topLeft->setCoords(
                    (xValue()->value(dpBottom, mMainWindow->units()) - xMin) / (xMax - xMin),
                    -0.1);

        rect->bottomRight->setType(QCPItemPosition::ptAxisRectRatio);
        rect->bottomRight->setAxes(xAxis, yValue(Elevation)->axis());
        rect->bottomRight->setCoords(1.1, 1.1);
    }

    // Draw plots
    for (int j = 0; j < yaLast; ++j)
    {
        if (!yValue(j)->visible()) continue;

        QVector< double > y;
        for (int i = 0; i < mMainWindow->dataSize(); ++i)
        {
            const DataPoint &dp = mMainWindow->dataPoint(i);
            y.append(yValue(j)->value(dp, mMainWindow->units()));
        }

        QCPAxis *axis = yValue(j)->axis();
        QCPGraph *graph = addGraph(
                    axisRect()->axis(QCPAxis::atBottom),
                    axis);
        graph->setData(x, y);
        graph->setPen(QPen(yValue(j)->color(), mMainWindow->lineThickness()));

        if (yValue(j)->hasOptimal())
        {
            QVector< double > xOptimal, yOptimal;
            for (int i = 0; i < mMainWindow->optimalSize(); ++i)
            {
                const DataPoint &dp = mMainWindow->optimalPoint(i);
                xOptimal.append(xValue()->value(dp, mMainWindow->units()));
                yOptimal.append(yValue(j)->value(dp, mMainWindow->units()));
            }

            QCPGraph *graph = addGraph(
                        axisRect()->axis(QCPAxis::atBottom),
                        axis);
            graph->setData(xOptimal, yOptimal);
            graph->setPen(QPen(QBrush(yValue(j)->color()), mMainWindow->lineThickness(), Qt::DotLine));
        }
    }

    // Draw mark
    if (mMainWindow->markActive())
    {
        const DataPoint &dpEnd = mMainWindow->interpolateDataT(mMainWindow->markEnd());

        QVector< double > xMark, yMark;

        xMark.append(xValue()->value(dpEnd, mMainWindow->units()));

        int k = 0;
        for (int j = 0; j < yaLast; ++j)
        {
            if (!yValue(j)->visible()) continue;

            yMark.clear();
            yMark.append(yValue(j)->value(dpEnd, mMainWindow->units()));

            QCPGraph *graph = addGraph(
                        xAxis,
                        axisRect()->axis(QCPAxis::atLeft, k++));

            graph->setData(xMark, yMark);
            graph->setPen(QPen(Qt::black, mMainWindow->lineThickness()));
            graph->setLineStyle(QCPGraph::lsNone);
            graph->setScatterStyle(QCPScatterStyle::ssDisc);
        }
    }

    // Set x-axis range
    if (mMainWindow->dataSize() > 0)
    {
        xAxis->setRange(QCPRange(xMin, xMax));
    }

    if (mMainWindow->windAdjustment())
    {
        // Add label to indicate wind correction
        QCPItemText *textLabel = new QCPItemText(this);
        addItem(textLabel);

        textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignRight);
        textLabel->setTextAlignment(Qt::AlignRight);
        textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
        textLabel->position->setCoords(1, 0);
        textLabel->setBrush(QBrush(Qt::red));
        textLabel->setColor(Qt::white);
        textLabel->setText(tr("Results are adjusted for wind"));
        textLabel->setFont(QFont(font().family(), font().pointSize(), QFont::Black));
        textLabel->setPadding(QMargins(2, 2, 2, 2));
    }

    updateYRanges();
}

DataPoint DataPlot::interpolateDataX(
        double x)
{
    const int i1 = findIndexBelowX(x);
    const int i2 = findIndexAboveX(x);

    if (i1 < 0)
    {
        return mMainWindow->dataPoint(0);
    }
    else if (i2 >= mMainWindow->dataSize())
    {
        return mMainWindow->dataPoint(mMainWindow->dataSize() - 1);
    }
    else
    {
        const DataPoint &dp1 = mMainWindow->dataPoint(i1);
        const DataPoint &dp2 = mMainWindow->dataPoint(i2);
        const double x1 = xValue()->value(dp1, mMainWindow->units());
        const double x2 = xValue()->value(dp2, mMainWindow->units());
        return DataPoint::interpolate(dp1, dp2, (x - x1) / (x2 - x1));
    }
}

int DataPlot::findIndexBelowX(
        double x)
{
    int below = -1;
    int above = mMainWindow->dataSize();

    while (below + 1 != above)
    {
        int mid = (below + above) / 2;
        const DataPoint &dp = mMainWindow->dataPoint(mid);

        if (xValue()->value(dp, mMainWindow->units()) < x) below = mid;
        else                                               above = mid;
    }

    return below;
}

int DataPlot::findIndexAboveX(
        double x)
{
    int below = -1;
    int above = mMainWindow->dataSize();

    while (below + 1 != above)
    {
        int mid = (below + above) / 2;
        const DataPoint &dp = mMainWindow->dataPoint(mid);

        if (xValue()->value(dp, mMainWindow->units()) > x) above = mid;
        else                                               below = mid;
    }

    return above;
}

void DataPlot::togglePlot(
        YAxisType plot)
{
    m_yValues[plot]->setVisible(!m_yValues[plot]->visible());
    updatePlot();
}

void DataPlot::setXAxisType(
        XAxisType xAxisType)
{
    const QCPRange &range = xAxis->range();

    DataPoint dpLower = interpolateDataX(range.lower);
    DataPoint dpUpper = interpolateDataX(range.upper);

    m_xAxisType = xAxisType;

    xAxis->setRange(QCPRange(xValue()->value(dpLower, mMainWindow->units()),
                             xValue()->value(dpUpper, mMainWindow->units())));

    updatePlot();
}
