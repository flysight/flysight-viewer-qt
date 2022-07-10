/***************************************************************************
**                                                                        **
**  FlySight Viewer                                                       **
**  Copyright 2018 Michael Cooper                                         **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>. **
**                                                                        **
****************************************************************************
**  Contact: Michael Cooper                                               **
**  Website: http://flysight.ca/                                          **
****************************************************************************/

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

    double      dist2D;
    double      dist3D;

    double      curv;
    double      accel;

    double      ax;
    double      ay;
    double      az;
    double      amag;

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

    static double northSpeedRaw(const DataPoint &dp)
    {
        return dp.velN;
    }

    static double eastSpeedRaw(const DataPoint &dp)
    {
        return dp.velE;
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

    static double accForward(const DataPoint &dp)
    {
        return dp.ax;
    }

    static double accRight(const DataPoint &dp)
    {
        return dp.ay;
    }

    static double accDown(const DataPoint &dp)
    {
        return dp.az;
    }

    static double accMagnitude(const DataPoint &dp)
    {
        return dp.amag;
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
