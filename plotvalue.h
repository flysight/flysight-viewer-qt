#ifndef PLOTVALUE_H
#define PLOTVALUE_H

#include <QString>

class PlotValue
{
public:
    PlotValue(const QString &titleMetric, const QString &titleImperial);
    PlotValue();

    const QString &titleMetric() { return mTitleMetric; }
    const QString &titleImperial() { return mTitleImperial; }

private:
    QString mTitleMetric;
    QString mTitleImperial;
};

#endif // PLOTVALUE_H
