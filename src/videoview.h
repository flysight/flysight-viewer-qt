#ifndef VIDEOVIEW_H
#define VIDEOVIEW_H

#include <QDialog>

namespace Ui {
    class VideoView;
}

class MainWindow;

class VlcInstance;
class VlcMedia;
class VlcMediaPlayer;

class VideoView : public QWidget
{
    Q_OBJECT

public:
    explicit VideoView(QWidget *parent = 0);
    ~VideoView();

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow) { mMainWindow = mainWindow; }
    void setMedia(const QString &fileName);

private:
    Ui::VideoView  *ui;
    MainWindow     *mMainWindow;

    VlcInstance    *mInstance;
    VlcMedia       *mMedia;
    VlcMediaPlayer *mPlayer;

    qint64          mZeroPosition;
    bool            mBusy;

public slots:
    void play();
    void updateView();
    void zero();

private slots:
    void stateChanged();
    void timeChanged(int position);
    void lengthChanged(int duration);
    void setPosition(int position);
    void setScrubPosition(int position);
};

#endif // VIDEOVIEW_H
