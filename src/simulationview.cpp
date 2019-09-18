#include "simulationview.h"
#include "ui_simulationview.h"

#include <QFileDialog>
#include <QSettings>

#include "mainwindow.h"

SimulationView::SimulationView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SimulationView),
    mMainWindow(0)
{
    ui->setupUi(this);
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

        // Remember last file read
        settings.setValue("configFolder", QFileInfo(fileName).absoluteFilePath());
    }
}

void SimulationView::on_processButton_clicked()
{
    // Reset configuration
    mConfig.reset();

    // Get file name
    QString fileName = ui->fileName->text();

    if (!fileName.isEmpty())
    {
        // Read the file
        mConfig.readSingle(fileName);
    }

    ui->progressBar->setRange(0, mMainWindow->dataSize());
    for (int i = 0; i < mMainWindow->dataSize(); ++i)
    {
        const DataPoint &dp = mMainWindow->dataPoint(i);

        // Update progress bar
        ui->progressBar->setValue(i + 1);
    }
}
