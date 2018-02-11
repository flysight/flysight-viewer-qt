#ifndef WIDEOPENDISTANCESCORING_H
#define WIDEOPENDISTANCESCORING_H

#include "scoringmethod.h"

class MainWindow;

class WideOpenDistanceScoring : public ScoringMethod
{
public:
    typedef enum {
        None, Start, End
    } MapMode;

    WideOpenDistanceScoring(MainWindow *mainWindow);

    double endLatitude(void) const { return mEndLatitude; }
    double endLongitude(void) const { return mEndLongitude; }
    void setEnd(double endLatitude, double endLongitude);

    double bearing(void) const { return mBearing; }
    void setBearing(double bearing);

    double bottom(void) const { return mBottom; }
    void setBottom(double bottom);

    double laneWidth(void) const { return mLaneWidth; }
    void setLaneWidth(double laneWidth);

    double laneLength(void) const { return mLaneLength; }
    void setLaneLength(double laneLength);

    void setMapMode(MapMode mode);

    void prepareDataPlot(DataPlot *plot);
    void prepareMapView(MapView *view);

    bool updateReference(double lat, double lon);
    void closeReference();

    bool getWindowBounds(const MainWindow::DataPoints &result,
                         DataPoint &dpBottom);

    void readSettings();
    void writeSettings();

private:
    MainWindow *mMainWindow;

    double      mEndLatitude;
    double      mEndLongitude;

    double      mBearing;

    double      mBottom;
    double      mLaneWidth;
    double      mLaneLength;

    MapMode     mMapMode;

    void splitLine(QVector< double > &lat, QVector< double > &lon,
                   double startLat, double startLon,
                   double endLat, double endLon,
                   double threshold, int depth);

signals:

public slots:
};

#endif // WIDEOPENDISTANCESCORING_H
