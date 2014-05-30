#ifndef WAYPOINT_H
#define WAYPOINT_H

class Waypoint
{
public:
    double m_lat, m_lon, m_hMSL;

    Waypoint();
    Waypoint(double lat, double lon, double hMSL);
};

#endif // WAYPOINT_H
