#ifndef COMMON_H
#define COMMON_H

#include <QPointF>

double distSqrToLine(
        const QPointF &start,
        const QPointF &end,
        const QPointF &point,
        double &mu);

#endif // COMMON_H
