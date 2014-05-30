#ifndef DATAPOINT_H
#define DATAPOINT_H

#include <QDateTime>

class DataPoint
{
public:
    QDateTime   dateTime;

    double      lat;
    double      lon;
    double      hMSL;

    double      velN;
    double      velE;
    double      velD;

    double      t;
    double      x;
    double      y;

    double      dist2D;
    double      dist3D;

    double      curv;
};

#endif // DATAPOINT_H
