#include "simulationview.h"
#include "ui_simulationview.h"

#include <QFileDialog>
#include <QSettings>

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

void SimulationView::on_browseButton_clicked()
{
    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get last file read
    QString rootFolder = settings.value("configFolder").toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open configuration"), rootFolder);

    if (!fileName.isEmpty())
    {
        // Update file name
        ui->fileName->setText(fileName);

        // Reset configuration
        mConfig.reset();

        // Read the file
        mConfig.readSingle(fileName);
    }
}

void SimulationView::on_reloadButton_clicked()
{
    QString fileName = ui->fileName->text();

    if (!fileName.isEmpty())
    {
        // Reset configuration
        mConfig.reset();

        // Read the file
        mConfig.readSingle(fileName);
    }
}
