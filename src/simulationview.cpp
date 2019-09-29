#include "simulationview.h"
#include "ui_simulationview.h"

#include <QFile>
#include <QFileDialog>
#include <QSettings>
#include <QTextStream>

#include "mainwindow.h"

SimulationView::SimulationView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SimulationView),
    mMainWindow(0),
    mTone(mConfig),
    mUBX(mConfig, mTone)
{
    ui->setupUi(this);

    // Initialize configuration file name
    QSettings settings("FlySight", "Viewer");
    QString fileName = settings.value("configFolder").toString();
    ui->fileName->setText(fileName);
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

    // Reset the simulation
    reset();

    // Return now if there's no data
    if (mMainWindow->dataSize() == 0) return;

    const DataPoint &dpStart = mMainWindow->dataPoint(0);
    const DataPoint &dpEnd = mMainWindow->dataPoint(mMainWindow->dataSize() - 1);
    const qint64 numSamples = dpStart.dateTime.msecsTo(dpEnd.dateTime) * 8000 / 256;

    // Initialize progress bar
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);

    int iNextSample = 0;
    DataPoint dpNext = mMainWindow->dataPoint(iNextSample);
    qint64 msNextSample = dpStart.dateTime.msecsTo(dpNext.dateTime);
    qint64 msNextTick = 0;

    QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QFile file(desktop + "/temp.wav");
    file.open(QIODevice::ReadWrite);

    const unsigned char header[] =
    {
        0x52, 0x49, 0x46, 0x46, // ChunkID = "RIFF"
        0x1C, 0x0B, 0x00, 0x00, // ChunkSize
        0x57, 0x41, 0x56, 0x45, // Format = "WAVE"
        0x66, 0x6D, 0x74, 0x20, // Subchunk1ID = "fmt "
        0x10, 0x00, 0x00, 0x00, // Subchunk1Size
        0x01, 0x00,             // AudioFormat
        0x01, 0x00,             // NumChannels
        0x12, 0x7A, 0x00, 0x00, // SampleRate = 31250
        0x12, 0x7A, 0x00, 0x00, // ByteRate = 31250
        0x01, 0x00,             // BlockAlign
        0x08, 0x00,             // BitsPerSample
        0x64, 0x61, 0x74, 0x61  // Subchunk2ID = "data"
    };

    file.write((const char *) header, 40);

    uint32_t numSamples32 = (uint32_t) numSamples;
    file.write((char *) &numSamples32, 4);

    for (qint64 s = 0; s < numSamples; ++s)
    {
        qint64 ms = s * 256 / 8000;

        uint8_t sample = mTone.sample();
        file.write((char *) &sample, 1);

        if (ms >= msNextSample)
        {
            mUBX.receiveMessage(dpNext);

            dpNext = mMainWindow->dataPoint(++iNextSample);
            msNextSample = dpStart.dateTime.msecsTo(dpNext.dateTime);
        }

        if (ms >= msNextTick)
        {
            mTone.update();

            mUBX.task();
            mTone.task();

            ++msNextTick;
        }

        // Update progress bar
        ui->progressBar->setValue(static_cast<int>(100 * (s + 1) / numSamples));
    }

    file.close();
}

void SimulationView::reset()
{
    mTone.reset();
    mUBX.reset();
}
