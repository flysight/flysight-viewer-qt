#ifndef SCORINGVIEW_H
#define SCORINGVIEW_H

#include <QWidget>

namespace Ui {
    class ScoringView;
}

class DataPoint;
class MainWindow;
class PPCForm;
class SpeedForm;

class ScoringView : public QWidget
{
    Q_OBJECT

public:
    explicit ScoringView(QWidget *parent = 0);
    ~ScoringView();

    void setMainWindow(MainWindow *mainWindow);

    double score(const QVector< DataPoint > &result);
    QString scoreAsText(double score);

private:
    Ui::ScoringView *ui;
    MainWindow      *mMainWindow;
    PPCForm         *mPPCForm;
    SpeedForm       *mSpeedForm;

public slots:
    void updateView();

private slots:
    void changePage(int page);
    void onActualButtonClicked();
    void onOptimalButtonClicked();
    void onOptimizeButtonClicked();
};

#endif // SCORINGVIEW_H
