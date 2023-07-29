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

#ifndef COMMON_H
#define COMMON_H

#include <QPointF>

#define PI          3.14159265359
#define SQRT_2      1.41421356237

#define A_GRAVITY   9.80665     // Standard acceleration due to gravity (m/s^2)
#define SL_PRESSURE 101325      // Sea level pessure (Pa)
#define LAPSE_RATE  0.0065      // Temperature lapse rate (K/m)
#define SL_TEMP     288.15      // Sea level temperature (K)
#define MM_AIR      0.0289644   // Molar mass of dry air (kg/mol)
#define GAS_CONST   8.31447     // Universal gas constant (J/mol/K)

#define METERS_TO_FEET 3.280839895
#define MPS_TO_MPH     2.23694
#define MPS_TO_KMH     3.6

double distSqrToLine(
        const QPointF &start,
        const QPointF &end,
        const QPointF &point,
        double &mu);

#endif // COMMON_H
