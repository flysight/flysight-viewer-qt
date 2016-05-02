#ifndef SPEEDSCORING_H
#define SPEEDSCORING_H

#include "scoringmethod.h"

class MainWindow;

class SpeedScoring : public ScoringMethod
{
public:
    SpeedScoring(MainWindow *mainWindow);

    double score(const QVector< DataPoint > &result);
    QString scoreAsText(double score);

private:
    MainWindow *mMainWindow;

signals:

public slots:
};

#endif // SPEEDSCORING_H
