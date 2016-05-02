#include "speedscoring.h"

#include "mainwindow.h"

SpeedScoring::SpeedScoring(
        MainWindow *mainWindow):
    ScoringMethod(mainWindow),
    mMainWindow(mainWindow)
{

}

double SpeedScoring::score(
        const QVector< DataPoint > &result)
{
    const double bottom = mMainWindow->windowBottom();
    const double top = mMainWindow->windowTop();

    DataPoint dpBottom, dpTop;
    if (mMainWindow->getWindowBounds(result, dpBottom, dpTop))
    {
        return (top - bottom) / (dpBottom.t - dpTop.t);
    }

    return 0;
}

QString SpeedScoring::scoreAsText(
        double score)
{
    return (mMainWindow->units() == PlotValue::Metric) ?
                QString::number(score * MPS_TO_KMH) + QString(" km/h"):
                QString::number(score * MPS_TO_MPH) + QString(" mph");
}
