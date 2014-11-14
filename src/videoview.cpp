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
    mZeroPosition(0)
{
    ui->setupUi(this);

    QVideoWidget *videoWidget = new QVideoWidget;

    connect(ui->openButton, SIGNAL(clicked()), this, SLOT(openFile()));

    ui->playButton->setEnabled(false);
    ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    connect(ui->playButton, SIGNAL(clicked()), this, SLOT(play()));

    ui->zeroButton->setEnabled(false);

    connect(ui->zeroButton, SIGNAL(clicked()), this, SLOT(zero()));

    ui->positionSlider->setRange(0, 0);

    connect(ui->positionSlider, SIGNAL(sliderMoved(int)), this, SLOT(setPosition(int)));

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
        mPlayer.setMedia(QUrl::fromLocalFile(fileName));
        ui->playButton->setEnabled(true);
        ui->zeroButton->setEnabled(true);
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
}

void VideoView::positionChanged(qint64 position)
{
    ui->positionSlider->setValue(position);

    // Update other views
    mMainWindow->setMark((double) (position - mZeroPosition) / 1000);
}

void VideoView::durationChanged(qint64 duration)
{
    ui->positionSlider->setRange(0, duration);
}

void VideoView::setPosition(int position)
{
    mPlayer.setPosition(position);
}

void VideoView::zero()
{
    mZeroPosition = mPlayer.position();
}

void VideoView::updateView()
{
/*    if (mMainWindow->markActive())
    {
        // Add marker to map
        const DataPoint &dpEnd = mMainWindow->interpolateDataT(mMainWindow->markEnd());

        if (dpEnd.t * 1000 != mPlayer.position())
        {
            // Set playback position
            mPlayer.setPosition(dpEnd.t * 1000);

            // Force update
            // TODO: There must be a better way to handle this
            //mPlayer.play();
            //mPlayer.pause();
        }
    }
*/
}
