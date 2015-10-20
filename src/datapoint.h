#ifndef DATAPOINT_H
#define DATAPOINT_H

#include <QDateTime>

#include <math.h>

#include "common.h"

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

    double      hAcc;
    double      vAcc;
    double      sAcc;

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

    double      windE;
    double      windN;
    double      velAircraft;
    double      windErr;

    double      lift;
    double      drag;

    double      optimal_lift;
    double      optimal_drag;

    double      optimal_velH;
    double      optimal_velD;
    double      optimal_hMSL;
    double      optimal_alt;

    static DataPoint interpolate(const DataPoint &p1,
                                 const DataPoint &p2,
                                 double a);

    static double elevation(const DataPoint &dp)
    {
        return dp.alt;
    }

    static double northSpeed(const DataPoint &dp)
    {
        return dp.velN;
    }

    static double eastSpeed(const DataPoint &dp)
    {
        return dp.velE;
    }

    static double verticalSpeed(const DataPoint &dp)
    {
        return dp.velD;
    }

    static double horizontalSpeed(const DataPoint &dp)
    {
        return sqrt(dp.velE * dp.velE + dp.velN * dp.velN);
    }

    static double totalSpeed(const DataPoint &dp)
    {
        return sqrt(dp.velE * dp.velE + dp.velN * dp.velN + dp.velD * dp.velD);
    }

    static double diveAngle(const DataPoint &dp)
    {
        const double pi = 3.14159265359;
        return atan2(dp.velD, sqrt(dp.velE * dp.velE + dp.velN * dp.velN)) / pi * 180;
    }

    static double curvature(const DataPoint &dp)
    {
        return dp.curv;
    }

    static double glideRatio(const DataPoint &dp)
    {
        if (dp.velD != 0) return sqrt(dp.velE * dp.velE + dp.velN * dp.velN) / dp.velD;
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

    static double windSpeed(const DataPoint &dp)
    {
        return sqrt(dp.windE * dp.windE + dp.windN * dp.windN);
    }

    static double windDirection(const DataPoint &dp)
    {
        return fmod(atan2(-dp.windE, -dp.windN) / M_PI * 180 + 360, 360);
    }

    static double aircraftSpeed(const DataPoint &dp)
    {
        return dp.velAircraft;
    }

    static double windError(const DataPoint &dp)
    {
        return dp.windErr;
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
};

#endif // DATAPOINT_H
