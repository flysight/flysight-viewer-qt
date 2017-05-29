#ifndef FLARESCORING_H
#define FLARESCORING_H

#include "scoringmethod.h"

class MainWindow;

class FlareScoring : public ScoringMethod
{
public:
    FlareScoring(MainWindow *mainWindow);

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

#endif // FLARESCORING_H
