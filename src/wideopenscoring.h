#ifndef WIDEOPENSCORING_H
#define WIDEOPENSCORING_H

#include "scoringmethod.h"

class MainWindow;

class WideOpenScoring : public ScoringMethod
{
public:
    WideOpenScoring(MainWindow *mainWindow);

    Mode mode() const { return mMode; }
    void setMode(Mode mode);

    double windowTop(void) const { return mWindowTop; }
    double windowBottom(void) const { return mWindowBottom; }
    void setWindow(double windowBottom, double windowTop);

    double score(const QVector< DataPoint > &result);
    QString scoreAsText(double score);

    void prepareDataPlot(DataPlot *plot);

    bool getWindowBounds(const QVector< DataPoint > &result,
                         DataPoint &dpBottom, DataPoint &dpTop);

    void optimize() { ScoringMethod::optimize(mMainWindow, mWindowBottom); }

private:
    MainWindow *mMainWindow;

    Mode        mMode;
    double      mWindowTop;
    double      mWindowBottom;

signals:

public slots:
};

#endif // WIDEOPENSCORING_H
