#include <QGridLayout>
#include <QPushButton>
#include <QToolTip>

#include "common.h"
#include "windplot.h"
#include "mainwindow.h"

WindPlot::WindPlot(QWidget *parent) :
    QCustomPlot(parent),
    mMainWindow(0)
{
    QGridLayout *layout = new QGridLayout;
    setLayout(layout);

    QPushButton *save = new QPushButton(tr("Save"));
    layout->addWidget(save, 0, 0, Qt::AlignRight | Qt::AlignTop);

    connect(save, SIGNAL(clicked()),
            this, SLOT(save()));
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
    clearPlottables();
    clearItems();

    // Return now if plot empty
    if (mMainWindow->dataSize() == 0) return;

    double lower = mMainWindow->rangeLower();
    double upper = mMainWindow->rangeUpper();

    QVector< double > t, x, y;

    double xMin, xMax;
    double yMin, yMax;

    int start = mMainWindow->findIndexBelowT(lower) + 1;
    int end   = mMainWindow->findIndexAboveT(upper);

    bool first = true;
    for (int i = start; i < end; ++i)
    {
        const DataPoint &dp = mMainWindow->dataPoint(i);

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

    QCPCurve *curve = new QCPCurve(xAxis, yAxis);
    curve->setData(t, x, y);
    curve->setPen(QPen(Qt::lightGray, mMainWindow->lineThickness()));
    addPlottable(curve);

    setViewRange(xMin, xMax, yMin, yMax);

    if (mMainWindow->markActive())
    {
        const DataPoint &dpEnd = mMainWindow->interpolateDataT(mMainWindow->markEnd());

        t.clear();
        x.clear();
        y.clear();

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
        graph->setPen(QPen(Qt::black, mMainWindow->lineThickness()));
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle::ssDisc);
    }

    updateWind(start, end);

    QVector< double > xMark, yMark;

    if (mMainWindow->units() == PlotValue::Metric)
    {
        xMark.append(mWindE * MPS_TO_KMH);
        yMark.append(mWindN * MPS_TO_KMH);
    }
    else
    {
        xMark.append(mWindE * MPS_TO_MPH);
        yMark.append(mWindN * MPS_TO_MPH);
    }

    QCPGraph *graph = addGraph();
    graph->setData(xMark, yMark);
    graph->setPen(QPen(Qt::red, mMainWindow->lineThickness()));
    graph->setLineStyle(QCPGraph::lsNone);
    graph->setScatterStyle(QCPScatterStyle::ssDisc);

    const double x0 = mWindE;
    const double y0 = mWindN;
    const double r = mVelAircraft;

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
    curve->setPen(QPen(Qt::red, mMainWindow->lineThickness()));
    addPlottable(curve);

    // Add label to show best fit
    QCPItemText *textLabel = new QCPItemText(this);
    addItem(textLabel);

    const double factor = (mMainWindow->units() == PlotValue::Metric) ? MPS_TO_KMH : MPS_TO_MPH;
    const QString units = (mMainWindow->units() == PlotValue::Metric) ? "km/h" : "mph";

    double direction = atan2(-mWindE, -mWindN) / M_PI * 180.0;
    if (direction < 0) direction += 360.0;

    QPainter painter(this);
    double mmPerPix = (double) painter.device()->widthMM() / painter.device()->width();

    double xRatioPerPix = 1.0 / axisRect()->width();
    double xRatioPerMM = xRatioPerPix / mmPerPix;

    double yRatioPerPix = 1.0 / axisRect()->height();
    double yRatioPerMM = yRatioPerPix / mmPerPix;

    textLabel->setPositionAlignment(Qt::AlignBottom|Qt::AlignRight);
    textLabel->setTextAlignment(Qt::AlignRight);
    textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    textLabel->position->setCoords(1 - 5 * xRatioPerMM,
                                   1 - 5 * yRatioPerMM);
    textLabel->setText(
                QString("Wind speed = %1 %2\nWind direction = %3 deg\nAircraft speed = %4 %5")
                    .arg(sqrt(mWindE * mWindE + mWindN * mWindN) * factor)
                    .arg(units)
                    .arg(direction)
                    .arg(mVelAircraft * factor)
                    .arg(units));

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

void WindPlot::updateWind(
        const int start,
        const int end)
{
    // Weighted least-squares circle fit based on this:
    //   http://www.dtcenter.org/met/users/docs/write_ups/circle_fit.pdf

    double xbar = 0, ybar = 0, N = 0;
    for (int i = start; i < end; ++i)
    {
        const DataPoint &dp = mMainWindow->dataPoint(i);

        const double wi = 1.0;

        const double xi = dp.velE;
        const double yi = dp.velN;

        xbar += wi * xi;
        ybar += wi * yi;

        N += wi;
    }

    xbar /= N;
    ybar /= N;

    double suu = 0, suv = 0, svv = 0;
    double suuu = 0, suvv = 0, svuu = 0, svvv = 0;
    for (int i = start; i < end; ++i)
    {
        const DataPoint &dp = mMainWindow->dataPoint(i);

        const double wi = 1.0;

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
        mWindE = 0;
        mWindN = 0;
        mVelAircraft = 0;
        return;
    }

    const double uc = 1 / det * (0.5 * svv * (suuu + suvv) - 0.5 * suv * (svvv + svuu));
    const double vc = 1 / det * (0.5 * suu * (svvv + svuu) - 0.5 * suv * (suuu + suvv));

    const double xc = uc + xbar;
    const double yc = vc + ybar;

    const double alpha = uc * uc + vc * vc + (suu + svv) / N;
    const double R = sqrt(alpha);

    mWindE = xc;
    mWindN = yc;
    mVelAircraft = R;
}

void WindPlot::save()
{
    mMainWindow->setWind(mWindE, mWindN);
}
