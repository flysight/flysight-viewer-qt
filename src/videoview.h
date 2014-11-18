#ifndef VIDEOVIEW_H
#define VIDEOVIEW_H

#include <QWidget>
#include <QMediaPlayer>

class MainWindow;

namespace Ui {
    class VideoView;
}

class VideoView : public QWidget
{
    Q_OBJECT

public:
    explicit VideoView(QWidget *parent = 0);
    ~VideoView();

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow) { mMainWindow = mainWindow; }

private:
    Ui::VideoView *ui;
    MainWindow    *mMainWindow;
    QMediaPlayer   mPlayer;
    qint64         mZeroPosition;
    bool           mBlockUpdate;

public slots:
    void openFile();
    void play();
    void updateView();
    void zero();

private slots:
    void mediaStateChanged(QMediaPlayer::State state);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void setPosition(int position);
    void setScrubPosition(int position);
};

#endif // VIDEOVIEW_H
