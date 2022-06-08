/***************************************************************************
**                                                                        **
**  FlySight Viewer                                                       **
**  Copyright 2020 Michael Cooper                                         **
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

#include "mapview.h"

#include <QList>
#include <QMap>
#include <QVariant>
#include <QWebChannel>

#include "common.h"
#include "mainwindow.h"
#include "mapcore.h"
#include "secrets.h"

MapView::MapView(QWidget *parent) :
    QWebEngineView(parent),
    mMainWindow(0),
    mDragging(false)
{
    QFile file(":/html/mapview.html");
    if (file.open(QIODevice::ReadOnly))
    {
        QString html = file.readAll();
        html.replace("GOOGLE_MAPS_API_KEY", GOOGLE_MAPS_API_KEY);
        setHtml(html);
    }

    // Set up the channel
    mWebChannel = new QWebChannel(this);

    // Set up the core and publish it to the QWebChannel
    mMapCore = new MapCore(this);
    mWebChannel->registerObject("core", mMapCore);

    // Associate the QWebChannel with the page
    page()->setWebChannel(mWebChannel);
}

QSize MapView::sizeHint() const
{
    // Keeps windows from being initialized as very short
    return QSize(175, 175);
}

void MapView::mouseDown(
        QMap<QString, QVariant> latLng)
{
    if (updateReference(latLng))
    {
        mMainWindow->clearMark();
        mDragging = true;
    }
}

void MapView::mouseUp(
        QMap<QString, QVariant> latLng)
{
    if (mDragging)
    {
        updateMarker(latLng);
        mMainWindow->setMapMode(MainWindow::Default);
        mDragging = false;
    }
}

void MapView::mouseOver(
        QMap<QString, QVariant> latLng)
{
    mouseMove(latLng);
}

void MapView::mouseOut()
{
    mMainWindow->clearMark();
}

void MapView::mouseMove(
        QMap<QString, QVariant> latLng)
{
    if (mDragging)
    {
        updateReference(latLng);
    }
    else
    {
        updateMarker(latLng);
    }
}

bool MapView::updateReference(
        QMap<QString, QVariant> latLng)
{
    double lat = latLng["lat"].toDouble();
    double lng = latLng["lng"].toDouble();

    // Pass to main window
    return mMainWindow->updateReference(lat, lng);
}

void MapView::updateMarker(
        QMap<QString, QVariant> latLng)
{
    double lat = latLng["lat"].toDouble();
    double lng = latLng["lng"].toDouble();

    QPointF pos = QPointF(width() * (lng - mLonMin) / (mLonMax - mLonMin),
                          height() * (mLatMax - lat) / (mLatMax - mLatMin));

    double lower = mMainWindow->rangeLower();
    double upper = mMainWindow->rangeUpper();

    double resultTime;
    double resultDistance = std::numeric_limits<double>::max();

    for (int i = 0; i + 1 < mMainWindow->dataSize(); ++i)
    {
        const DataPoint &dp1 = mMainWindow->dataPoint(i);
        const DataPoint &dp2 = mMainWindow->dataPoint(i + 1);

        if (lower <= dp1.t && dp1.t <= upper &&
            lower <= dp2.t && dp2.t <= upper)
        {
            QPointF pt1 = QPointF(width() * (dp1.lon - mLonMin) / (mLonMax - mLonMin),
                                  height() * (mLatMax - dp1.lat) / (mLatMax - mLatMin));
            QPointF pt2 = QPointF(width() * (dp2.lon - mLonMin) / (mLonMax - mLonMin),
                                  height() * (mLatMax - dp2.lat) / (mLatMax - mLatMin));

            double mu;
            double dist = sqrt(distSqrToLine(pt1, pt2, pos, mu));

            if (dist < resultDistance)
            {
                double t1 = dp1.t;
                double t2 = dp2.t;

                resultTime = t1 + mu * (t2 - t1);
                resultDistance = dist;
            }
        }
    }

    const int selectionTolerance = 8;
    if (resultDistance < selectionTolerance)
    {
        mMainWindow->setMark(resultTime);
    }
    else
    {
        mMainWindow->clearMark();
    }
}

void MapView::boundsChanged(
        QMap<QString, QVariant> sw,
        QMap<QString, QVariant> ne)
{
    double oldScale = metersPerPixel();

    mLatMin = sw["lat"].toDouble();
    mLonMin = sw["lng"].toDouble();
    mLatMax = ne["lat"].toDouble();
    mLonMax = ne["lng"].toDouble();

    if (oldScale != metersPerPixel())
    {
        updateView();
    }
}

void MapView::initView()
{
    double xMin, xMax;
    double yMin, yMax;

    for (int i = 0; i < mMainWindow->dataSize(); ++i)
    {
        const DataPoint &dp = mMainWindow->dataPoint(i);

        if (i == 0)
        {
            xMin = xMax = dp.lon;
            yMin = yMax = dp.lat;
        }
        else
        {
            if (dp.lon < xMin) xMin = dp.lon;
            if (dp.lon > xMax) xMax = dp.lon;

            if (dp.lat < yMin) yMin = dp.lat;
            if (dp.lat > yMax) yMax = dp.lat;
        }
    }

    QMap<QString, QVariant> sw;
    sw["lat"] = yMin;
    sw["lng"] = xMin;

    QMap<QString, QVariant> ne;
    ne["lat"] = yMax;
    ne["lng"] = xMax;

    mMapCore->setBounds(sw, ne);
}

void MapView::updateView()
{
    double lower = mMainWindow->rangeLower();
    double upper = mMainWindow->rangeUpper();

    // Distance threshold
    const double threshold = metersPerPixel();

    QList<QVariant> data;

    double distPrev;
    for (int i = 0; i < mMainWindow->dataSize(); ++i)
    {
        const DataPoint &dp = mMainWindow->dataPoint(i);

        if (i > 0 && dp.dist2D - distPrev < threshold) continue;
        distPrev = dp.dist2D;

        if (lower <= dp.t && dp.t <= upper)
        {
            QMap<QString, QVariant> val;
            val["lat"] = dp.lat;
            val["lng"] = dp.lon;

            data.push_back(val);
        }
    }

    mMapCore->setData(data);

    updateCursor();

    // Clear all annotations
    mMapCore->clearAnnotations();

    // Draw annotations on map
    mMainWindow->prepareMapView(this);
}

void MapView::updateCursor()
{
    if (mMainWindow->dataSize() == 0) return;

    if (mMainWindow->markActive())
    {
        // Add marker to map
        const DataPoint &dpEnd = mMainWindow->interpolateDataT(mMainWindow->markEnd());

        QMap<QString, QVariant> latLng;
        latLng["lat"] = dpEnd.lat;
        latLng["lng"] = dpEnd.lon;

        mMapCore->setMark(latLng);
    }
    else
    {
        // Clear marker
        mMapCore->clearMark();
    }

    if (mMainWindow->mediaCursorRef() > 0)
    {
        // Add media cursor to map
        const DataPoint &dp = mMainWindow->interpolateDataT(mMainWindow->mediaCursor());

        QMap<QString, QVariant> latLng;
        latLng["lat"] = dp.lat;
        latLng["lng"] = dp.lon;

        mMapCore->setMediaCursor(latLng);
    }
    else
    {
        // Clear media cursor
        mMapCore->clearMediaCursor();
    }
}

void MapView::updateMapMode()
{
    if (mMainWindow->mapMode() == MainWindow::Default)
    {
        mMapCore->enableDrag();
    }
    else
    {
        mMapCore->disableDrag();
    }
}

double MapView::metersPerPixel() const
{
    const double earthCircumference = 40075000; // m
    return earthCircumference * (mLatMax - mLatMin) / 360. / height();
}
