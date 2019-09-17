#include "simulationview.h"
#include "ui_simulationview.h"

SimulationView::SimulationView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SimulationView),
    mMainWindow(0)
{
    ui->setupUi(this);

    ui->reloadButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
}

SimulationView::~SimulationView()
{
    delete ui;
}

void SimulationView::setMainWindow(
        MainWindow *mainWindow)
{
    mMainWindow = mainWindow;
}
