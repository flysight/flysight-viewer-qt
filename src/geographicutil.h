#ifndef GEOGRAPHICUTIL_H
#define GEOGRAPHICUTIL_H

namespace GeographicUtil
{
    void intercept(double lata1, double lona1, double lata2, double lona2,
                   double latb1, double lonb1, double &lat0, double &lon0);
}

#endif // GEOGRAPHICUTIL_H
