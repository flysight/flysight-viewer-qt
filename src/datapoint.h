#ifndef DATAPOINT_H
#define DATAPOINT_H

#include <QDateTime>

#include <math.h>

#include "common.h"

class DataPoint
{
public:
    QDateTime   dateTime;

    bool        hasGeodetic;

    double      lat;
    double      lon;
    double      hMSL;

    double      velN;
    double      velE;
    double      velD;

    double      hAcc;
    double      vAcc;
    double      sAcc;

    double      heading;
    double      cAcc;

    int         numSV;

    double      t;
    double      x;
    double      y;
    double      z;
    double      alt;

    double      dist2D;
    double      dist3D;

    double      curv;
    double      accel;

    double      lift;
    double      drag;

    double      vx;     // Wind-corrected velocity
    double      vy;

    double      theta;
    double      omega;

    static DataPoint interpolate(const DataPoint &p1,
                                 const DataPoint &p2,
                                 double a);

    static double elevation(const DataPoint &dp)
    {
        return dp.z;
    }

    static double northSpeed(const DataPoint &dp)
    {
        return dp.vy;
    }

    static double eastSpeed(const DataPoint &dp)
    {
        return dp.vx;
    }

    static double verticalSpeed(const DataPoint &dp)
    {
        return dp.velD;
    }

    static double horizontalSpeed(const DataPoint &dp)
    {
        return sqrt(dp.vx * dp.vx + dp.vy * dp.vy);
    }

    static double totalSpeed(const DataPoint &dp)
    {
        return sqrt(dp.vx * dp.vx + dp.vy * dp.vy + dp.velD * dp.velD);
    }

    static double diveAngle(const DataPoint &dp)
    {
        const double pi = 3.14159265359;
        return atan2(dp.velD, sqrt(dp.vx * dp.vx + dp.vy * dp.vy)) / pi * 180;
    }

    static double curvature(const DataPoint &dp)
    {
        return dp.curv;
    }

    static double glideRatio(const DataPoint &dp)
    {
        if (dp.velD != 0) return sqrt(dp.vx * dp.vx + dp.vy * dp.vy) / dp.velD;
        else              return 0;
    }

    static double horizontalAccuracy(const DataPoint &dp)
    {
        return dp.hAcc;
    }

    static double verticalAccuracy(const DataPoint &dp)
    {
        return dp.vAcc;
    }

    static double speedAccuracy(const DataPoint &dp)
    {
        return dp.sAcc;
    }

    static double numberOfSatellites(const DataPoint &dp)
    {
        return dp.numSV;
    }

    static double time(const DataPoint &dp)
    {
        return dp.t;
    }

    static double distance2D(const DataPoint &dp)
    {
        return dp.dist2D;
    }

    static double distance3D(const DataPoint &dp)
    {
        return dp.dist3D;
    }

    static double acceleration(const DataPoint &dp)
    {
        return dp.accel;
    }

    static double totalEnergy(const DataPoint &dp)
    {
        const double v = totalSpeed(dp);
        return v * v / 2 + A_GRAVITY * elevation(dp);
    }

    static double energyRate(const DataPoint &dp)
    {
        return totalSpeed(dp) * acceleration(dp) - A_GRAVITY * verticalSpeed(dp);
    }

    static double liftCoefficient(const DataPoint &dp)
    {
        return dp.lift;
    }

    static double dragCoefficient(const DataPoint &dp)
    {
        return dp.drag;
    }

    static double course(const DataPoint &dp)
    {
        return dp.theta;
    }

    static double courseRate(const DataPoint &dp)
    {
        return dp.omega;
    }

    static double courseAccuracy(const DataPoint &dp)
    {
        return dp.cAcc;
    }
};

#endif // DATAPOINT_H
