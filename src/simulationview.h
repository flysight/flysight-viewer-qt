#ifndef SIMULATIONVIEW_H
#define SIMULATIONVIEW_H

#include <QWidget>

#include "config.h"
#include "tone.h"
#include "ubx.h"

namespace Ui {
class SimulationView;
}

class DataPoint;
class MainWindow;

class SimulationView : public QWidget
{
    Q_OBJECT

public:
    explicit SimulationView(QWidget *parent = 0);
    ~SimulationView();

    void setMainWindow(MainWindow *mainWindow);

    const Config &config() const { return mConfig; }

private:
    Ui::SimulationView *ui;
    MainWindow         *mMainWindow;

    Config mConfig;
    Tone mTone;
    UBX mUBX;

    void reset();

private slots:
    void on_browseButton_clicked();
    void on_processButton_clicked();
};

#endif // SIMULATIONVIEW_H
