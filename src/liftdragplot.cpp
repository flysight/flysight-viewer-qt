#include <QToolTip>
#include <QVector2D>

#include "common.h"
#include "liftdragplot.h"
#include "mainwindow.h"

LiftDragPlot::LiftDragPlot(QWidget *parent) :
    QCustomPlot(parent),
    mMainWindow(0),
    mDragging(false)
{

}

QSize LiftDragPlot::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void LiftDragPlot::mousePressEvent(
        QMouseEvent *event)
{
    if (axisRect()->rect().contains(event->pos()))
    {
        mBeginPos = event->pos();
    }

    QCustomPlot::mousePressEvent(event);
}

void LiftDragPlot::mouseReleaseEvent(
        QMouseEvent *event)
{
    if (!mDragging)
    {
        mMainWindow->setMaxLift(xAxis->pixelToCoord(mBeginPos.x()));
        mMainWindow->clearMark();
        QToolTip::hideText();
    }
    else
    {
        mDragging = false;
    }

    QCustomPlot::mouseReleaseEvent(event);
}

void LiftDragPlot::mouseMoveEvent(
        QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        QPoint pos = event->pos();
        QVector2D diff = QVector2D(pos - mBeginPos);

        if (diff.length() > selectionTolerance())
        {
            mDragging = true;
        }

        if (mDragging)
        {
            const double cl = xAxis->pixelToCoord(pos.x());
            const double cd = yAxis->pixelToCoord(pos.y());

            const double c = cd / 2;
            const double a = c / cl / cl;

            const double bb = mMainWindow->wingSpan();
            const double ss = mMainWindow->planformArea();
            const double ar = bb * bb / ss;

            mMainWindow->setMinDrag(c);
            mMainWindow->setEfficiency(1 / (M_PI * a * ar));
        }
    }
    else if (QCPCurve *graph = qobject_cast<QCPCurve *>(plottable(0)))
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

    xAxis->setLabel(tr("Lift Coefficient"));
    yAxis->setLabel(tr("Drag Coefficient"));

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
            xMax = x.back();
            yMax = y.back();

            first = false;
        }
        else
        {
            if (x.back() > xMax) xMax = x.back();
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

    setViewRange(xMax, yMax);

    // Update plot limits
    xMin = xAxis->range().lower;
    xMax = xAxis->range().upper;

    yMin = yAxis->range().lower;
    yMax = yAxis->range().upper;

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

    const double bb = mMainWindow->wingSpan();
    const double ss = mMainWindow->planformArea();
    const double ar = bb * bb / ss;

    const double a = 1 / (M_PI * mMainWindow->efficiency() * ar);
    const double c = mMainWindow->minDrag();

    // Draw tangent line
    const double xt = sqrt(c / a);
    const double m = 2 * a * xt;

    if (a != 0)
    {
        x.clear();
        y.clear();

        x << xMin << xMax;
        y << m * xMin << m * xMax;

        QCPGraph *graph = addGraph();
        graph->setData(x, y);
        graph->setPen(QPen(Qt::blue, 0, Qt::DashLine));
    }

    // Draw minimum drag
    x.clear();
    y.clear();

    x << xMin << xMax;
    y << mMainWindow->minDrag() << mMainWindow->minDrag();

    QCPGraph *graph = addGraph();
    graph->setData(x, y);
    graph->setPen(QPen(Qt::blue, 0, Qt::DashLine));

    // Draw maximum lift
    x.clear();
    y.clear();

    x << mMainWindow->maxLift() << mMainWindow->maxLift();
    y << yMin << yMax;

    graph = addGraph();
    graph->setData(x, y);
    graph->setPen(QPen(Qt::blue, 0, Qt::DashLine));

    // Draw saved curve
    t.clear();
    x.clear();
    y.clear();

    for (int i = 0; i <= 100; ++i)
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

    // Draw dot at maximum L/D
    if (a != 0)
    {
        x.clear();
        y.clear();

        x << xt;
        y << a * xt * xt + c;

        QCPGraph *graph = addGraph();
        graph->setData(x, y);
        graph->setPen(QPen(Qt::red));
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle::ssDisc);
    }

    // Add label to show equation for saved curve
    QCPItemText *textLabel = new QCPItemText(this);
    addItem(textLabel);

    textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
    textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    textLabel->position->setCoords(0.5, 0);
    textLabel->setText(
                QString("Minimum drag = %1\nMaximum lift = %2\nMaximum L/D = %3")
                    .arg(fabs(c))
                    .arg(mMainWindow->maxLift())
                    .arg(1 / m));

    replot();
}

void LiftDragPlot::setViewRange(
        double xMax,
        double yMax)
{
    xAxis->setRange(0, xMax * 1.1);
    yAxis->setRange(0, yMax * 1.1);
}
