/***************************************************************************
**                                                                        **
**  FlySight Viewer                                                       **
**  Copyright 2018 Michael Cooper                                         **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>. **
**                                                                        **
****************************************************************************
**  Contact: Michael Cooper                                               **
**  Website: http://flysight.ca/                                          **
****************************************************************************/

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

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

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
