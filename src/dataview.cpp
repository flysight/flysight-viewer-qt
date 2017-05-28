#include "dataview.h"

#include "common.h"
#include "mainwindow.h"

#define WINDOW_MARGIN 1.2

DataView::DataView(QWidget *parent) :
    QCustomPlot(parent),
    mMainWindow(0),
    m_topViewPan(false)
{
    setMouseTracking(true);
}

QSize DataView::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void DataView::mousePressEvent(
        QMouseEvent *event)
{
    if (mDirection == Top && axisRect()->rect().contains(event->pos()))
    {
        m_topViewBeginPos = event->pos() - axisRect()->center();
        m_topViewPan = true;
    }

    QCustomPlot::mousePressEvent(event);
}

void DataView::mouseReleaseEvent(
        QMouseEvent *event)
{
    m_topViewPan = false;
    QCustomPlot::mouseReleaseEvent(event);
}

void DataView::mouseMoveEvent(
        QMouseEvent *event)
{
    if (m_topViewPan)
    {
        QRect rect = axisRect()->rect();
        QPoint endPos = event->pos() - rect.center();

        double r = (double) qMin(rect.width(), rect.height()) / 2 / WINDOW_MARGIN;

        double a1 = (double) (m_topViewBeginPos.x() - rect.left()) / r;
        double a2 = (double) (endPos.x() - rect.left()) / r;
        double a = mMainWindow->rotation() - (a2 - a1);

        while (a < -PI) a += 2 * PI;
        while (a >  PI) a -= 2 * PI;

        mMainWindow->setRotation(a);

        m_topViewBeginPos = endPos;
    }

    if (QCPCurve *curve = qobject_cast<QCPCurve *>(plottable(0)))
    {
        const QCPCurveDataMap *data = curve->data();

        double resultTime;
        double resultDistance = std::numeric_limits<double>::max();

        for (QCPCurveDataMap::const_iterator it = data->constBegin();
             it != data->constEnd() && (it + 1) != data->constEnd();
             ++ it)
        {
            QPointF pt1 = QPointF(xAxis->coordToPixel(it.value().key),
                                  yAxis->coordToPixel(it.value().value));
            QPointF pt2 = QPointF(xAxis->coordToPixel((it + 1).value().key),
                                  yAxis->coordToPixel((it + 1).value().value));

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

        if (resultDistance < selectionTolerance())
        {
            mMainWindow->setMark(resultTime);
        }
        else
        {
            mMainWindow->clearMark();
        }
    }
}

void DataView::updateView()
{
    clearPlottables();

    m_cursors.clear();

    // Return now if plot empty
    if (mMainWindow->dataSize() == 0) return;

    // Get plot range
    double lower = mMainWindow->rangeLower();
    double upper = mMainWindow->rangeUpper();

    QVector< double > t, x, y, z;

    double xMin, xMax;
    double yMin, yMax;
    double zMin, zMax;

    double uMin, uMax;
    double vMin, vMax;

    bool first = true;

    for (int i = 0; i < mMainWindow->dataSize(); ++i)
    {
        const DataPoint &dp = mMainWindow->dataPoint(i);

        if (lower <= dp.t && dp.t <= upper)
        {
            t.append(dp.t);

            if (mMainWindow->units() == PlotValue::Metric)
            {
                x.append(dp.x *  cos(mMainWindow->rotation()) + dp.y * sin(mMainWindow->rotation()));
                y.append(dp.x * -sin(mMainWindow->rotation()) + dp.y * cos(mMainWindow->rotation()));
                z.append(dp.z);
            }
            else
            {
                x.append((dp.x *  cos(mMainWindow->rotation()) + dp.y * sin(mMainWindow->rotation())) * METERS_TO_FEET);
                y.append((dp.x * -sin(mMainWindow->rotation()) + dp.y * cos(mMainWindow->rotation())) * METERS_TO_FEET);
                z.append((dp.z) * METERS_TO_FEET);
            }

            if (first)
            {
                xMin = xMax = x.back();
                yMin = yMax = y.back();
                zMin = zMax = z.back();

                uMin = uMax = dp.x;
                vMin = vMax = dp.y;

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

                if (dp.x < uMin) uMin = dp.x;
                if (dp.x > uMax) uMax = dp.x;

                if (dp.y < vMin) vMin = dp.y;
                if (dp.y > vMax) vMax = dp.y;
            }
        }
    }

    QCPCurve *curve = new QCPCurve(xAxis, yAxis);
    switch (mDirection)
    {
    case Top:
        curve->setData(t, x, y);
        curve->setPen(QPen(Qt::black, mMainWindow->lineThickness()));
        break;
    case Left:
        curve->setData(t, x, z);
        curve->setPen(QPen(Qt::blue, mMainWindow->lineThickness()));
        break;
    case Front:
        curve->setData(t, y, z);
        curve->setPen(QPen(Qt::red, mMainWindow->lineThickness()));
        break;
    }

    addPlottable(curve);

    double uMid = (uMin + uMax) / 2;
    double vMid = (vMin + vMax) / 2;

    double xMid = uMid *  cos(mMainWindow->rotation()) + vMid * sin(mMainWindow->rotation());
    double yMid = uMid * -sin(mMainWindow->rotation()) + vMid * cos(mMainWindow->rotation());

    double rMax = 0;
    for (int i = 0; i < x.size(); ++i)
    {
        const double dx = x[i] - xMid;
        const double dy = y[i] - yMid;
        const double r = dx * dx + dy * dy;
        if (r > rMax) rMax = r;
    }
    rMax = sqrt(rMax);

    switch (mDirection)
    {
    case Top:
        setViewRange(xMid - rMax, xMid + rMax,
                     yMid - rMax, yMid + rMax);
        break;
    case Left:
        setViewRange(xMid - rMax, xMid + rMax,
                     zMin, zMax);
        break;
    case Front:
        setViewRange(yMid - rMax, yMid + rMax,
                     zMin, zMax);
        break;
    }

    if (mDirection == Top)
    {
        QCPGraph *graph = addGraph();
        graph->addData(xAxis->range().upper, (yMin + yMax) / 2);
        graph->setPen(QPen(Qt::red, mMainWindow->lineThickness()));
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 12));

        graph = addGraph();
        graph->addData((xMin + xMax) / 2, yAxis->range().lower);
        graph->setPen(QPen(Qt::blue, mMainWindow->lineThickness()));
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 12));
    }

    if (mMainWindow->dataSize() > 0)
    {
        QVector< double > xMark, yMark, zMark;

        for (int i = 0; i < mMainWindow->waypointSize(); ++i)
        {
            const DataPoint &dp0 = mMainWindow->interpolateDataT(0);
            DataPoint dp = mMainWindow->waypoint(i);

            double distance = mMainWindow->getDistance(dp0, dp);
            double bearing = mMainWindow->getBearing(dp0, dp);

            dp.x = distance * sin(bearing);
            dp.y = distance * cos(bearing);

            if (mMainWindow->units() == PlotValue::Metric)
            {
                xMark.append(dp.x *  cos(mMainWindow->rotation()) + dp.y * sin(mMainWindow->rotation()));
                yMark.append(dp.x * -sin(mMainWindow->rotation()) + dp.y * cos(mMainWindow->rotation()));
                zMark.append(dp.z);
            }
            else
            {
                xMark.append((dp.x *  cos(mMainWindow->rotation()) + dp.y * sin(mMainWindow->rotation())) * METERS_TO_FEET);
                yMark.append((dp.x * -sin(mMainWindow->rotation()) + dp.y * cos(mMainWindow->rotation())) * METERS_TO_FEET);
                zMark.append((dp.z) * METERS_TO_FEET);
            }
        }

        QCPGraph *graph = addGraph();
        switch (mDirection)
        {
        case Top:
            graph->setData(xMark, yMark);
            break;
        case Left:
            graph->setData(xMark, zMark);
            break;
        case Front:
            graph->setData(yMark, zMark);
            break;
        }
        graph->setPen(QPen(Qt::black, mMainWindow->lineThickness()));
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle::ssPlus);
    }

    if (mDirection == Top)
    {
        addNorthArrow();
    }

    updateCursor();
}

void DataView::updateCursor()
{
    for (int i = 0; i < m_cursors.size(); ++i)
    {
        removeGraph(m_cursors[i]);
    }

    m_cursors.clear();

    if (mMainWindow->markActive())
    {
        const DataPoint &dpEnd = mMainWindow->interpolateDataT(mMainWindow->markEnd());

        QVector< double > xMark, yMark, zMark;

        if (mMainWindow->units() == PlotValue::Metric)
        {
            xMark.append(dpEnd.x *  cos(mMainWindow->rotation()) + dpEnd.y * sin(mMainWindow->rotation()));
            yMark.append(dpEnd.x * -sin(mMainWindow->rotation()) + dpEnd.y * cos(mMainWindow->rotation()));
            zMark.append(dpEnd.z);
        }
        else
        {
            xMark.append((dpEnd.x *  cos(mMainWindow->rotation()) + dpEnd.y * sin(mMainWindow->rotation())) * METERS_TO_FEET);
            yMark.append((dpEnd.x * -sin(mMainWindow->rotation()) + dpEnd.y * cos(mMainWindow->rotation())) * METERS_TO_FEET);
            zMark.append((dpEnd.z) * METERS_TO_FEET);
        }

        QCPGraph *graph = addGraph();
        switch (mDirection)
        {
        case Top:
            graph->setData(xMark, yMark);
            break;
        case Left:
            graph->setData(xMark, zMark);
            break;
        case Front:
            graph->setData(yMark, zMark);
            break;
        }
        graph->setPen(QPen(Qt::black, mMainWindow->lineThickness()));
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle::ssDisc);

        m_cursors.push_back(graph);
    }

    replot();
}

void DataView::addNorthArrow()
{
    QPainter painter(this);

    double mmPerPix = (double) painter.device()->widthMM() / painter.device()->width();
    double valPerPix = xAxis->range().size() / axisRect()->width();
    double valPerMM = valPerPix / mmPerPix;

    // rotated arrow
    QPointF north( 5 * sin(mMainWindow->rotation()),  5 * cos(mMainWindow->rotation()));
    QPointF south(-5 * sin(mMainWindow->rotation()), -5 * cos(mMainWindow->rotation()));

    // offset from corner
    north -= QPointF(10, 10);
    south -= QPointF(10, 10);

    // convert from mm to values
    north *= valPerMM;
    south *= valPerMM;

    QPointF corner (xAxis->range().upper, yAxis->range().upper);

    north += corner;
    south += corner;

    removeItem(0);

    QCPItemLine *arrow = new QCPItemLine(this);
    addItem(arrow);
    arrow->start->setCoords(south);
    arrow->end->setCoords(north);
    arrow->setHead(QCPLineEnding::esSpikeArrow);
}

void DataView::setViewRange(
        double xMin,
        double xMax,
        double yMin,
        double yMax)
{
    QPainter painter(this);

    double xMMperPix = (double) painter.device()->widthMM() / painter.device()->width();
    double yMMperPix = (double) painter.device()->heightMM() / painter.device()->height();

    QRect rect = axisRect()->rect();

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

    xAxis->setRange(xMin, xMax);
    yAxis->setRange(yMin, yMax);
}
