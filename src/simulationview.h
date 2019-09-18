#ifndef SIMULATIONVIEW_H
#define SIMULATIONVIEW_H

#include <QWidget>

#include "config.h"

namespace Ui {
class SimulationView;
}

class MainWindow;

class SimulationView : public QWidget
{
    Q_OBJECT

public:
    explicit SimulationView(QWidget *parent = 0);
    ~SimulationView();

    void setMainWindow(MainWindow *mainWindow);

private:
    Ui::SimulationView *ui;
    MainWindow         *mMainWindow;

    Config mConfig;

private slots:
    void on_browseButton_clicked();
    void on_processButton_clicked();
};

#endif // SIMULATIONVIEW_H
