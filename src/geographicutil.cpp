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

#include "geographicutil.h"

#include "GeographicLib/Constants.hpp"
#include "GeographicLib/Geodesic.hpp"
#include "GeographicLib/Gnomonic.hpp"

using namespace GeographicLib;

// Compute the geodesic intercept from a point at (latb1, lonb1) to a geodesic
// between (lata1, lona1) and (lata2, lona2).
//
// From https://sourceforge.net/p/geographiclib/discussion/1026621/thread/21aaff9f/#8a93
// See http://arxiv.org/pdf/1102.1215v1.pdf

void GeographicUtil::intercept(
        double lata1,
        double lona1,
        double lata2,
        double lona2,
        double latb1,
        double lonb1,
        double &lat0,
        double &lon0)
{
    class vector3 {
    public:
        double _x, _y, _z;
        vector3(double x, double y, double z = 1) throw()
            : _x(x)
            , _y(y)
            , _z(z) {}
        vector3 cross(const vector3& b) const throw() {
            return vector3(_y * b._z - _z * b._y,
                _z * b._x - _x * b._z,
                _x * b._y - _y * b._x);
        }
        void norm() throw() {
            _x /= _z;
            _y /= _z;
            _z = 1;
        }
    };

    const Geodesic geod = Geodesic::WGS84();
    const Gnomonic gn(geod);

    // Possibly need to deal with longitudes wrapping around
    lat0 = (lata1 + lata2) / 2;
    lon0 = (lona1 + lona2) / 2;

    for (int i = 0; i < 10; ++i)
    {
        double xa1, ya1, xa2, ya2;
        double xb1, yb1;

        // Convert to Gnomonic projection
        gn.Forward(lat0, lon0, lata1, lona1, xa1, ya1);
        gn.Forward(lat0, lon0, lata2, lona2, xa2, ya2);
        gn.Forward(lat0, lon0, latb1, lonb1, xb1, yb1);

        // See Hartley and Zisserman, Multiple View Geometry, Sec. 2.2.1
        vector3 va1(xa1, ya1);
        vector3 va2(xa2, ya2);

        // la is homogeneous representation of line A1,A2
        vector3 la = va1.cross(va2);

        // lb is homogeneous representation of line thru B1 perpendicular to la
        vector3 lb(la._y, -la._x, la._x * yb1 - la._y * xb1);

        // p0 is homogeneous representation of intersection of la and lb
        vector3 p0 = la.cross(lb);
        p0.norm();

        double lat1, lon1;
        gn.Reverse(lat0, lon0, p0._x, p0._y, lat1, lon1);

        lat0 = lat1;
        lon0 = lon1;
    }
}
