#ifndef DATAPOINT_H
#define DATAPOINT_H

#include <QDateTime>

#include <math.h>

#define A_GRAVITY   9.80665     // Standard acceleration due to gravity (m/s^2)
#define SL_PRESSURE 101325      // Sea level pessure (Pa)
#define LAPSE_RATE  0.0065      // Temperature lapse rate (K/m)
#define SL_TEMP     288.15      // Sea level temperature (K)
#define MM_AIR      0.0289644   // Molar mass of dry air (kg/mol)
#define GAS_CONST   8.31447     // Universal gas constant (J/mol/K)

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

    double      temp;
    double      mass;
    double      area;

    double      accelLift;
    double      accelDrag;

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

    static double airPressure(const DataPoint &dp)
    {
        // From https://en.wikipedia.org/wiki/Atmospheric_pressure#Altitude_variation
        return SL_PRESSURE * pow(1 - LAPSE_RATE * dp.hMSL / SL_TEMP, A_GRAVITY * MM_AIR / GAS_CONST / LAPSE_RATE);
    }

    static double airDensity(const DataPoint &dp)
    {
        // From https://en.wikipedia.org/wiki/Density_of_air
        return airPressure(dp) / (GAS_CONST / MM_AIR) / temperature(dp);
    }

    static double dynamicPressure(const DataPoint &dp)
    {
        // From https://en.wikipedia.org/wiki/Dynamic_pressure
        const double v = totalSpeed(dp);
        return airDensity(dp) * v * v / 2;
    }

    static double temperature(const DataPoint &dp)
    {
        return dp.temp;
    }

    static double lift(const DataPoint &dp)
    {
        return dp.mass * dp.accelLift / dynamicPressure(dp) / dp.area;
    }

    static double drag(const DataPoint &dp)
    {
        return dp.mass * dp.accelDrag / dynamicPressure(dp) / dp.area;
    }
};

#endif // DATAPOINT_H
