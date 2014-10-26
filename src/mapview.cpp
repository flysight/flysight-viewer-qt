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

    QString js;
    js = QString("var path = poly.getPath();") +
         QString("while (path.length > 0) { path.pop(); }");

    for (int i = 0; i < mMainWindow->dataSize(); ++i)
    {
        const DataPoint &dp = mMainWindow->dataPoint(i);

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

    //
    // TODO: Manage flight path better
    //

    js = QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg((yMin + yMax) / 2).arg((xMin + xMax) / 2) +
         QString("map.setCenter(newLoc);");

    page()->currentFrame()->documentElement().evaluateJavaScript(js);
}
