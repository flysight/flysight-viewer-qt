#include "playbackview.h"
#include "ui_playbackview.h"

#include "common.h"
#include "mainwindow.h"

PlaybackView::PlaybackView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlaybackView),
    mMainWindow(0),
    mBusy(false)
{
    ui->setupUi(this);

    ui->playButton->setEnabled(false);
    ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(ui->playButton, SIGNAL(clicked()), this, SLOT(play()));

    ui->positionSlider->setEnabled(false);
    ui->positionSlider->setRange(0, 0);
    ui->positionSlider->setSingleStep(200);
    ui->positionSlider->setPageStep(2000);
    connect(ui->positionSlider, SIGNAL(valueChanged(int)), this, SLOT(setPosition(int)));

    updateView();
}

PlaybackView::~PlaybackView()
{
    delete ui;
}

QSize PlaybackView::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(300, 75);
}

void PlaybackView::play()
{
/*    switch(mPlayer->state())
    {
    case Vlc::Playing:
        mPlayer->pause();
        break;
    default:
        mPlayer->play();
        break;
    }
*/}
/*
void PlaybackView::stateChanged()
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

void PlaybackView::timeChanged(int position)
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

void PlaybackView::lengthChanged(int duration)
{
    ui->positionSlider->setRange(0, duration);
}
*/
void PlaybackView::setPosition(int position)
{
    if (!mBusy)
    {
        mBusy = true;

        // References
        const double range = mMainWindow->rangeUpper() - mMainWindow->rangeLower();
        const DataPoint &dpStart = mMainWindow->dataPoint(0);

        // Change window position
        mMainWindow->setRange(
                    dpStart.t + position / 1000.,
                    dpStart.t + position / 1000. + range);

        mBusy = false;
    }
}
/*
void PlaybackView::setScrubPosition(int position)
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
*/
void PlaybackView::updateView()
{
    if (mBusy || !mMainWindow) return;

    if (mMainWindow->dataSize() > 0)
    {
        // Enable controls
        ui->playButton->setEnabled(true);
        ui->positionSlider->setEnabled(true);

        // Length changed
        const double range = mMainWindow->rangeUpper() - mMainWindow->rangeLower();
        const DataPoint &dpStart = mMainWindow->dataPoint(0);
        const DataPoint &dpEnd = mMainWindow->dataPoint(mMainWindow->dataSize() - 1);
        const int duration = ((dpEnd.t - dpStart.t) - range) * 1000;
        ui->positionSlider->setRange(0, duration);

        // Position changed
        const int position = (mMainWindow->rangeLower() - dpStart.t) * 1000;
        ui->positionSlider->setValue(position);

        // Update text label
        ui->timeLabel->setText(QString("%1 s").arg(dpStart.t, 0, 'f', 3));
    }
    else
    {
        // Disable controls
        ui->playButton->setEnabled(false);
        ui->positionSlider->setEnabled(false);
    }
}
