#include "dataview.h"
#include "mainwindow.h"

DataView::DataView(QWidget *parent) :
    QCustomPlot(parent),
    mMainWindow(0)
{

}

QSize DataView::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void DataView::mousePressEvent(
        QMouseEvent *event)
{
    QCustomPlot::mousePressEvent(event);
}

void DataView::mouseReleaseEvent(
        QMouseEvent *event)
{
    QCustomPlot::mouseReleaseEvent(event);
}

void DataView::mouseMoveEvent(
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
            emit mark(resultTime);
        }
        else
        {
            emit clear();
        }
    }
}

double DataView::distSqrToLine(
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
