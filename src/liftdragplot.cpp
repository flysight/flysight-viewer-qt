#include <QToolTip>
#include <QVector2D>

#include "common.h"
#include "liftdragplot.h"
#include "mainwindow.h"

LiftDragPlot::LiftDragPlot(QWidget *parent) :
    QCustomPlot(parent),
    mMainWindow(0)
{

}

QSize LiftDragPlot::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void LiftDragPlot::mouseMoveEvent(
        QMouseEvent *event)
{
    if (QCPCurve *graph = qobject_cast<QCPCurve *>(plottable(0)))
    {
        const QCPCurveDataMap *data = graph->data();

        double resultTime;
        double resultDistance = std::numeric_limits<double>::max();

        for (QCPCurveDataMap::const_iterator it = data->constBegin();
             it != data->constEnd();
             ++it)
        {
            QVector2D pt = QVector2D(xAxis->coordToPixel(it.value().key),
                                     yAxis->coordToPixel(it.value().value));

            double dist = pt.distanceToPoint(QVector2D(event->pos()));

            if (dist < resultDistance)
            {
                resultTime = it.value().t;
                resultDistance = dist;
            }
        }

        if (resultDistance < selectionTolerance())
        {
            setMark(resultTime);
        }
        else
        {
            mMainWindow->clearMark();
            QToolTip::hideText();
        }
    }
}

void LiftDragPlot::setMark(
        double mark)
{
    if (mMainWindow->dataSize() == 0) return;

    DataPoint dp = mMainWindow->interpolateDataT(mark);
    mMainWindow->setMark(mark);

    QString status;
    status = QString("<table width='200'>");

    status += QString("<tr style='color:%3;'><td>%1</td><td>%2</td></tr>")
            .arg(PlotLift().title(mMainWindow->units()))
            .arg(PlotLift().value(dp, mMainWindow->units()))
            .arg(PlotLift().color().name());

    status += QString("<tr style='color:%3;'><td>%1</td><td>%2</td></tr>")
            .arg(PlotDrag().title(mMainWindow->units()))
            .arg(PlotDrag().value(dp, mMainWindow->units()))
            .arg(PlotDrag().color().name());

    status += QString("</table>");

    QToolTip::showText(QCursor::pos(), status);
}

void LiftDragPlot::updatePlot()
{
    clearPlottables();
    clearItems();

    double lower = mMainWindow->rangeLower();
    double upper = mMainWindow->rangeUpper();

    QVector< double > t, x, y;

    double xMin, xMax;
    double yMin, yMax;

    int start = mMainWindow->findIndexBelowT(lower) + 1;
    int end   = mMainWindow->findIndexAboveT(upper);

    double s10 = 0, s01 = 0, s20 = 0, s11 = 0;
    double s21 = 0, s30 = 0, s40 = 0;

    bool first = true;
    for (int i = start; i < end; ++i)
    {
        const DataPoint &dp = mMainWindow->dataPoint(i);

        t.append(dp.t);
        x.append(dp.lift);
        y.append(dp.drag);

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

        s10 += dp.lift;
        s01 += dp.drag;
        s20 += dp.lift * dp.lift;
        s11 += dp.lift * dp.drag;
        s21 += dp.lift * dp.lift * dp.drag;
        s30 += dp.lift * dp.lift * dp.lift;
        s40 += dp.lift * dp.lift * dp.lift * dp.lift;
    }

    QCPCurve *curve = new QCPCurve(xAxis, yAxis);
    curve->setData(t, x, y);
    curve->setPen(QPen(Qt::lightGray));
    curve->setLineStyle(QCPCurve::lsNone);
    curve->setScatterStyle(QCPScatterStyle::ssDisc);
    addPlottable(curve);

    setViewRange(xMin, xMax, yMin, yMax);

    const double s00 = end - start + 1;
    const double det = s00 * s40 - s20 * s20;

    if (det != 0)
    {
        // y = ax^2 + c
        const double a = (-s20 * s01 + s00 * s21) / det;
        const double c = ( s40 * s01 - s20 * s21) / det;

        t.clear();
        x.clear();
        y.clear();

        xMin = xAxis->range().lower;
        xMax = xAxis->range().upper;

        // Add best fit curve
        for (int i = 0; i < 101; ++i)
        {
            const double xx = xMin + (xMax - xMin) / 100 * i;

            t.append(xx);
            x.append(xx);
            y.append(a * xx * xx + c);
        }

        curve = new QCPCurve(xAxis, yAxis);
        curve->setData(t, x, y);
        curve->setPen(QPen(Qt::red));
        addPlottable(curve);

        // Add label to show equation for fit
        QCPItemText *textLabel = new QCPItemText(this);
        addItem(textLabel);

        textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
        textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
        textLabel->position->setCoords(0.5, 0);
        textLabel->setText(QString("y = %1 x² %2 %3").arg(a).arg((c < 0) ? '-' : '+').arg(fabs(c)));

        if (a != 0)
        {
            t.clear();
            x.clear();
            y.clear();

            // Find tangent line
            const double xt = sqrt(c / a);
            const double m = 2 * a * xt;

            // Add tangent line
            for (int i = 0; i < 101; ++i)
            {
                const double xx = xMin + (xMax - xMin) / 100 * i;

                t.append(xx);
                x.append(xx);
                y.append(m * xx);
            }

            curve = new QCPCurve(xAxis, yAxis);
            curve->setData(t, x, y);
            curve->setPen(QPen(Qt::blue, 0, Qt::DashLine));
            addPlottable(curve);

            x.clear();
            y.clear();

            x.append(xt);
            y.append(a * xt * xt + c);

            QCPGraph *graph = addGraph();
            graph->setData(x, y);
            graph->setPen(QPen(Qt::blue));
            graph->setLineStyle(QCPGraph::lsNone);
            graph->setScatterStyle(QCPScatterStyle::ssDisc);
        }
    }

    if (mMainWindow->markActive())
    {
        int i1 = mMainWindow->findIndexBelowT(mMainWindow->markEnd()) + 1;
        int i2 = mMainWindow->findIndexAboveT(mMainWindow->markEnd()) - 1;

        const DataPoint &dp1 = mMainWindow->dataPoint(i1);
        const DataPoint &dp2 = mMainWindow->dataPoint(i2);

        QVector< double > xMark, yMark;

        if (mMainWindow->markEnd() - dp1.t < dp2.t - mMainWindow->markEnd())
        {
            xMark.append(dp1.lift);
            yMark.append(dp1.drag);
        }
        else
        {
            xMark.append(dp2.lift);
            yMark.append(dp2.drag);
        }

        QCPGraph *graph = addGraph();
        graph->setData(xMark, yMark);
        graph->setPen(QPen(Qt::black));
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle::ssDisc);
    }

    replot();
}

void LiftDragPlot::setViewRange(
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
