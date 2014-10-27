#include "common.h"

#include <QVector2D>

double distSqrToLine(
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
