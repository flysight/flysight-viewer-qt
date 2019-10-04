#ifndef NAV_H
#define NAV_H

#include <stdint.h>

class Config;

class Nav
{
public:
    static int32_t calcDistance(int32_t lat1, int32_t lon1, int32_t lat2, int32_t lon2);
    static int calcDirection(const Config &config, int32_t lat, int32_t lon, int32_t chead);
    static int calcRelBearing(int bearing, int heading);

private:
    static float dtor(float degrees);
    static float rtod(float radians);
    static int32_t round_nearest(float f);
    static int calcBearing(float lat1, float lon1, float lat2, float lon2);
    static int32_t calcDistanceRad(float lat1, float lon1, float lat2, float lon2);
};

#endif // NAV_H
