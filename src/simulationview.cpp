#include "simulationview.h"
#include "ui_simulationview.h"

#include <QFile>
#include <QFileDialog>
#include <QSettings>
#include <QTextStream>

#include <VLCQtCore/Common.h>
#include <VLCQtCore/Instance.h>
#include <VLCQtCore/Media.h>
#include <VLCQtCore/MediaPlayer.h>

#include "mainwindow.h"

SimulationView::SimulationView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SimulationView),
    mMainWindow(0),
    mTone(mConfig),
    mUBX(mConfig, mTone),
    mMedia(0),
    mBusy(false)
{
    ui->setupUi(this);

    // Initialize configuration file name
    QSettings settings("FlySight", "Viewer");
    QString fileName = settings.value("configFolder").toString();
    ui->fileName->setText(fileName);

    ui->playButton->setEnabled(false);
    ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(ui->playButton, SIGNAL(clicked()), this, SLOT(play()));

    ui->positionSlider->setEnabled(false);
    ui->positionSlider->setRange(0, 0);
    ui->positionSlider->setSingleStep(200);
    ui->positionSlider->setPageStep(2000);
    connect(ui->positionSlider, SIGNAL(valueChanged(int)), this, SLOT(setPosition(int)));

    ui->scrubDial->setEnabled(false);
    ui->scrubDial->setRange(0, 1000);
    ui->scrubDial->setSingleStep(30);
    ui->scrubDial->setPageStep(300);
    connect(ui->scrubDial, SIGNAL(valueChanged(int)), this, SLOT(setScrubPosition(int)));

    mInstance = new VlcInstance(VlcCommon::args(), this);
    mPlayer = new VlcMediaPlayer(mInstance);

    connect(mPlayer, SIGNAL(stateChanged()), this, SLOT(stateChanged()));
    connect(mPlayer, SIGNAL(timeChanged(int)), this, SLOT(timeChanged(int)));
    connect(mPlayer, SIGNAL(lengthChanged(int)), this, SLOT(lengthChanged(int)));
}

SimulationView::~SimulationView()
{
    delete mPlayer;
    delete mMedia;
    delete mInstance;
    delete ui;
}

QSize SimulationView::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(400, 139);
}

void SimulationView::setMainWindow(
        MainWindow *mainWindow)
{
    mMainWindow = mainWindow;
}

void SimulationView::showEvent(
        QShowEvent *event)
{
    mMainWindow->mediaCursorAddRef();
}

void SimulationView::hideEvent(
        QHideEvent *event)
{
    mMainWindow->mediaCursorRemoveRef();
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
    mTone.reset();
    mUBX.reset();

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

    setMedia(desktop + "/temp.wav");
}

void SimulationView::setMedia(const QString &fileName)
{
    // Set media
    mMedia = new VlcMedia(fileName, true, mInstance);
    mPlayer->open(mMedia);

    // Update buttons
    ui->playButton->setEnabled(true);
    ui->positionSlider->setEnabled(true);
    ui->scrubDial->setEnabled(true);
}

void SimulationView::play()
{
    switch(mPlayer->state())
    {
    case Vlc::Playing:
        mPlayer->pause();
        break;
    default:
        mPlayer->play();
        break;
    }
}

void SimulationView::stateChanged()
{
    switch(mPlayer->state())
    {
    case Vlc::Playing:
        ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        break;
    default:
        ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        break;
    }
}

void SimulationView::timeChanged(int position)
{
    mBusy = true;

    // Update controls
    ui->positionSlider->setValue(position);
    ui->scrubDial->setValue(position % 1000);

    // Update text label
    double time = (double) position / 1000;

    if (mMainWindow->dataSize() > 0)
    {
        const DataPoint &dp0 = mMainWindow->data()[0];
        time += dp0.t;
    }

    ui->timeLabel->setText(QString("%1 s").arg(time, 0, 'f', 3));

    // Update other views
    mMainWindow->setMediaCursor(time);

    mBusy = false;
}

void SimulationView::lengthChanged(int duration)
{
    ui->positionSlider->setRange(0, duration);
}

void SimulationView::setPosition(int position)
{
    if (!mBusy)
    {
        // Update video position
        mPlayer->setTime(position);
        timeChanged(position);
    }
}

void SimulationView::setScrubPosition(int position)
{
    if (!mBusy)
    {
        int oldPosition = mPlayer->time();
        int newPosition = oldPosition - oldPosition % 1000 + position;

        while (newPosition <= oldPosition - 500) newPosition += 1000;
        while (newPosition >  oldPosition + 500) newPosition -= 1000;

        // Update video position
        mPlayer->setTime(newPosition);
        timeChanged(newPosition);
    }
}

void SimulationView::updateView()
{
    if (mBusy) return;

    // Get media cursor
    const DataPoint &dp = mMainWindow->interpolateDataT(mMainWindow->mediaCursor());
    const DataPoint &dp0 = mMainWindow->data()[0];

    // Get playback position
    int position = (dp.t - dp0.t) * 1000;

    // If playback position is within video bounds
    if (0 <= position && position <= mPlayer->length())
    {
        // Update video position
        mPlayer->setTime(position);
        timeChanged(position);
    }
}
