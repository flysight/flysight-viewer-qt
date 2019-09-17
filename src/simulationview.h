#ifndef SIMULATIONVIEW_H
#define SIMULATIONVIEW_H

#include <QWidget>

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
};

#endif // SIMULATIONVIEW_H
