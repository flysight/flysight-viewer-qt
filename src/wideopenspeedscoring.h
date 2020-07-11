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

#ifndef WIDEOPENSPEEDSCORING_H
#define WIDEOPENSPEEDSCORING_H

#include "scoringmethod.h"

class MainWindow;

class WideOpenSpeedScoring : public ScoringMethod
{
public:
    WideOpenSpeedScoring(MainWindow *mainWindow);

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
    void prepareMapView(MapView *view);

    bool updateReference(double lat, double lon);

    bool getWindowBounds(const MainWindow::DataPoints &result,
                         DataPoint &dpBottom);

    void readSettings();
    void writeSettings();

    void invalidateFinish();
    void setFinishPoint(const DataPoint &dp);

private:
    MainWindow *mMainWindow;

    double      mEndLatitude;
    double      mEndLongitude;

    double      mBearing;

    double      mBottom;
    double      mLaneWidth;
    double      mLaneLength;

    bool        mFinishValid;
    DataPoint   mFinishPoint;

    void splitLine(QList<QVariant> &data,
                   double startLat, double startLon,
                   double endLat, double endLon,
                   double threshold, int depth);

signals:

public slots:
};

#endif // WIDEOPENSPEEDSCORING_H
