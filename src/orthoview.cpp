#include "orthoview.h"

#include "common.h"
#include "mainwindow.h"

OrthoView::OrthoView(QWidget *parent) :
    QCustomPlot(parent),
    mMainWindow(0),
    m_pan(false)
{
    setMouseTracking(true);
}

QSize OrthoView::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void OrthoView::mousePressEvent(
        QMouseEvent *event)
{
    if (axisRect()->rect().contains(event->pos()))
    {
        m_beginPos = event->pos() - axisRect()->center();
        m_pan = true;
    }

    QCustomPlot::mousePressEvent(event);
}

void OrthoView::mouseReleaseEvent(
        QMouseEvent *event)
{
    m_pan = false;
    QCustomPlot::mouseReleaseEvent(event);
}

void OrthoView::mouseMoveEvent(
        QMouseEvent *event)
{
    if (m_pan)
    {
        const double pi = 3.14159265359;

        QRect rect = axisRect()->rect();
        QPoint endPos = event->pos() - rect.center();

        double a1 = (double) (m_beginPos.x() - rect.left()) / rect.width();
        double a2 = (double) (endPos.x() - rect.left()) / rect.width();
        double a = a2 - a1;

        while (a < -pi) a += 2 * pi;
        while (a >  pi) a -= 2 * pi;

        mMainWindow->setRotation(mMainWindow->rotation() - a);

        m_beginPos = endPos;
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

void OrthoView::updateView()
{
    double lower = mMainWindow->rangeLower();
    double upper = mMainWindow->rangeUpper();

    QVector< double > t, x, y, z;

    double xMin, xMax;
    double yMin, yMax;
    double zMin, zMax;

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

    clearPlottables();

    QCPCurve *curve = new QCPCurve(xAxis, yAxis);
    curve->setData(t, x, y);
    curve->setPen(QPen(Qt::black));
    addPlottable(curve);

    setViewRange(xMin, xMax, yMin, yMax);

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
        graph->setData(xMark, yMark);
        graph->setPen(QPen(Qt::black));
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle::ssDisc);
    }

    if (mMainWindow->dataSize() > 0)
    {
        QVector< double > xMark, yMark, zMark;

        for (int i = 0; i < mMainWindow->waypointSize(); ++i)
        {
            const DataPoint &dp0 = mMainWindow->dataPoint(
                        mMainWindow->dataSize() - 1);
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
        graph->setData(xMark, yMark);
        graph->setPen(QPen(Qt::black));
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle::ssPlus);
    }

    replot();
}

void OrthoView::setViewRange(
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
