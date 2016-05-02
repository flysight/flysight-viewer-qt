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
