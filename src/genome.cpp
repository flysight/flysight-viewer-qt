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
    append(QVector< double >(back(), partSize));
}
