#ifndef GENOME_H
#define GENOME_H

#include <QVector>

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
};

#endif // GENOME_H
