#include "mapview.h"

#include <QWebFrame>
#include <QWebElement>

#include "mainwindow.h"

MapView::MapView(QWidget *parent) :
    QWebView(parent),
    mMainWindow(0)
{
    setUrl(QUrl("qrc:/html/mapview.html"));
}

QSize MapView::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void MapView::updateView()
{
    double lower = mMainWindow->rangeLower();
    double upper = mMainWindow->rangeUpper();

    double xMin, xMax;
    double yMin, yMax;

    bool first = true;

    // Distance threshold
    const double earthCircumference = 40075000; // m
    const double zoom = page()->currentFrame()->documentElement().evaluateJavaScript("map.getZoom();").toDouble();
    const double threshold = earthCircumference / pow(2, zoom) / width();

    // Add track to map
    QString js = QString("var path = poly.getPath();") +
                 QString("while (path.length > 0) { path.pop(); }");

    double distPrev;
    for (int i = 0; i < mMainWindow->dataSize(); ++i)
    {
        const DataPoint &dp = mMainWindow->dataPoint(i);

        if (i > 0 && dp.dist2D - distPrev < threshold) continue;
        distPrev = dp.dist2D;

        if (lower <= dp.t && dp.t <= upper)
        {
            if (first)
            {
                xMin = xMax = dp.lon;
                yMin = yMax = dp.lat;
                first = false;
            }
            else
            {
                if (dp.lon < xMin) xMin = dp.lon;
                if (dp.lon > xMax) xMax = dp.lon;

                if (dp.lat < yMin) yMin = dp.lat;
                if (dp.lat > yMax) yMax = dp.lat;
            }

            js += QString("path.push(new google.maps.LatLng(%1, %2));").arg(dp.lat, 0, 'f').arg(dp.lon, 0, 'f');
        }
    }

    page()->currentFrame()->documentElement().evaluateJavaScript(js);

    if (mMainWindow->markActive())
    {
        // Add marker to map
        const DataPoint &dpEnd = mMainWindow->interpolateDataT(mMainWindow->markEnd());

        js = QString("marker.setPosition(new google.maps.LatLng(%1, %2));").arg(dpEnd.lat, 0, 'f').arg(dpEnd.lon, 0, 'f') +
             QString("marker.setVisible(true);");

        page()->currentFrame()->documentElement().evaluateJavaScript(js);
    }
    else
    {
        // Clear marker
        js = QString("marker.setVisible(false);");

        page()->currentFrame()->documentElement().evaluateJavaScript(js);
    }

    // Resize map
    js = QString("var bounds = new google.maps.LatLngBounds();") +
         QString("bounds.extend(new google.maps.LatLng(%1, %2));").arg(yMin).arg(xMin) +
         QString("bounds.extend(new google.maps.LatLng(%1, %2));").arg(yMax).arg(xMax) +
         QString("map.fitBounds(bounds);");

    page()->currentFrame()->documentElement().evaluateJavaScript(js);
}
