#include "wideopenscoring.h"

#include "GeographicLib/Geodesic.hpp"

#include "mainwindow.h"

using namespace GeographicLib;

WideOpenScoring::WideOpenScoring(
        MainWindow *mainWindow):
    ScoringMethod(mainWindow),
    mMainWindow(mainWindow),
    mEndLatitude(51.0500),
    mEndLongitude(-114.0667),
    mBearing(0),
    mBottom(2000),
    mLaneWidth(500),
    mLaneLength(10000)
{

}

void WideOpenScoring::setEnd(
        double endLatitude,
        double endLongitude)
{
    mEndLatitude = endLatitude;
    mEndLongitude = endLongitude;
    emit dataChanged();
}

void WideOpenScoring::setBearing(
        double bearing)
{
    mBearing = bearing;
    emit dataChanged();
}

void WideOpenScoring::setBottom(
        double bottom)
{
    mBottom = bottom;
    emit dataChanged();
}

void WideOpenScoring::setLaneWidth(
        double laneWidth)
{
    mLaneWidth = laneWidth;
    emit dataChanged();
}

void WideOpenScoring::setLaneLength(
        double laneLength)
{
    mLaneLength = laneLength;
    emit dataChanged();
}

void WideOpenScoring::prepareDataPlot(
        DataPlot *plot)
{
    DataPoint dpBottom, dpTop;
    bool success;

    success = getWindowBounds(mMainWindow->data(), dpBottom, dpTop);

    // Add shading for scoring window
    if (success && plot->yValue(DataPlot::Elevation)->visible())
    {
        DataPoint dpLower = mMainWindow->interpolateDataT(mMainWindow->rangeLower());
        DataPoint dpUpper = mMainWindow->interpolateDataT(mMainWindow->rangeUpper());

        const double xMin = plot->xValue()->value(dpLower, mMainWindow->units());
        const double xMax = plot->xValue()->value(dpUpper, mMainWindow->units());

        QVector< double > xElev, yElev;

        xElev << xMin << xMax;
        yElev << plot->yValue(DataPlot::Elevation)->value(dpBottom, mMainWindow->units())
              << plot->yValue(DataPlot::Elevation)->value(dpBottom, mMainWindow->units());

        QCPGraph *graph = plot->addGraph(
                    plot->axisRect()->axis(QCPAxis::atBottom),
                    plot->yValue(DataPlot::Elevation)->axis());
        graph->setData(xElev, yElev);
        graph->setPen(QPen(QBrush(Qt::lightGray), mMainWindow->lineThickness(), Qt::DashLine));

        QCPItemRect *rect = new QCPItemRect(plot);
        plot->addItem(rect);

        rect->setPen(QPen(QBrush(Qt::lightGray), mMainWindow->lineThickness(), Qt::DashLine));
        rect->setBrush(QColor(0, 0, 0, 8));

        rect->topLeft->setType(QCPItemPosition::ptAxisRectRatio);
        rect->topLeft->setAxes(plot->xAxis, plot->yValue(DataPlot::Elevation)->axis());
        rect->topLeft->setCoords(
                    (plot->xValue()->value(dpBottom, mMainWindow->units()) - xMin) / (xMax - xMin),
                    -0.1);

        rect->bottomRight->setType(QCPItemPosition::ptAxisRectRatio);
        rect->bottomRight->setAxes(plot->xAxis, plot->yValue(DataPlot::Elevation)->axis());
        rect->bottomRight->setCoords(1.1, 1.1);
    }
}

bool WideOpenScoring::getWindowBounds(
        const QVector< DataPoint > &result,
        DataPoint &dpBottom,
        DataPoint &dpTop)
{
    bool foundBottom = false;
    int bottom;

    for (int i = result.size() - 1; i >= 0; --i)
    {
        const DataPoint &dp = result[i];

        if (dp.z < mBottom)
        {
            bottom = i;
            foundBottom = true;
        }

        if (dp.t < 0) break;
    }

    if (foundBottom)
    {
        // Calculate bottom of window
        const DataPoint &dp1 = result[bottom - 1];
        const DataPoint &dp2 = result[bottom];
        dpBottom = DataPoint::interpolate(dp1, dp2, (mBottom - dp1.z) / (dp2.z - dp1.z));

        // Calculate top of window
        dpTop = DataPoint();
        dpTop.hasGeodetic = true;

        const Geodesic &geod = Geodesic::WGS84();
        geod.Direct(mEndLatitude, mEndLongitude, mBearing + 180, mLaneLength, dpTop.lat, dpTop.lon);

        return true;
    }
    else
    {
        return false;
    }
}
