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

    QString flightPath("var flightPlanCoordinates = [");

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

            flightPath += QString("new google.maps.LatLng(%1, %2),").arg(dp.lat, 0, 'f').arg(dp.lon, 0, 'f');
        }
    }

    flightPath = flightPath.left(flightPath.size() - 1);
    flightPath += QString("];");

    flightPath += QString("var flightPath = new google.maps.Polyline({");
    flightPath += QString("  path: flightPlanCoordinates,");
    flightPath += QString("  geodesic: true,");
    flightPath += QString("  strokeColor: '#FF0000',");
    flightPath += QString("  strokeOpacity: 1.0,");
    flightPath += QString("  strokeWeight: 2");
    flightPath += QString("});");

    flightPath += QString("flightPath.setMap(map);");
    page()->currentFrame()->documentElement().evaluateJavaScript(flightPath);

    QString str =
            QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg((yMin + yMax) / 2).arg((xMin + xMax) / 2) +
            QString("map.setCenter(newLoc);");

    page()->currentFrame()->documentElement().evaluateJavaScript(str);
}
