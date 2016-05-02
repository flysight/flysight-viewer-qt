#ifndef PPCSCORING_H
#define PPCSCORING_H

#include "scoringmethod.h"

class MainWindow;

class PPCScoring : public ScoringMethod
{
public:
    typedef enum {
        Time, Distance, Speed
    } Mode;

    PPCScoring(MainWindow *mainWindow);

    Mode mode() const { return mMode; }
    void setMode(Mode mode);

    double score(const QVector< DataPoint > &result);
    QString scoreAsText(double score);

    void prepareDataPlot(DataPlot *plot);

private:
    MainWindow *mMainWindow;
    Mode        mMode;

signals:

public slots:
};

#endif // PPCSCORING_H
