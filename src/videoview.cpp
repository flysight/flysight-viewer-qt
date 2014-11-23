#include "videoview.h"
#include "ui_videoview.h"

#include <QDir>
#include <QFileDialog>

#include <vlc-qt/Common.h>
#include <vlc-qt/Instance.h>
#include <vlc-qt/Media.h>
#include <vlc-qt/MediaPlayer.h>

#include "common.h"
#include "mainwindow.h"

VideoView::VideoView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoView),
    mMainWindow(0),
    mZeroPosition(0),
    mBusy(false),
    mMedia(0)
{
    ui->setupUi(this);

    connect(ui->openButton, SIGNAL(clicked()), this, SLOT(openFile()));

    ui->playButton->setEnabled(false);
    ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(ui->playButton, SIGNAL(clicked()), this, SLOT(play()));

    ui->zeroButton->setEnabled(false);
    connect(ui->zeroButton, SIGNAL(clicked()), this, SLOT(zero()));

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
    mPlayer->setVideoWidget(ui->videoWidget);

    connect(mPlayer, SIGNAL(stateChanged()), this, SLOT(stateChanged()));
    connect(mPlayer, SIGNAL(timeChanged(int)), this, SLOT(timeChanged(int)));
    connect(mPlayer, SIGNAL(lengthChanged(int)), this, SLOT(lengthChanged(int)));
}

VideoView::~VideoView()
{
    delete mPlayer;
    delete mMedia;
    delete mInstance;
    delete ui;
}

QSize VideoView::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void VideoView::openFile()
{
    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get last file read
    QString rootFolder = settings.value("videoFolder").toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Video"), rootFolder);

    if (!fileName.isEmpty())
    {
        // Remember last file read
        settings.setValue("videoFolder", QFileInfo(fileName).absoluteFilePath());

        // Set media
        mMedia = new VlcMedia(fileName, true, mInstance);
        mPlayer->open(mMedia);

        // Update buttons
        ui->playButton->setEnabled(true);
        ui->zeroButton->setEnabled(true);
        ui->positionSlider->setEnabled(true);
        ui->scrubDial->setEnabled(true);
    }
}

void VideoView::play()
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

void VideoView::stateChanged()
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

void VideoView::timeChanged(int position)
{
    mBusy = true;

    // Update controls
    ui->positionSlider->setValue(position);
    ui->scrubDial->setValue(position % 1000);

    // Update text label
    double time = (double) (position - mZeroPosition) / 1000;
    ui->timeLabel->setText(QString("%1 s").arg(time, 0, 'f', 3));

    // Update other views
    mMainWindow->setMark(time);

    mBusy = false;
}

void VideoView::lengthChanged(int duration)
{
    ui->positionSlider->setRange(0, duration);
}

void VideoView::setPosition(int position)
{
    if (!mBusy)
    {
        // Update video position
        mPlayer->setTime(position);
        timeChanged(position);
    }
}

void VideoView::setScrubPosition(int position)
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

void VideoView::zero()
{
    mZeroPosition = mPlayer->time();

    // Update text label
    double time = (double) (mZeroPosition - mZeroPosition) / 1000;
    ui->timeLabel->setText(QString("%1 s").arg(time, 0, 'f', 3));
}

void VideoView::updateView()
{
    if (!mBusy && mMainWindow->markActive())
    {
        // Get marked point
        const DataPoint &dpEnd = mMainWindow->interpolateDataT(mMainWindow->markEnd());

        // Set playback position
        int position = dpEnd.t * 1000 + mZeroPosition;

        // Clamp to video bounds
        if (position < 0) position = 0;
        if (position >= mPlayer->length()) position = mPlayer->length() - 1;

        // Update video position
        mPlayer->setTime(position);
        timeChanged(position);
    }
}
