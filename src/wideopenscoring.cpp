#include "wideopenscoring.h"

#include <QVector>
#include <QWebFrame>
#include <QWebElement>

#include "GeographicLib/Geodesic.hpp"
#include "GeographicLib/GeodesicLine.hpp"

#include "mainwindow.h"
#include "mapview.h"

#define MAX_SPLIT_DEPTH 8

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
    mLaneLength(10000),
    mMapMode(None)
{

}

void WideOpenScoring::setEnd(
        double endLatitude,
        double endLongitude)
{
    mEndLatitude = endLatitude;
    mEndLongitude = endLongitude;
    emit scoringChanged();
}

void WideOpenScoring::setBearing(
        double bearing)
{
    mBearing = bearing;
    emit scoringChanged();
}

void WideOpenScoring::setBottom(
        double bottom)
{
    mBottom = bottom;
    emit scoringChanged();
}

void WideOpenScoring::setLaneWidth(
        double laneWidth)
{
    mLaneWidth = laneWidth;
    emit scoringChanged();
}

void WideOpenScoring::setLaneLength(
        double laneLength)
{
    mLaneLength = laneLength;
    emit scoringChanged();
}

void WideOpenScoring::setMapMode(
        MapMode mode)
{
    mMapMode = mode;
    emit scoringChanged();
}

void WideOpenScoring::prepareDataPlot(
        DataPlot *plot)
{
    DataPoint dpBottom;
    bool success = getWindowBounds(mMainWindow->data(), dpBottom);

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

void WideOpenScoring::prepareMapView(
        MapView *view)
{
    // Distance threshold
    const double earthCircumference = 40075000; // m
    const double zoom = view->page()->currentFrame()->documentElement().evaluateJavaScript("map.getZoom();").toDouble();
    const double threshold = earthCircumference / pow(2, zoom) / view->width();

    // Draw lane center
    double woProjLat, woProjLon;
    Geodesic::WGS84().Direct(mEndLatitude, mEndLongitude, mBearing, mLaneLength, woProjLat, woProjLon);

    QVector< double > lat, lon;

    lat.push_back(mEndLatitude);
    lon.push_back(mEndLongitude);

    splitLine(lat, lon, mEndLatitude, mEndLongitude, woProjLat, woProjLon, threshold, 0);

    lat.push_back(woProjLat);
    lon.push_back(woProjLon);

    QString js;
    for (int i = 0; i < lat.size(); ++i)
    {
        js += QString("path2.push(new google.maps.LatLng(%1, %2));").arg(lat[i], 0, 'f').arg(lon[i], 0, 'f');
    }

    // Draw shading around lane
    QVector< double > ltLat, ltLon, rtLat, rtLon;
    for (int i = 0; i < lat.size(); ++i)
    {
        double ltBearing, rtBearing;

        if (i + 1 < lat.size())
        {
            double azi1, azi2;
            Geodesic::WGS84().Inverse(lat[i], lon[i], lat[i + 1], lon[i + 1], azi1, azi2);

            ltBearing = azi1 + 90;
            rtBearing = azi1 - 90;
        }
        else if (i - 1 >= 0)
        {
            double azi1, azi2;
            Geodesic::WGS84().Inverse(lat[i - 1], lon[i - 1], lat[i], lon[i], azi1, azi2);

            ltBearing = azi2 + 90;
            rtBearing = azi2 - 90;
        }

        double tempLat, tempLon;
        Geodesic::WGS84().Direct(lat[i], lon[i], ltBearing, mLaneWidth / 2, tempLat, tempLon);
        ltLat.push_back(tempLat);
        ltLon.push_back(tempLon);

        Geodesic::WGS84().Direct(lat[i], lon[i], rtBearing, mLaneWidth / 2, tempLat, tempLon);
        rtLat.push_front(tempLat);
        rtLon.push_front(tempLon);
    }

    // Now take ltLat + rtLat (and same with lon) to form loop
    for (int i = 0; i < ltLat.size(); ++i)
    {
        js += QString("path3.push(new google.maps.LatLng(%1, %2));").arg(ltLat[i], 0, 'f').arg(ltLon[i], 0, 'f');
    }

    for (int i = 0; i < rtLat.size(); ++i)
    {
        js += QString("path3.push(new google.maps.LatLng(%1, %2));").arg(rtLat[i], 0, 'f').arg(rtLon[i], 0, 'f');
    }

    // Draw finish line
    double woLeftLat, woLeftLon;
    Geodesic::WGS84().Direct(mEndLatitude, mEndLongitude, mBearing - 90, mLaneLength / 2, woLeftLat, woLeftLon);

    double woRightLat, woRightLon;
    Geodesic::WGS84().Direct(mEndLatitude, mEndLongitude, mBearing + 90, mLaneLength / 2, woRightLat, woRightLon);

    lat.clear();
    lon.clear();

    lat.push_back(woLeftLat);
    lon.push_back(woLeftLon);

    splitLine(lat, lon, woLeftLat, woLeftLon, woRightLat, woRightLon, threshold, 0);

    lat.push_back(woRightLat);
    lon.push_back(woRightLon);

    for (int i = 0; i < lat.size(); ++i)
    {
        js += QString("path4.push(new google.maps.LatLng(%1, %2));").arg(lat[i], 0, 'f').arg(lon[i], 0, 'f');
    }

    view->page()->currentFrame()->documentElement().evaluateJavaScript(js);
}

void WideOpenScoring::splitLine(
        QVector< double > &lat,
        QVector< double > &lon,
        double startLat,
        double startLon,
        double endLat,
        double endLon,
        double threshold,
        int depth)
{
    GeodesicLine l = Geodesic::WGS84().InverseLine(startLat, startLon, endLat, endLon);
    double midLat, midLon;
    l.Position(l.Distance() / 2, midLat, midLon);

    double dist;
    Geodesic::WGS84().Inverse((startLat + endLat) / 2, (startLon + endLon) / 2, midLat, midLon, dist);

    if (dist > threshold && depth < MAX_SPLIT_DEPTH)
    {
        splitLine(lat, lon, startLat, startLon, midLat, midLon, threshold, depth + 1);

        lat.push_back(midLat);
        lon.push_back(midLon);

        splitLine(lat, lon, midLat, midLon, endLat, endLon, threshold, depth + 1);
    }
}

bool WideOpenScoring::getWindowBounds(
        const QVector< DataPoint > &result,
        DataPoint &dpBottom)
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

        return true;
    }
    else
    {
        return false;
    }
}

bool WideOpenScoring::updateReference(
        double lat,
        double lon)
{
    if (mMapMode == Start)
    {
        double azi1, azi2;
        Geodesic::WGS84().Inverse(mEndLatitude, mEndLongitude, lat, lon, azi1, azi2);
        setBearing(azi1);
        return true;
    }
    else if (mMapMode == End)
    {
        setEnd(lat, lon);
        return true;
    }
    else
    {
        return false;
    }
}

void WideOpenScoring::closeReference()
{
    setMapMode(None);
    mMainWindow->setFocus();
}
