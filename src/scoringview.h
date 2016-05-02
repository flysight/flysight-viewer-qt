#ifndef SCORINGVIEW_H
#define SCORINGVIEW_H

#include <QWidget>

namespace Ui {
    class ScoringView;
}

class DataPlot;
class DataPoint;
class MainWindow;
class PerformanceForm;
class PPCForm;
class SpeedForm;

class ScoringView : public QWidget
{
    Q_OBJECT

public:
    explicit ScoringView(QWidget *parent = 0);
    ~ScoringView();

    void setMainWindow(MainWindow *mainWindow);

private:
    Ui::ScoringView *ui;
    MainWindow      *mMainWindow;
    PPCForm         *mPPCForm;
    SpeedForm       *mSpeedForm;
    PerformanceForm *mPerformanceForm;

public slots:
    void updateView();

private slots:
    void changePage(int page);
};

#endif // SCORINGVIEW_H
