#ifndef GENOME_H
#define GENOME_H

#include <QVector>

#include "datapoint.h"

class Genome:
        public QVector< double >
{
public:
    Genome();
    Genome(const QVector< double > &rhs);
    Genome(const Genome &p1, const Genome &p2, int k);
    Genome(int genomeSize, int k, double minLift, double maxLift);

    void mutate(int k, int kMin, double minLift, double maxLift);
    void truncate(int k);
    QVector< DataPoint > simulate(double h, double a, double c,
                                  double planformArea, double mass,
                                  const DataPoint &dp0, double windowBottom);

private:
    static double dtheta_dt(double theta, double v, double x, double y, double lift,
                            double planformArea, double mass);
    static double dv_dt(double theta, double v, double x, double y, double drag,
                        double planformArea, double mass);
    static double dx_dt(double theta, double v, double x, double y);
    static double dy_dt(double theta, double v, double x, double y);

    static double lift(double cl);
    static double drag(double cl, double a, double c);
};

#endif // GENOME_H
