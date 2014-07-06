#include "plotvalue.h"

PlotValue::PlotValue(
        const QString &titleMetric,
        const QString &titleImperial,
        const QColor  &color):
    mTitleMetric(titleMetric),
    mTitleImperial(titleImperial),
    mColor(color)
{

}

PlotValue::PlotValue()
{
}
