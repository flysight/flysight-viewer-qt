#include "genome.h"

Genome::Genome()
{

}

Genome::Genome(
        const QVector< double > &rhs):
    QVector< double >(rhs)
{
}

Genome::Genome(
        const Genome &p1,
        const Genome &p2,
        int k)
{
    const int parts = 1 << k;
    const int partSize = (p1.size() - 1) / parts;

    const int pivot = qrand() % parts;

    const int j1 = pivot * partSize;
    const int j2 = (pivot + 1) * partSize;

    const double cl1 = p1[j1];
    const double cl2 = p2[j2];

    append(p1.mid(0, j1));
    for (int i = 0; i < partSize; ++i)
    {
        append(cl1 + (double) i / partSize * (cl2 - cl1));
    }
    append(p2.mid(j2, p2.size() - j2));
}

Genome::Genome(
        int genomeSize,
        int k,
        double minLift,
        double maxLift)
{
    const int parts = 1 << k;
    const int partSize = (genomeSize - 1) / parts;

    double prevLift = minLift + (double) qrand() / RAND_MAX * (maxLift - minLift);
    for (int i = 0; i < parts; ++i)
    {
        double nextLift = minLift + (double) qrand() / RAND_MAX * (maxLift - minLift);
        for (int j = 0; j < partSize; ++j)
        {
            append(prevLift + (double) j / partSize * (nextLift - prevLift));
        }
        prevLift = nextLift;
    }
    append(prevLift);
}

void Genome::mutate(
        int k,
        int kMin,
        double minLift,
        double maxLift)
{
    const int parts = 1 << k;
    const int partSize = (size() - 1) / parts;

    const int i = qrand() % (parts + 1);
    const double cl = at(i * partSize);

    const double range = maxLift / (1 << (k - kMin));
    const double minr = qMax(minLift - cl, -range);
    const double maxr = qMin(maxLift - cl,  range);
    const double r = minr + (double) qrand() / RAND_MAX * (maxr - minr);

    if (i > 0)
    {
        const int jPrev = (i - 1) * partSize;
        const int jNext = i * partSize;

        const double rPrev = 0.0;
        const double rNext = r;

        for (int j = jPrev; j < jNext; ++j)
        {
            const double r = rPrev + (rNext - rPrev) * (j - jPrev) / (jNext - jPrev);
            replace(j, qMax(minLift, qMin(maxLift, at(j) + r)));
        }
    }

    if (i < parts)
    {
        const int jPrev = i * partSize;
        const int jNext = (i + 1) * partSize;

        const double rPrev = r;
        const double rNext = 0.0;

        for (int j = jPrev; j < jNext; ++j)
        {
            const double r = rPrev + (rNext - rPrev) * (j - jPrev) / (jNext - jPrev);
            replace(j, qMax(minLift, qMin(maxLift, at(j) + r)));
        }
    }
}

void Genome::truncate(
        int k)
{
    const int parts = 1 << k;
    const int partSize = (size() - 1) / parts;

    erase(begin(), begin() + partSize);
    append(QVector< double >(partSize, last()));
}

QVector< DataPoint > Genome::simulate(
        double h,
        double a,
        double c,
        double planformArea,
        double mass,
        const DataPoint &dp0,
        double windowBottom)
{
    const double velH = sqrt(dp0.vx * dp0.vx + dp0.vy * dp0.vy);

    double t      = dp0.t;
    double theta  = atan2(-dp0.velD, velH);
    double v      = sqrt(dp0.velD * dp0.velD + velH * velH);
    double x      = 0;
    double y      = dp0.hMSL;
    double dist2D = dp0.dist2D;
    double dist3D = dp0.dist3D;

    QVector< DataPoint > result(1, dp0);

    for (int i = 0; i < size(); ++i)
    {
        const double lift_prev = lift(at(i));
        const double drag_prev = drag(at(i), a, c);

        const double lift_next = lift(at(i + 1));
        const double drag_next = drag(at(i + 1), a, c);

        // Runge-Kutta integration
        // See https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods
        const double k0 = h * dtheta_dt(theta, v, x, y, lift_prev, planformArea, mass);
        const double l0 = h *     dv_dt(theta, v, x, y, drag_prev, planformArea, mass);
        const double m0 = h *     dx_dt(theta, v, x, y);
        const double n0 = h *     dy_dt(theta, v, x, y);

        const double k1 = h * dtheta_dt(theta + k0/2, v + l0/2, x + m0/2, y + n0/2, (lift_prev + lift_next) / 2, planformArea, mass);
        const double l1 = h *     dv_dt(theta + k0/2, v + l0/2, x + m0/2, y + n0/2, (drag_prev + drag_next) / 2, planformArea, mass);
        const double m1 = h *     dx_dt(theta + k0/2, v + l0/2, x + m0/2, y + n0/2);
        const double n1 = h *     dy_dt(theta + k0/2, v + l0/2, x + m0/2, y + n0/2);

        const double k2 = h * dtheta_dt(theta + k1/2, v + l1/2, x + m1/2, y + n1/2, (lift_prev + lift_next) / 2, planformArea, mass);
        const double l2 = h *     dv_dt(theta + k1/2, v + l1/2, x + m1/2, y + n1/2, (drag_prev + drag_next) / 2, planformArea, mass);
        const double m2 = h *     dx_dt(theta + k1/2, v + l1/2, x + m1/2, y + n1/2);
        const double n2 = h *     dy_dt(theta + k1/2, v + l1/2, x + m1/2, y + n1/2);

        const double k3 = h * dtheta_dt(theta + k2, v + l2, x + m2, y + n2, lift_next, planformArea, mass);
        const double l3 = h *     dv_dt(theta + k2, v + l2, x + m2, y + n2, drag_next, planformArea, mass);
        const double m3 = h *     dx_dt(theta + k2, v + l2, x + m2, y + n2);
        const double n3 = h *     dy_dt(theta + k2, v + l2, x + m2, y + n2);

        const double dtheta = (k0 + 2 * k1 + 2 * k2 + k3) / 6;
        const double dv     = (l0 + 2 * l1 + 2 * l2 + l3) / 6;
        const double dx     = (m0 + 2 * m1 + 2 * m2 + m3) / 6;
        const double dy     = (n0 + 2 * n1 + 2 * n2 + n3) / 6;

        t     += h;
        theta += dtheta;
        v     += dv;
        x     += dx;
        y     += dy;

        // Add data point
        DataPoint pt;

        pt.hasGeodetic = false;

        pt.hMSL  = y;

        pt.vx    = 0;
        pt.vy    = v * cos(theta);
        pt.velD  = -v * sin(theta);

        pt.t = t;
        pt.x = x;
        pt.y = 0;
        pt.z = pt.alt = y + dp0.alt - dp0.hMSL;

        dist2D += dx;
        dist3D += sqrt(dx * dx + dy * dy);

        pt.dist2D = dist2D;
        pt.dist3D = dist3D;

        // curv
        // accel

        pt.lift = lift_next;
        pt.drag = drag_next;

        result.append(pt);

        if (pt.alt < windowBottom) break;
    }

    return result;
}

double Genome::dtheta_dt(
        double theta,
        double v,
        double x,
        double y,
        double lift,
        double planformArea,
        double mass)
{
    // From https://en.wikipedia.org/wiki/Atmospheric_pressure#Altitude_variation
    const double airPressure = SL_PRESSURE * pow(1 - LAPSE_RATE * y / SL_TEMP, A_GRAVITY * MM_AIR / GAS_CONST / LAPSE_RATE);

    // From https://en.wikipedia.org/wiki/Lapse_rate
    const double temperature = SL_TEMP - LAPSE_RATE * y;

    // From https://en.wikipedia.org/wiki/Density_of_air
    const double airDensity = airPressure / (GAS_CONST / MM_AIR) / temperature;

    // From https://en.wikipedia.org/wiki/Dynamic_pressure
    const double dynamicPressure = airDensity * v * v / 2;

    // Calculate acceleration due to drag and lift
    const double accelLift = dynamicPressure * planformArea * lift / mass;

    return (accelLift - A_GRAVITY * cos(theta)) / v;
}

double Genome::dv_dt(
        double theta,
        double v,
        double x,
        double y,
        double drag,
        double planformArea,
        double mass)
{
    // From https://en.wikipedia.org/wiki/Atmospheric_pressure#Altitude_variation
    const double airPressure = SL_PRESSURE * pow(1 - LAPSE_RATE * y / SL_TEMP, A_GRAVITY * MM_AIR / GAS_CONST / LAPSE_RATE);

    // From https://en.wikipedia.org/wiki/Lapse_rate
    const double temperature = SL_TEMP - LAPSE_RATE * y;

    // From https://en.wikipedia.org/wiki/Density_of_air
    const double airDensity = airPressure / (GAS_CONST / MM_AIR) / temperature;

    // From https://en.wikipedia.org/wiki/Dynamic_pressure
    const double dynamicPressure = airDensity * v * v / 2;

    // Calculate acceleration due to drag and lift
    const double accelDrag = dynamicPressure * planformArea * drag / mass;

    return -accelDrag - A_GRAVITY * sin(theta);
}

double Genome::dx_dt(
        double theta,
        double v,
        double x,
        double y)
{
    return v * cos(theta);
}

double Genome::dy_dt(
        double theta,
        double v,
        double x,
        double y)
{
    return v * sin(theta);
}

double Genome::lift(
        double cl)
{
    return cl;
}

double Genome::drag(
        double cl,
        double a,
        double c)
{
    return a * cl * cl + c;
}
