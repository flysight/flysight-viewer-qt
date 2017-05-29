#include "playbackview.h"
#include "ui_playbackview.h"

#include <QTimer>

#include "common.h"
#include "mainwindow.h"

#define INTERVAL 250    // Timer interval in ms

PlaybackView::PlaybackView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlaybackView),
    mMainWindow(0),
    mBusy(false),
    mState(Paused)
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

    // Set up timer
    mTimer = new QTimer(this);
    mTimer->setInterval(INTERVAL);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(tick()));
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
    switch(mState)
    {
    case Paused:
        ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        mTimer->start();
        mState = Playing;
        break;
    default:
        ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        mTimer->stop();
        mState = Paused;
        break;
    }
}

void PlaybackView::tick()
{
    // Return now if plot empty
    if (mMainWindow->dataSize() == 0) return;

    const DataPoint &dpEnd = mMainWindow->dataPoint(mMainWindow->dataSize() - 1);

    if (mMainWindow->rangeUpper() == dpEnd.t)
    {
        // Stop playback
        ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        mTimer->stop();
        mState = Paused;
    }
    else
    {
        double lower = mMainWindow->rangeLower() + INTERVAL / 1000.;
        double upper = mMainWindow->rangeUpper() + INTERVAL / 1000.;

        if (upper > dpEnd.t)
        {
            lower -= upper - dpEnd.t;
            upper = dpEnd.t;
        }

        // Round to nearest interval
        lower = (int) (lower * 1000 / INTERVAL) * INTERVAL / 1000.;
        upper = (int) (upper * 1000 / INTERVAL) * INTERVAL / 1000.;

        // Change window position
        mMainWindow->setRange(lower, upper);
    }
}

void PlaybackView::setPosition(int position)
{
    // Return now if plot empty
    if (mMainWindow->dataSize() == 0) return;

    if (!mBusy)
    {
        mBusy = true;

        // Stop playback
        ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        mTimer->stop();
        mState = Paused;

        // Get view range
        const double lower = mMainWindow->rangeLower() + INTERVAL / 1000.;
        const double upper = mMainWindow->rangeUpper() + INTERVAL / 1000.;

        // Get data range
        const DataPoint &dpStart = mMainWindow->dataPoint(0);

        // Change window position
        mMainWindow->setRange(
                    dpStart.t + position / 1000.,
                    dpStart.t + position / 1000. + upper - lower);

        // Update text label
        ui->timeLabel->setText(QString("%1 s").arg(lower, 0, 'f', 3));

        mBusy = false;
    }
}

void PlaybackView::updateView()
{
    if (mBusy || !mMainWindow) return;

    if (mMainWindow->dataSize() > 0)
    {
        mBusy = true;

        // Enable controls
        ui->playButton->setEnabled(true);
        ui->positionSlider->setEnabled(true);

        // Get view range
        const double lower = mMainWindow->rangeLower() + INTERVAL / 1000.;
        const double upper = mMainWindow->rangeUpper() + INTERVAL / 1000.;

        // Get data range
        const DataPoint &dpStart = mMainWindow->dataPoint(0);
        const DataPoint &dpEnd = mMainWindow->dataPoint(mMainWindow->dataSize() - 1);

        // Update slider range
        const int duration = ((dpEnd.t - dpStart.t) - (upper - lower)) * 1000;
        ui->positionSlider->setRange(0, duration);

        // Update slider position
        const int position = (lower - dpStart.t) * 1000;
        ui->positionSlider->setValue(position);

        // Update text label
        ui->timeLabel->setText(QString("%1 s").arg(lower, 0, 'f', 3));

        mBusy = false;
    }
    else
    {
        // Disable controls
        ui->playButton->setEnabled(false);
        ui->positionSlider->setEnabled(false);
    }
}
