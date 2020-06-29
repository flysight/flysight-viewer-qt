/***************************************************************************
**                                                                        **
**  FlySight Viewer                                                       **
**  Copyright 2018 Michael Cooper                                         **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>. **
**                                                                        **
****************************************************************************
**  Contact: Michael Cooper                                               **
**  Website: http://flysight.ca/                                          **
****************************************************************************/

#include "wideopendistancescoring.h"

#include <QSettings>
#include <QVector>

#include "GeographicLib/Geodesic.hpp"
#include "GeographicLib/GeodesicLine.hpp"

#include "geographicutil.h"
#include "mainwindow.h"
#include "mapview.h"
#include "mapcore.h"

#define MAX_SPLIT_DEPTH 8

using namespace GeographicLib;
using namespace GeographicUtil;

WideOpenDistanceScoring::WideOpenDistanceScoring(
        MainWindow *mainWindow):
    ScoringMethod(mainWindow),
    mMainWindow(mainWindow),
    mEndLatitude(51.0500),
    mEndLongitude(-114.0667),
    mBearing(0),
    mBottom(6000 / METERS_TO_FEET),
    mLaneWidth(500),
    mLaneLength(10000),
    mMapMode(None)
{

}

void WideOpenDistanceScoring::readSettings()
{
    QSettings settings("FlySight", "Viewer");

    settings.beginGroup("wideOpenDistanceScoring");
        mEndLatitude  = settings.value("endLatitude", mEndLatitude).toDouble();
        mEndLongitude = settings.value("endLongitude", mEndLongitude).toDouble();
        mBearing      = settings.value("bearing", mBearing).toDouble();
        mBottom       = settings.value("bottom", mBottom).toDouble();
        mLaneWidth    = settings.value("laneWidth", mLaneWidth).toDouble();
        mLaneLength   = settings.value("laneLength", mLaneLength).toDouble();
    settings.endGroup();
}

void WideOpenDistanceScoring::writeSettings()
{
    QSettings settings("FlySight", "Viewer");

    settings.beginGroup("wideOpenDistanceScoring");
        settings.setValue("endLatitude", mEndLatitude);
        settings.setValue("endLongitude", mEndLongitude);
        settings.setValue("bearing", mBearing);
        settings.setValue("bottom", mBottom);
        settings.setValue("laneWidth", mLaneWidth);
        settings.setValue("laneLength", mLaneLength);
    settings.endGroup();
}

void WideOpenDistanceScoring::setEnd(
        double endLatitude,
        double endLongitude)
{
    mEndLatitude = endLatitude;
    mEndLongitude = endLongitude;
    emit scoringChanged();
}

void WideOpenDistanceScoring::setBearing(
        double bearing)
{
    mBearing = bearing;
    emit scoringChanged();
}

void WideOpenDistanceScoring::setBottom(
        double bottom)
{
    mBottom = bottom;
    emit scoringChanged();
}

void WideOpenDistanceScoring::setLaneWidth(
        double laneWidth)
{
    mLaneWidth = laneWidth;
    emit scoringChanged();
}

void WideOpenDistanceScoring::setLaneLength(
        double laneLength)
{
    mLaneLength = laneLength;
    emit scoringChanged();
}

void WideOpenDistanceScoring::setMapMode(
        MapMode mode)
{
    mMapMode = mode;
    emit scoringChanged();
}

void WideOpenDistanceScoring::prepareDataPlot(
        DataPlot *plot)
{
    // Return now if plot empty
    if (mMainWindow->dataSize() == 0) return;

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

void WideOpenDistanceScoring::prepareMapView(
        MapView *view)
{
    // Return now if plot empty
    if (mMainWindow->dataSize() == 0) return;

    // Distance threshold
    const double threshold = view->metersPerPixel();

    // Get start point
    double woProjLat, woProjLon;
    Geodesic::WGS84().Direct(mEndLatitude, mEndLongitude, mBearing, mLaneLength, woProjLat, woProjLon);

    // Draw lane center
    QList<QVariant> data;

    QMap<QString, QVariant> val;
    val["lat"] = mEndLatitude;
    val["lng"] = mEndLongitude;
    data.push_back(val);

    splitLine(data, mEndLatitude, mEndLongitude, woProjLat, woProjLon, threshold, 0);

    val["lat"] = woProjLat;
    val["lng"] = woProjLon;
    data.push_back(val);

    // Add to map
    view->mapCore()->addPolyline(data);

    // Draw shading around lane
    QList<QVariant> lt, rt;
    for (int i = 0; i < data.size(); ++i)
    {
        double lat1 = data[i].toMap()["lat"].toDouble();
        double lon1 = data[i].toMap()["lng"].toDouble();

        double ltBearing, rtBearing;

        if (i + 1 < data.size())
        {
            double lat2 = data[i+1].toMap()["lat"].toDouble();
            double lon2 = data[i+1].toMap()["lng"].toDouble();

            double azi1, azi2;
            Geodesic::WGS84().Inverse(lat1, lon1, lat2, lon2, azi1, azi2);

            ltBearing = azi1 + 90;
            rtBearing = azi1 - 90;
        }
        else if (i - 1 >= 0)
        {
            double lat2 = data[i-1].toMap()["lat"].toDouble();
            double lon2 = data[i-1].toMap()["lng"].toDouble();

            double azi1, azi2;
            Geodesic::WGS84().Inverse(lat2, lon2, lat1, lon1, azi1, azi2);

            ltBearing = azi2 + 90;
            rtBearing = azi2 - 90;
        }

        double tempLat, tempLon;
        Geodesic::WGS84().Direct(lat1, lon1, ltBearing, mLaneWidth / 2, tempLat, tempLon);

        QMap<QString, QVariant> val;
        val["lat"] = tempLat;
        val["lng"] = tempLon;
        lt.push_back(val);

        Geodesic::WGS84().Direct(lat1, lon1, rtBearing, mLaneWidth / 2, tempLat, tempLon);

        val["lat"] = tempLat;
        val["lng"] = tempLon;
        rt.push_front(val);
    }

    // Now take lt + rt to form loop
    view->mapCore()->addPolygon(lt + rt);

    // Find exit point
    DataPoint dp0 = mMainWindow->interpolateDataT(0);

    // Find where we cross the bottom
    DataPoint dpBottom;
    bool success = getWindowBounds(mMainWindow->data(), dpBottom);

    if (mMainWindow->dataSize() == 0)
    {
        // Draw long finish line
        double woLeftLat, woLeftLon;
        Geodesic::WGS84().Direct(mEndLatitude, mEndLongitude, mBearing - 90, mLaneLength / 2, woLeftLat, woLeftLon);

        double woRightLat, woRightLon;
        Geodesic::WGS84().Direct(mEndLatitude, mEndLongitude, mBearing + 90, mLaneLength / 2, woRightLat, woRightLon);

        data.clear();

        QMap<QString, QVariant> val;
        val["lat"] = woLeftLat;
        val["lng"] = woLeftLon;
        data.push_back(val);

        splitLine(data, woLeftLat, woLeftLon, woRightLat, woRightLon, threshold, 0);

        val["lat"] = woRightLat;
        val["lng"] = woRightLon;
        data.push_back(val);

        // Add to map
        view->mapCore()->addPolyline(data);
    }
    else if (dp0.z >= mBottom && success)
    {
        // Get projected point
        double lat0, lon0;
        intercept(woProjLat, woProjLon, mEndLatitude, mEndLongitude, dpBottom.lat, dpBottom.lon, lat0, lon0);

        // Distance from top
        double topDist;
        Geodesic::WGS84().Inverse(woProjLat, woProjLon, lat0, lon0, topDist);

        // Distance from bottom
        double bottomDist;
        Geodesic::WGS84().Inverse(mEndLatitude, mEndLongitude, lat0, lon0, bottomDist);

        bool inside = true;
        if (topDist > bottomDist)
        {
            if (topDist > mLaneLength)
            {
                // Point is after bottom
                inside = false;

                double woLeftLat, woLeftLon;

                // Draw first line of arrow
                Geodesic::WGS84().Direct(mEndLatitude, mEndLongitude, mBearing + 45, mLaneWidth, woLeftLat, woLeftLon);

                data.clear();

                QMap<QString, QVariant> val;
                val["lat"] = woLeftLat;
                val["lng"] = woLeftLon;
                data.push_back(val);

                splitLine(data, woLeftLat, woLeftLon, mEndLatitude, mEndLongitude, threshold, 0);

                val["lat"] = mEndLatitude;
                val["lng"] = mEndLongitude;
                data.push_back(val);

                // Draw second line of arrow
                Geodesic::WGS84().Direct(mEndLatitude, mEndLongitude, mBearing - 45, mLaneWidth, woLeftLat, woLeftLon);

                splitLine(data, mEndLatitude, mEndLongitude, woLeftLat, woLeftLon, threshold, 0);

                val["lat"] = woLeftLat;
                val["lng"] = woLeftLon;
                data.push_back(val);

                // Add to map
                view->mapCore()->addPolyline(data);
            }
        }
        else
        {
            if (bottomDist > mLaneLength)
            {
                // Point is before top
                inside = false;

                double woLeftLat, woLeftLon;

                // Draw first line of arrow
                Geodesic::WGS84().Direct(woProjLat, woProjLon, mBearing + 135, mLaneWidth, woLeftLat, woLeftLon);

                data.clear();

                QMap<QString, QVariant> val;
                val["lat"] = woLeftLat;
                val["lng"] = woLeftLon;
                data.push_back(val);

                splitLine(data, woLeftLat, woLeftLon, woProjLat, woProjLon, threshold, 0);

                val["lat"] = woProjLat;
                val["lng"] = woProjLon;
                data.push_back(val);

                // Draw second line of arrow
                Geodesic::WGS84().Direct(woProjLat, woProjLon, mBearing - 135, mLaneWidth, woLeftLat, woLeftLon);

                splitLine(data, woProjLat, woProjLon, woLeftLat, woLeftLon, threshold, 0);

                val["lat"] = woLeftLat;
                val["lng"] = woLeftLon;
                data.push_back(val);

                // Add to map
                view->mapCore()->addPolyline(data);
            }
        }

        if (inside)
        {
            // Draw finish line
            double woLeftLat, woLeftLon;
            Geodesic::WGS84().Direct(lat0, lon0, mBearing - 90, mLaneWidth, woLeftLat, woLeftLon);

            double woRightLat, woRightLon;
            Geodesic::WGS84().Direct(lat0, lon0, mBearing + 90, mLaneWidth, woRightLat, woRightLon);

            data.clear();

            QMap<QString, QVariant> val;
            val["lat"] = woLeftLat;
            val["lng"] = woLeftLon;
            data.push_back(val);

            splitLine(data, woLeftLat, woLeftLon, woRightLat, woRightLon, threshold, 0);

            val["lat"] = woRightLat;
            val["lng"] = woRightLon;
            data.push_back(val);

            // Add to map
            view->mapCore()->addPolyline(data);
        }
    }
    else
    {
        // Draw first line of 'X'
        double woLeftLat, woLeftLon;
        double woRightLat, woRightLon;

        Geodesic::WGS84().Direct(mEndLatitude, mEndLongitude, mBearing + 45, mLaneWidth, woLeftLat, woLeftLon);
        Geodesic::WGS84().Direct(mEndLatitude, mEndLongitude, mBearing + 225, mLaneWidth, woRightLat, woRightLon);

        data.clear();

        QMap<QString, QVariant> val;
        val["lat"] = woLeftLat;
        val["lng"] = woLeftLon;
        data.push_back(val);

        splitLine(data, woLeftLat, woLeftLon, woRightLat, woRightLon, threshold, 0);

        val["lat"] = woRightLat;
        val["lng"] = woRightLon;
        data.push_back(val);

        // Add to map
        view->mapCore()->addPolyline(data);

        // Draw second line of 'X'
        Geodesic::WGS84().Direct(mEndLatitude, mEndLongitude, mBearing - 45, mLaneWidth, woLeftLat, woLeftLon);
        Geodesic::WGS84().Direct(mEndLatitude, mEndLongitude, mBearing - 225, mLaneWidth, woRightLat, woRightLon);

        data.clear();

        val["lat"] = woLeftLat;
        val["lng"] = woLeftLon;
        data.push_back(val);

        splitLine(data, woLeftLat, woLeftLon, woRightLat, woRightLon, threshold, 0);

        val["lat"] = woRightLat;
        val["lng"] = woRightLon;
        data.push_back(val);

        // Add to map
        view->mapCore()->addPolyline(data);
    }
}

void WideOpenDistanceScoring::splitLine(
        QList<QVariant> &data,
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
        splitLine(data, startLat, startLon, midLat, midLon, threshold, depth + 1);

        QMap<QString, QVariant> val;
        val["lat"] = midLat;
        val["lng"] = midLon;
        data.push_back(val);

        splitLine(data, midLat, midLon, endLat, endLon, threshold, depth + 1);
    }
}

bool WideOpenDistanceScoring::getWindowBounds(
        const MainWindow::DataPoints &result,
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

bool WideOpenDistanceScoring::updateReference(
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

void WideOpenDistanceScoring::closeReference()
{
    setMapMode(None);
    mMainWindow->setFocus();
}
