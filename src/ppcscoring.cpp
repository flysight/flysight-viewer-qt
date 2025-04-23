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

#include "ppcscoring.h"

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

PPCScoring::PPCScoring(
        MainWindow *mainWindow):
    ScoringMethod(mainWindow),
    mMainWindow(mainWindow),
    mMode(Time),
    mWindowTop(2500),
    mWindowBottom(1500),
    mDrawLane(false),
    mEndLatitude(51.0500),
    mEndLongitude(-114.0667),
    mLaneWidth(600)
{

}

void PPCScoring::readSettings()
{
    QSettings settings("FlySight", "Viewer");

    settings.beginGroup("ppcScoring");
        mDrawLane     = settings.value("drawLane", mDrawLane).toBool();
        mEndLatitude  = settings.value("endLatitude", mEndLatitude).toDouble();
        mEndLongitude = settings.value("endLongitude", mEndLongitude).toDouble();
    settings.endGroup();
}

void PPCScoring::writeSettings()
{
    QSettings settings("FlySight", "Viewer");

    settings.beginGroup("ppcScoring");
        settings.setValue("drawLane", mDrawLane);
        settings.setValue("endLatitude", mEndLatitude);
        settings.setValue("endLongitude", mEndLongitude);
    settings.endGroup();
}

void PPCScoring::setDrawLane(
        bool drawLane)
{
    mDrawLane = drawLane;
    emit scoringChanged();
}

void PPCScoring::setEnd(
        double endLatitude,
        double endLongitude)
{
    mEndLatitude = endLatitude;
    mEndLongitude = endLongitude;
    emit scoringChanged();
}

void PPCScoring::setMode(
        Mode mode)
{
    mMode = mode;
    emit scoringChanged();
}

void PPCScoring::setWindow(
        double windowBottom,
        double windowTop)
{
    mWindowBottom = windowBottom;
    mWindowTop = windowTop;
    emit scoringChanged();
}

double PPCScoring::score(
        const MainWindow::DataPoints &result)
{
    DataPoint dpBottom, dpTop;
    if (getWindowBounds(result, dpBottom, dpTop))
    {
        switch (mMode)
        {
        case Time:
            return dpBottom.t - dpTop.t;
        case Distance:
            return mMainWindow->getDistance(dpTop, dpBottom);
        default: // Speed
            return mMainWindow->getDistance(dpTop, dpBottom) / (dpBottom.t - dpTop.t);
        }
    }

    return 0;
}

QString PPCScoring::scoreAsText(
        double score)
{
    switch (mMode)
    {
    case Time:
        return QString::number(score) + QString(" s");
    case Distance:
        return (mMainWindow->units() == PlotValue::Metric) ?
                    QString::number(score / 1000) + QString(" km"):
                    QString::number(score * METERS_TO_FEET / 5280) + QString(" mi");
    default: // Speed
        return (mMainWindow->units() == PlotValue::Metric) ?
                    QString::number(score * MPS_TO_KMH) + QString(" km/h"):
                    QString::number(score * MPS_TO_MPH) + QString(" mph");
    }
}

void PPCScoring::prepareDataPlot(
        DataPlot *plot)
{
    // Return now if plot empty
    if (mMainWindow->dataSize() == 0) return;

    DataPoint dpBottom, dpTop;
    bool success;

    switch (mMainWindow->windowMode())
    {
    case MainWindow::Actual:
        success = getWindowBounds(mMainWindow->data(), dpBottom, dpTop);
        break;
    case MainWindow::Optimal:
        success = getWindowBounds(mMainWindow->optimal(), dpBottom, dpTop);
        break;
    }

    // Add shading for scoring window
    if (success && plot->yValue(DataPlot::Elevation)->visible())
    {
        DataPoint dpLower = mMainWindow->interpolateDataT(mMainWindow->rangeLower());
        DataPoint dpUpper = mMainWindow->interpolateDataT(mMainWindow->rangeUpper());

        const double xMin = plot->xValue()->value(dpLower, mMainWindow->units());
        const double xMax = plot->xValue()->value(dpUpper, mMainWindow->units());

        QVector< double > xElev, yElev;

        xElev << xMin << xMax;
        yElev << plot->yValue(DataPlot::Elevation)->value(dpTop, mMainWindow->units())
              << plot->yValue(DataPlot::Elevation)->value(dpTop, mMainWindow->units());

        QCPGraph *graph = plot->addGraph(
                    plot->axisRect()->axis(QCPAxis::atBottom),
                    plot->yValue(DataPlot::Elevation)->axis());
        graph->setData(xElev, yElev);
        graph->setPen(QPen(QBrush(Qt::lightGray), mMainWindow->lineThickness(), Qt::DashLine));

        yElev.clear();
        yElev << plot->yValue(DataPlot::Elevation)->value(dpBottom, mMainWindow->units())
              << plot->yValue(DataPlot::Elevation)->value(dpBottom, mMainWindow->units());

        graph = plot->addGraph(
                    plot->axisRect()->axis(QCPAxis::atBottom),
                    plot->yValue(DataPlot::Elevation)->axis());
        graph->setData(xElev, yElev);
        graph->setPen(QPen(QBrush(Qt::lightGray), mMainWindow->lineThickness(), Qt::DashLine));

        QCPItemRect *rect = new QCPItemRect(plot);

        rect->setPen(QPen(QBrush(Qt::lightGray), mMainWindow->lineThickness(), Qt::DashLine));
        rect->setBrush(QColor(0, 0, 0, 8));

        rect->topLeft->setType(QCPItemPosition::ptAxisRectRatio);
        rect->topLeft->setAxes(plot->xAxis, plot->yValue(DataPlot::Elevation)->axis());
        rect->topLeft->setCoords(-0.1, -0.1);

        rect->bottomRight->setType(QCPItemPosition::ptAxisRectRatio);
        rect->bottomRight->setAxes(plot->xAxis, plot->yValue(DataPlot::Elevation)->axis());
        rect->bottomRight->setCoords(
                    (plot->xValue()->value(dpTop, mMainWindow->units()) - xMin) / (xMax - xMin),
                    1.1);

        rect = new QCPItemRect(plot);

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

void PPCScoring::prepareMapView(
        MapView *view)
{
    // Return now if plot empty
    if (mMainWindow->dataSize() == 0) return;

    if (!mDrawLane) return;

    // Distance threshold
    const double threshold = view->metersPerPixel();

    // Get start point
    DataPoint dpTenMS = mMainWindow->performanceStart();
    DataPoint dpStart = mMainWindow->interpolateDataT(dpTenMS.t + 9.0);

    // Draw lane center
    QList<QVariant> data;

    QMap<QString, QVariant> val;
    val["lat"] = mEndLatitude;
    val["lng"] = mEndLongitude;
    data.push_back(val);

    splitLine(data, mEndLatitude, mEndLongitude, dpStart.lat, dpStart.lon, threshold, 0);

    val["lat"] = dpStart.lat;
    val["lng"] = dpStart.lon;
    data.push_back(val);

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
}

void PPCScoring::splitLine(
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

bool PPCScoring::getWindowBounds(
        const MainWindow::DataPoints &result,
        DataPoint &dpBottom,
        DataPoint &dpTop)
{
    bool foundBottom = false;
    bool foundTop = false;
    int bottom, top;

    for (int i = result.size() - 1; i >= 0; --i)
    {
        const DataPoint &dp = result[i];

        if (dp.z < mWindowBottom)
        {
            bottom = i;
            foundBottom = true;
        }

        if (dp.z < mWindowTop)
        {
            top = i;
            foundTop = false;
        }

        if (dp.z > mWindowTop)
        {
            foundTop = true;
        }

        if (dp.t < 0) break;
    }

    if (foundBottom && foundTop)
    {
        // Calculate bottom of window
        const DataPoint &dp1 = result[bottom - 1];
        const DataPoint &dp2 = result[bottom];
        dpBottom = DataPoint::interpolate(dp1, dp2, (mWindowBottom - dp1.z) / (dp2.z - dp1.z));

        // Calculate top of window
        const DataPoint &dp3 = result[top - 1];
        const DataPoint &dp4 = result[top];
        dpTop = DataPoint::interpolate(dp3, dp4, (mWindowTop - dp3.z) / (dp4.z - dp3.z));

        return true;
    }
    else
    {
        return false;
    }
}

bool PPCScoring::updateReference(
        double lat,
        double lon)
{
    if (mMainWindow->mapMode() == MainWindow::SetEnd)
    {
        setEnd(lat, lon);
        return true;
    }
    else
    {
        return false;
    }
}

bool PPCScoring::getSEP(
    const MainWindow::DataPoints &result,
    double &sep)
{
    bool found = false;

    for (int i = result.size() - 1; i >= 0; --i)
    {
        // Get end point
        const DataPoint &dp = result[i];

        // Get validation window
        const double zBottom = mWindowBottom - 20;
        const double zTop = mWindowTop + 20;

        // Check window conditions
        if (dp.t < 0) break;
        if (dp.z < zBottom) continue;
        if (dp.z > zTop) continue;

        // Calculate accuracy
        double val = DataPoint::sep(dp);
        if ((!found) || (val > sep))
        {
            sep = val;
        }

        found = true;
    }

    return found;
}
