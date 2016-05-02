#ifndef SPEEDSCORING_H
#define SPEEDSCORING_H

#include "scoringmethod.h"

class MainWindow;

class SpeedScoring : public ScoringMethod
{
public:
    SpeedScoring(MainWindow *mainWindow);

    double windowTop(void) const { return mWindowTop; }
    double windowBottom(void) const { return mWindowBottom; }
    void setWindow(double windowBottom, double windowTop);

    double score(const QVector< DataPoint > &result);
    QString scoreAsText(double score);

    void prepareDataPlot(DataPlot *plot);

    bool getWindowBounds(const QVector< DataPoint > &result,
                         DataPoint &dpBottom, DataPoint &dpTop);

private:
    MainWindow *mMainWindow;

    double      mWindowTop;
    double      mWindowBottom;

signals:

public slots:
};

#endif // SPEEDSCORING_H
