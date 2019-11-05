#include "simulationview.h"
#include "ui_simulationview.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QSettings>
#include <QTextStream>

#include <VLCQtCore/Common.h>
#include <VLCQtCore/Instance.h>
#include <VLCQtCore/Media.h>
#include <VLCQtCore/MediaPlayer.h>

#include "mainwindow.h"

#define POSITION_DIV 10

SimulationView::SimulationView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SimulationView),
    mMainWindow(0),
    mMedia(0),
    mBusy(false)
{
    ui->setupUi(this);

    // Initialize file names
    QSettings settings("FlySight", "Viewer");
    QString fileName = settings.value("rootConfig").toString();
    ui->rootFileName->setText(fileName);

    fileName = settings.value("selectedConfig").toString();
    ui->selectedFileName->setText(fileName);

    fileName = settings.value("audioFolder").toString();
    ui->audioFolderName->setText(fileName);

    // Initialize check boxes
    bool checked = settings.value("useSelectedConfig").toBool();
    ui->selectedCheckBox->setChecked(checked);
    ui->selectedFileName->setEnabled(checked);
    ui->selectedBrowseButton->setEnabled(checked);

    checked = settings.value("useAudioFolder").toBool();
    ui->audioCheckBox->setChecked(checked);
    ui->audioFolderName->setEnabled(checked);
    ui->audioBrowseButton->setEnabled(checked);

    ui->playButton->setEnabled(false);
    ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(ui->playButton, SIGNAL(clicked()), this, SLOT(play()));

    ui->positionSlider->setEnabled(false);
    ui->positionSlider->setRange(0, 0);
    ui->positionSlider->setSingleStep(200 / POSITION_DIV);
    ui->positionSlider->setPageStep(2000 / POSITION_DIV);
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

    mAudioFile = new QTemporaryFile(QDir::temp().absoluteFilePath("FlySightViewer-XXXXXX.wav"));
    mAudioFile->open();
    mAudioFile->close();
    ui->outputFileName->setText(mAudioFile->fileName());
}

SimulationView::~SimulationView()
{
    delete mPlayer;
    delete mMedia;
    delete mInstance;
    delete mAudioFile;
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
    mBusy = true;

    pauseMedia();
    mMainWindow->mediaCursorRemoveRef();

    mBusy = false;
}

void SimulationView::on_rootBrowseButton_clicked()
{
    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get configuration file name
    QString settingsPath = settings.value("rootConfig").toString();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open configuration"), settingsPath);

    if (!fileName.isEmpty())
    {
        // Update file name
        ui->rootFileName->setText(fileName);

        // Remember last file read
        settings.setValue("rootConfig", QFileInfo(fileName).absoluteFilePath());
    }
}

void SimulationView::on_selectedBrowseButton_clicked()
{
    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get configuration file name
    QString settingsPath = settings.value("selectedConfig").toString();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open configuration"), settingsPath);

    if (!fileName.isEmpty())
    {
        // Update file name
        ui->selectedFileName->setText(fileName);

        // Remember last file read
        settings.setValue("selectedConfig", QFileInfo(fileName).absoluteFilePath());
    }
}

void SimulationView::on_audioBrowseButton_clicked()
{
    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get audio folder name
    QString settingsPath = settings.value("audioFolder").toString();
    QString dirName = QFileDialog::getExistingDirectory(this, tr("Open audio folder"), settingsPath);

    if (!dirName.isEmpty())
    {
        // Update file name
        ui->audioFolderName->setText(dirName);

        // Remember last file read
        settings.setValue("audioFolder", QFileInfo(dirName).absoluteFilePath());
    }
}

void SimulationView::on_selectedCheckBox_stateChanged(int state)
{
    const bool checked = state == Qt::Checked;

    ui->selectedFileName->setEnabled(checked);
    ui->selectedBrowseButton->setEnabled(checked);

    QSettings settings("FlySight", "Viewer");
    settings.setValue("useSelectedConfig", checked);
}

void SimulationView::on_audioCheckBox_stateChanged(int state)
{
    const bool checked = state == Qt::Checked;
    ui->audioFolderName->setEnabled(checked);
    ui->audioBrowseButton->setEnabled(checked);

    QSettings settings("FlySight", "Viewer");
    settings.setValue("useAudioFolder", checked);
}

void SimulationView::on_processButton_clicked()
{
    Config config;
    Tone   tone(config);
    UBX    ubx(config, tone);

    // Read root configuration
    QString fileName = ui->rootFileName->text();
    config.readSingle(fileName);

    // Read selected configuration
    if (ui->selectedCheckBox->isChecked())
    {
        QString fileName = ui->selectedFileName->text();
        config.readSingle(fileName);
    }

    // Copy audio folder name
    if (ui->audioCheckBox->isChecked())
    {
        config.mAudioFolder = ui->audioFolderName->text();
    }

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

    mAudioFile->open();
    mAudioFile->resize(0);

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

    mAudioFile->write((const char *) header, 40);

    uint32_t numSamples32 = (uint32_t) numSamples;
    mAudioFile->write((char *) &numSamples32, 4);

    for (qint64 s = 0; s < numSamples; ++s)
    {
        qint64 ms = s * 256 / 8000;

        uint8_t sample = tone.sample();
        mAudioFile->write((char *) &sample, 1);

        if (ms >= msNextSample)
        {
            ubx.receiveMessage(dpNext);

            dpNext = mMainWindow->dataPoint(++iNextSample);
            msNextSample = dpStart.dateTime.msecsTo(dpNext.dateTime);
        }

        if (ms >= msNextTick)
        {
            tone.update();

            ubx.task();
            tone.task();

            ++msNextTick;
        }

        // Update progress bar
        ui->progressBar->setValue(static_cast<int>(100 * (s + 1) / numSamples));
    }

    mAudioFile->close();

    setMedia(mAudioFile->fileName());
}

void SimulationView::setMedia(const QString &fileName)
{
    // Set media
    delete mMedia;
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
        mMainWindow->pauseMedia();
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
    ui->positionSlider->setValue(position / POSITION_DIV);
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
    ui->positionSlider->setRange(0, duration / POSITION_DIV);
}

void SimulationView::setPosition(int position)
{
    if (!mBusy)
    {
        // Update video position
        mPlayer->setTime(position * POSITION_DIV);
        timeChanged(position * POSITION_DIV);
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

void SimulationView::pauseMedia()
{
    mPlayer->pause();
}
