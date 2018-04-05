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

#ifndef PERFORMANCESCORING_H
#define PERFORMANCESCORING_H

#include "scoringmethod.h"

class MainWindow;

class PerformanceScoring : public ScoringMethod
{
public:
    PerformanceScoring(MainWindow *mainWindow);

    double startTime(void) const { return mStartTime; }
    double endTime(void) const { return mEndTime; }
    void setRange(double startTime, double endTime);

    void prepareDataPlot(DataPlot *plot);

private:
    MainWindow *mMainWindow;

    double      mStartTime;
    double      mEndTime;

signals:

public slots:
};

#endif // PERFORMANCESCORING_H
