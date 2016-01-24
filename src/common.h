#ifndef COMMON_H
#define COMMON_H

#include <QPointF>

#define PI          3.14159265359

#define A_GRAVITY   9.80665     // Standard acceleration due to gravity (m/s^2)
#define SL_PRESSURE 101325      // Sea level pessure (Pa)
#define LAPSE_RATE  0.0065      // Temperature lapse rate (K/m)
#define SL_TEMP     288.15      // Sea level temperature (K)
#define MM_AIR      0.0289644   // Molar mass of dry air (kg/mol)
#define GAS_CONST   8.31447     // Universal gas constant (J/mol/K)

double distSqrToLine(
        const QPointF &start,
        const QPointF &end,
        const QPointF &point,
        double &mu);

#endif // COMMON_H
