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

#ifndef DATAPLOT_H
#define DATAPLOT_H

#include "QCustomPlot/qcustomplot.h"

#include "datapoint.h"
#include "plotvalue.h"

class MainWindow;

class DataPlot : public QCustomPlot
{
    Q_OBJECT

public:
    typedef enum {
        Time = 0,
        Distance2D,
        Distance3D
    } XAxisType;

    typedef enum {
        Elevation = 0,
        VerticalSpeed,
        HorizontalSpeed,
        TotalSpeed,
        DiveAngle,
        Curvature,
        GlideRatio,
        HorizontalAccuracy,
        VerticalAccuracy,
        SpeedAccuracy,
        SEP,
        NumberOfSatellites,
        Acceleration,
        TotalEnergy,
        EnergyRate,
        Lift,
        Drag,
        Course,
        CourseRate,
        CourseAccuracy,
        AccForward,
        AccRight,
        AccDown,
        AccMagnitude,
        yaLast
    } YAxisType;

    explicit DataPlot(QWidget *parent = 0);
    ~DataPlot();

    void setMainWindow(MainWindow *mainWindow) { mMainWindow = mainWindow; }

    PlotValue *xValue() const { return m_xValues[m_xAxisType]; }
    PlotValue *yValue(int i) const { return m_yValues[i]; }

    void togglePlot(YAxisType plot);
    bool plotVisible(YAxisType plot) const { return m_yValues[plot]->visible(); }

    void setXAxisType(XAxisType xAxisType);
    XAxisType xAxisType() const { return m_xAxisType ; }

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void wheelEvent(QWheelEvent *event);

    void leaveEvent(QEvent *);

private:
    double m_tCursor, m_tBegin;
    int m_yCursor, m_yBegin;
    bool m_cursorValid;

    bool m_dragging;

    MainWindow *mMainWindow;

    QVector< PlotValue* > m_xValues;
    XAxisType             m_xAxisType;

    QVector< PlotValue* > m_yValues;

    void updateYRanges();
    void setRange(const QCPRange &range);

    void setMark(double start, double end);
    void setMark(double mark);

    DataPoint interpolateDataX(double x);

    int findIndexBelowX(double x);
    int findIndexAboveX(double x);

    void initPlot();

    void readSettings();
    void writeSettings();

public slots:
    void updatePlot();
    void updateRange();
    void updateCursor();
};

#endif // DATAPLOT_H
