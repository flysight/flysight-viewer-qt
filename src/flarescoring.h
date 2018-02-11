#ifndef FLARESCORING_H
#define FLARESCORING_H

#include "scoringmethod.h"

class MainWindow;

class FlareScoring : public ScoringMethod
{
public:
    FlareScoring(MainWindow *mainWindow);

    double windowBottom(void) const { return mWindowBottom; }
    void setWindowBottom(double windowBottom);

    double score(const MainWindow::DataPoints &result);
    QString scoreAsText(double score);

    void prepareDataPlot(DataPlot *plot);

    bool getWindowBounds(const MainWindow::DataPoints &result,
                         DataPoint &dpBottom, DataPoint &dpTop);

    void optimize() { ScoringMethod::optimize(mMainWindow, mWindowBottom); }

private:
    MainWindow *mMainWindow;

    double      mWindowBottom;

signals:

public slots:
};

#endif // FLARESCORING_H
