#ifndef SIMULATIONVIEW_H
#define SIMULATIONVIEW_H

#include <QWidget>

#include "config.h"
#include "tone.h"
#include "ubx.h"

namespace Ui {
class SimulationView;
}

class DataPoint;
class MainWindow;
class VlcInstance;
class VlcMedia;
class VlcMediaPlayer;

class SimulationView : public QWidget
{
    Q_OBJECT

public:
    explicit SimulationView(QWidget *parent = 0);
    ~SimulationView();

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow);

    const Config &config() const { return mConfig; }

private:
    Ui::SimulationView *ui;
    MainWindow         *mMainWindow;

    Config             mConfig;
    Tone               mTone;
    UBX                mUBX;

    VlcInstance        *mInstance;
    VlcMedia           *mMedia;
    VlcMediaPlayer     *mPlayer;

    bool               mBusy;

    void setMedia(const QString &fileName);

public slots:
    void play();
    void updateView();

private slots:
    void on_browseButton_clicked();
    void on_processButton_clicked();

    void stateChanged();
    void timeChanged(int position);
    void lengthChanged(int duration);
    void setPosition(int position);
    void setScrubPosition(int position);
};

#endif // SIMULATIONVIEW_H
