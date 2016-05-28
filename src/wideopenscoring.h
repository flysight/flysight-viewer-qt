#ifndef WIDEOPENSCORING_H
#define WIDEOPENSCORING_H

#include "scoringmethod.h"

class MainWindow;

class WideOpenScoring : public ScoringMethod
{
public:
    WideOpenScoring(MainWindow *mainWindow);

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

    void prepareDataPlot(DataPlot *plot);

    bool getWindowBounds(const QVector< DataPoint > &result,
                         DataPoint &dpBottom, DataPoint &dpTop);

private:
    MainWindow *mMainWindow;

    double      mEndLatitude;
    double      mEndLongitude;

    double      mBearing;

    double      mBottom;
    double      mLaneWidth;
    double      mLaneLength;

signals:

public slots:
};

#endif // WIDEOPENSCORING_H
