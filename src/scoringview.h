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

#ifndef SCORINGVIEW_H
#define SCORINGVIEW_H

#include <QWidget>

namespace Ui {
    class ScoringView;
}

class DataPlot;
class DataPoint;
class MainWindow;
class PerformanceForm;
class PPCForm;
class SpeedForm;
class WideOpenDistanceForm;
class WideOpenSpeedForm;
class FlareForm;

class ScoringView : public QWidget
{
    Q_OBJECT

public:
    explicit ScoringView(QWidget *parent = 0);
    ~ScoringView();

    void setMainWindow(MainWindow *mainWindow);

private:
    Ui::ScoringView      *ui;
    MainWindow           *mMainWindow;
    PPCForm              *mPPCForm;
    SpeedForm            *mSpeedForm;
    PerformanceForm      *mPerformanceForm;
    WideOpenSpeedForm    *mWideOpenSpeedForm;
    WideOpenDistanceForm *mWideOpenDistanceForm;
    FlareForm            *mFlareForm;

public slots:
    void updateView();

private slots:
    void changePage(int page);
};

#endif // SCORINGVIEW_H
