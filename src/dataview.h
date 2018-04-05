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

#ifndef DATAVIEW_H
#define DATAVIEW_H

#include "QCustomPlot/qcustomplot.h"

class MainWindow;

class DataView : public QCustomPlot
{
    Q_OBJECT

public:
    typedef enum {
        Top = 0,
        Left,
        Front
    } Direction;

    explicit DataView(QWidget *parent = 0);

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow) { mMainWindow = mainWindow; }
    void setDirection(Direction direction) { mDirection = direction; }

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    MainWindow *mMainWindow;

    Direction   mDirection;

    QPoint      m_topViewBeginPos;
    bool        m_topViewPan;

    QVector< QCPGraph* >  m_cursors;

    void setViewRange(double xMin, double xMax,
                      double yMin, double yMax);
    void addNorthArrow();

public slots:
    void updateView();
    void updateCursor();
};

#endif // DATAVIEW_H
