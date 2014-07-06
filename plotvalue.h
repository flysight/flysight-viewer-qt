#ifndef PLOTVALUE_H
#define PLOTVALUE_H

#include <QColor>
#include <QString>

class PlotValue
{
public:
    PlotValue(const QString &titleMetric, const QString &titleImperial,
              const QColor &color);
    PlotValue();

    const QColor &color() { return mColor; }
    const QString &titleMetric() { return mTitleMetric; }
    const QString &titleImperial() { return mTitleImperial; }

private:
    QString mTitleMetric;
    QString mTitleImperial;
    QColor  mColor;
};

#endif // PLOTVALUE_H
