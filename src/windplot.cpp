#include <QToolTip>

#include "common.h"
#include "windplot.h"
#include "mainwindow.h"

WindPlot::WindPlot(QWidget *parent) :
    QCustomPlot(parent),
    mMainWindow(0)
{

}

QSize WindPlot::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void WindPlot::mouseMoveEvent(
        QMouseEvent *event)
{
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

void WindPlot::updatePlot()
{
    double lower = mMainWindow->rangeLower();
    double upper = mMainWindow->rangeUpper();

    QVector< double > t, x, y;

    double xMin, xMax;
    double yMin, yMax;

    bool first = true;

    for (int i = 0; i < mMainWindow->dataSize(); ++i)
    {
        const DataPoint &dp = mMainWindow->dataPoint(i);

        if (lower <= dp.t && dp.t <= upper)
        {
            t.append(dp.t);

            if (mMainWindow->units() == PlotValue::Metric)
            {
                x.append(dp.velE * MPS_TO_KMH);
                y.append(dp.velN * MPS_TO_KMH);
            }
            else
            {
                x.append(dp.velE * MPS_TO_MPH);
                y.append(dp.velN * MPS_TO_MPH);
            }

            if (first)
            {
                xMin = xMax = x.back();
                yMin = yMax = y.back();

                first = false;
            }
            else
            {
                if (x.back() < xMin) xMin = x.back();
                if (x.back() > xMax) xMax = x.back();

                if (y.back() < yMin) yMin = y.back();
                if (y.back() > yMax) yMax = y.back();
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

        QVector< double > xMark, yMark;

        if (mMainWindow->units() == PlotValue::Metric)
        {
            xMark.append(dpEnd.velE * MPS_TO_KMH);
            yMark.append(dpEnd.velN * MPS_TO_KMH);
        }
        else
        {
            xMark.append(dpEnd.velE * MPS_TO_MPH);
            yMark.append(dpEnd.velN * MPS_TO_MPH);
        }

        QCPGraph *graph = addGraph();
        graph->setData(xMark, yMark);
        graph->setPen(QPen(Qt::black));
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle::ssDisc);

        const double x0 = dpEnd.windE;
        const double y0 = dpEnd.windN;
        const double r = dpEnd.velAircraft;

        QVector< double > tCircle, xCircle, yCircle;

        for (int i = 0; i <= 100; ++i)
        {
            tCircle.append(i);

            const double x = x0 + r * cos((double) i / 100 * 2 * M_PI);
            const double y = y0 + r * sin((double) i / 100 * 2 * M_PI);

            if (mMainWindow->units() == PlotValue::Metric)
            {
                xCircle.append(x * MPS_TO_KMH);
                yCircle.append(y * MPS_TO_KMH);
            }
            else
            {
                xCircle.append(x * MPS_TO_MPH);
                yCircle.append(y * MPS_TO_MPH);
            }
        }

        curve = new QCPCurve(xAxis, yAxis);
        curve->setData(tCircle, xCircle, yCircle);
        curve->setPen(QPen(Qt::red));
        addPlottable(curve);
    }

    replot();
}

void WindPlot::setViewRange(
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
