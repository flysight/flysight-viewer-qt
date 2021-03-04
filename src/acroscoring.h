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

#ifndef ACROSCORING_H
#define ACROSCORING_H

#include "scoringmethod.h"

class MainWindow;

class AcroScoring : public ScoringMethod
{
public:
    AcroScoring(MainWindow *mainWindow);

    double speed(void) const { return mSpeed; }
    void setSpeed(double speed);

    double altitude(void) const { return mAltitude; }
    void setAltitude(double altitude);

    void prepareDataPlot(DataPlot *plot);

    bool getWindowBounds(const MainWindow::DataPoints &result,
                         DataPoint &dpBottom, DataPoint &dpTop);

private:
    MainWindow *mMainWindow;

    double      mSpeed;
    double      mAltitude;

    int findIndexBelowT(const MainWindow::DataPoints &result,
                        double t);

signals:

public slots:
};

#endif // ACROSCORING_H
