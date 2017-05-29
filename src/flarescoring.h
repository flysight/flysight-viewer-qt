#ifndef FLARESCORING_H
#define FLARESCORING_H

#include "scoringmethod.h"

class MainWindow;

class FlareScoring : public ScoringMethod
{
public:
    FlareScoring(MainWindow *mainWindow);

    double score(const QVector< DataPoint > &result);
    QString scoreAsText(double score);

    void prepareDataPlot(DataPlot *plot);

    bool getWindowBounds(const QVector< DataPoint > &result,
                         DataPoint &dpBottom, DataPoint &dpTop);

    void optimize();

private:
    MainWindow *mMainWindow;

signals:

public slots:
};

#endif // FLARESCORING_H
