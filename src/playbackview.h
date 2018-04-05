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

#ifndef PLAYBACKVIEW_H
#define PLAYBACKVIEW_H

#include <QDialog>

namespace Ui {
    class PlaybackView;
}

class MainWindow;
class QTimer;

class PlaybackView : public QWidget
{
    Q_OBJECT

public:
    explicit PlaybackView(QWidget *parent = 0);
    ~PlaybackView();

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow) { mMainWindow = mainWindow; }

private:
    enum {
        Paused, Playing
    } mState;

    Ui::PlaybackView *ui;
    MainWindow       *mMainWindow;

    bool              mBusy;
    QTimer           *mTimer;

public slots:
    void play();
    void updateView();

private slots:
    void setPosition(int position);
    void tick();
};

#endif // PLAYBACKVIEW_H
