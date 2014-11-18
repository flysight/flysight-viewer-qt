#include "videoview.h"
#include "ui_videoview.h"

#include <QDir>
#include <QFileDialog>
#include <QMediaPlayer>
#include <QMediaPlaylist>

#include "common.h"
#include "mainwindow.h"

VideoView::VideoView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoView),
    mMainWindow(0),
    mZeroPosition(0),
    mBlockUpdate(false)
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

    mPlayer.setVideoOutput(ui->videoWidget);
    connect(&mPlayer, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(mediaStateChanged(QMediaPlayer::State)));
    connect(&mPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
    connect(&mPlayer, SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));
}

VideoView::~VideoView()
{
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
        mPlayer.setMedia(QUrl::fromLocalFile(fileName));

        // Update buttons
        ui->playButton->setEnabled(true);
        ui->zeroButton->setEnabled(true);
        ui->positionSlider->setEnabled(true);
        ui->scrubDial->setEnabled(true);

        // Update display
        mPlayer.play();
        mPlayer.pause();
    }
}

void VideoView::play()
{
    switch(mPlayer.state())
    {
    case QMediaPlayer::PlayingState:
        mPlayer.pause();
        break;
    default:
        mPlayer.play();
        break;
    }
}

void VideoView::mediaStateChanged(QMediaPlayer::State state)
{
    switch(state)
    {
    case QMediaPlayer::PlayingState:
        ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        break;
    default:
        ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        break;
    }

    // Update position sliders
    qint64 position = mPlayer.position();
    ui->positionSlider->setValue(position);
    ui->scrubDial->setValue(position % 1000);

    // Update text label
    double time = (double) (position - mZeroPosition) / 1000;
    ui->timeLabel->setText(QString("%1 s").arg(time, 0, 'f', 3));

    // Update other views
    mMainWindow->setMark(time);
}

void VideoView::positionChanged(qint64 position)
{
    if (mPlayer.state() == QMediaPlayer::PlayingState)
    {
        mBlockUpdate = true;

        // Update controls
        ui->positionSlider->setValue(position);
        ui->scrubDial->setValue(position % 1000);

        // Update text label
        double time = (double) (position - mZeroPosition) / 1000;
        ui->timeLabel->setText(QString("%1 s").arg(time, 0, 'f', 3));

        // Update other views
        mMainWindow->setMark(time);

        mBlockUpdate = false;
    }
}

void VideoView::durationChanged(qint64 duration)
{
    ui->positionSlider->setRange(0, duration);
}

void VideoView::setPosition(int position)
{
    if (!mBlockUpdate &&
            mPlayer.state() != QMediaPlayer::PlayingState)
    {
        mBlockUpdate = true;

        // Update video position
        mPlayer.setPosition(position);

        // Update scrub control
        ui->scrubDial->setValue(position % 1000);

        // Update text label
        double time = (double) (position - mZeroPosition) / 1000;
        ui->timeLabel->setText(QString("%1 s").arg(time, 0, 'f', 3));

        // Update other views
        mMainWindow->setMark(time);

        mBlockUpdate = false;
    }
}

void VideoView::setScrubPosition(int position)
{
    if (!mBlockUpdate &&
            mPlayer.state() != QMediaPlayer::PlayingState)
    {
        mBlockUpdate = true;

        int oldPosition = mPlayer.position();
        int newPosition = oldPosition - oldPosition % 1000 + position;

        while (newPosition <= oldPosition - 500) newPosition += 1000;
        while (newPosition >  oldPosition + 500) newPosition -= 1000;

        // Update video position
        mPlayer.setPosition(newPosition);

        // Update position control
        ui->positionSlider->setValue(newPosition);

        // Update text label
        double time = (double) (newPosition - mZeroPosition) / 1000;
        ui->timeLabel->setText(QString("%1 s").arg(time, 0, 'f', 3));

        // Update other views
        mMainWindow->setMark(time);

        mBlockUpdate = false;
    }
}

void VideoView::zero()
{
    mZeroPosition = mPlayer.position();

    // Update text label
    double time = (double) (mZeroPosition - mZeroPosition) / 1000;
    ui->timeLabel->setText(QString("%1 s").arg(time, 0, 'f', 3));
}

void VideoView::updateView()
{
    if (!mBlockUpdate &&
            mMainWindow->markActive() &&
            mPlayer.state() != QMediaPlayer::PlayingState)
    {
        mBlockUpdate = true;

        // Get marked point
        const DataPoint &dpEnd = mMainWindow->interpolateDataT(mMainWindow->markEnd());

        // Set playback position
        qint64 position = dpEnd.t * 1000 + mZeroPosition;

        // Clamp to video bounds
        if (position < 0) position = 0;
        if (position >= mPlayer.duration()) position = mPlayer.duration() - 1;

        // Update video position
        mPlayer.setPosition(position);

        // Update text label
        double time = (double) (position - mZeroPosition) / 1000;
        ui->timeLabel->setText(QString("%1 s").arg(time, 0, 'f', 3));

        // Update controls
        ui->positionSlider->setValue(position);
        ui->scrubDial->setValue(position % 1000);

        mBlockUpdate = false;
    }
}
