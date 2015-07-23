#include "datapoint.h"

DataPoint DataPoint::interpolate(
        const DataPoint &p1,
        const DataPoint &p2,
        double a)
{
    DataPoint ret;

    const qint64 ms1 = p1.dateTime.toMSecsSinceEpoch();
    const qint64 ms2 = p1.dateTime.toMSecsSinceEpoch();
    ret.dateTime.fromMSecsSinceEpoch(
                ms1 + (qint64) (a * (ms2 - ms1)));

    ret.lat = p1.lat + a * (p2.lat - p1.lat);
    ret.lon = p1.lon + a * (p2.lon - p1.lon);
    ret.hMSL = p1.hMSL + a * (p2.hMSL - p1.hMSL);

    ret.velN = p1.velN + a * (p2.velN - p1.velN);
    ret.velE = p1.velE + a * (p2.velE - p1.velE);
    ret.velD = p1.velD + a * (p2.velD - p1.velD);

    ret.hAcc = p1.hAcc + a * (p2.hAcc - p1.hAcc);
    ret.vAcc = p1.vAcc + a * (p2.vAcc - p1.vAcc);
    ret.sAcc = p1.sAcc + a * (p2.sAcc - p1.sAcc);

    if (a < 0.5) ret.numSV = p1.numSV;
    else         ret.numSV = p2.numSV;

    ret.t = p1.t + a * (p2.t - p1.t);
    ret.x = p1.x + a * (p2.x - p1.x);
    ret.y = p1.y + a * (p2.y - p1.y);
    ret.z = p1.z + a * (p2.z - p1.z);
    ret.alt = p1.alt + a * (p2.alt - p1.alt);

    ret.dist2D = p1.dist2D + a * (p2.dist2D - p1.dist2D);
    ret.dist3D = p1.dist3D + a * (p2.dist3D - p1.dist3D);

    ret.curv = p1.curv + a * (p2.curv - p1.curv);
    ret.accel = p1.accel + a * (p2.accel - p1.accel);

    ret.windE = p1.windE + a * (p2.windE - p1.windE);
    ret.windN = p1.windN + a * (p2.windN - p1.windN);
    ret.velAircraft = p1.velAircraft + a * (p2.velAircraft - p1.velAircraft);
    ret.windErr = p1.windErr + a * (p2.windErr - p1.windErr);

    ret.temp = p1.temp + a * (p2.temp - p1.temp);
    ret.accelLift = p1.accelLift + a * (p2.accelLift - p1.accelLift);
    ret.accelDrag = p1.accelDrag + a * (p2.accelDrag - p1.accelDrag);

    return ret;
}
