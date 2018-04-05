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

#ifndef ORTHOVIEW_H
#define ORTHOVIEW_H

#include "QCustomPlot/qcustomplot.h"

class MainWindow;
class QTimer;

class OrthoView : public QCustomPlot
{
    Q_OBJECT

public:
    explicit OrthoView(QWidget *parent = 0);

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow) { mMainWindow = mainWindow; }

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    MainWindow *mMainWindow;

    QPoint      m_beginPos;
    bool        m_pan;

    double      m_azimuth;
    double      m_elevation;
    double      m_scale;

    QTimer     *m_timer;

    void addOrientation();
    void setViewRange(double xMin, double xMax,
                      double yMin, double yMax);

public slots:
    void updateView();
    void endTimer();
};

#endif // ORTHOVIEW_H
