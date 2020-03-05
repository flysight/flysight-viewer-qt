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

#include "speedscoring.h"

#include "mainwindow.h"

#define TIME_DELTA 0.005

SpeedScoring::SpeedScoring(
        MainWindow *mainWindow):
    ScoringMethod(mainWindow),
    mMainWindow(mainWindow),
    mFromExit(2256),
    mWindowBottom(1707)
{

}

void SpeedScoring::setFromExit(
        double fromExit)
{
    mFromExit = fromExit;
    emit scoringChanged();
}

void SpeedScoring::setWindowBottom(
        double windowBottom)
{
    mWindowBottom = windowBottom;
    emit scoringChanged();
}

double SpeedScoring::score(
        const MainWindow::DataPoints &result)
{
    // Get exit
    DataPoint dpExit = mMainWindow->interpolateDataT(0);

    DataPoint dpBottom, dpTop;
    if (getWindowBounds(result, dpBottom, dpTop, dpExit))
    {
        return (dpTop.z - dpBottom.z) / (dpBottom.t - dpTop.t);
    }

    return 0;
}

QString SpeedScoring::scoreAsText(
        double score)
{
    return (mMainWindow->units() == PlotValue::Metric) ?
                QString::number(score * MPS_TO_KMH) + QString(" km/h"):
                QString::number(score * MPS_TO_MPH) + QString(" mph");
}

void SpeedScoring::prepareDataPlot(
        DataPlot *plot)
{
    // Return now if plot empty
    if (mMainWindow->dataSize() == 0) return;

    // Get exit
    DataPoint dpExit = mMainWindow->interpolateDataT(0);

    DataPoint dpBottom, dpTop;
    bool success;

    switch (mMainWindow->windowMode())
    {
    case MainWindow::Actual:
        success = getWindowBounds(mMainWindow->data(), dpBottom, dpTop, dpExit);
        break;
    case MainWindow::Optimal:
        success = getWindowBounds(mMainWindow->optimal(), dpBottom, dpTop, dpExit);
        break;
    }

    // Add shading for scoring window
    if (success && plot->yValue(DataPlot::Elevation)->visible())
    {
        DataPoint dpLower = mMainWindow->interpolateDataT(mMainWindow->rangeLower());
        DataPoint dpUpper = mMainWindow->interpolateDataT(mMainWindow->rangeUpper());

        const double xMin = plot->xValue()->value(dpLower, mMainWindow->units());
        const double xMax = plot->xValue()->value(dpUpper, mMainWindow->units());

        QVector< double > xElev, yElev;

        xElev << xMin << xMax;
        yElev << plot->yValue(DataPlot::Elevation)->value(dpTop, mMainWindow->units())
              << plot->yValue(DataPlot::Elevation)->value(dpTop, mMainWindow->units());

        QCPGraph *graph = plot->addGraph(
                    plot->axisRect()->axis(QCPAxis::atBottom),
                    plot->yValue(DataPlot::Elevation)->axis());
        graph->setData(xElev, yElev);
        graph->setPen(QPen(QBrush(Qt::lightGray), mMainWindow->lineThickness(), Qt::DashLine));

        yElev.clear();
        yElev << plot->yValue(DataPlot::Elevation)->value(dpBottom, mMainWindow->units())
              << plot->yValue(DataPlot::Elevation)->value(dpBottom, mMainWindow->units());

        graph = plot->addGraph(
                    plot->axisRect()->axis(QCPAxis::atBottom),
                    plot->yValue(DataPlot::Elevation)->axis());
        graph->setData(xElev, yElev);
        graph->setPen(QPen(QBrush(Qt::lightGray), mMainWindow->lineThickness(), Qt::DashLine));

        QCPItemRect *rect = new QCPItemRect(plot);

        rect->setPen(QPen(QBrush(Qt::lightGray), mMainWindow->lineThickness(), Qt::DashLine));
        rect->setBrush(QColor(0, 0, 0, 8));

        rect->topLeft->setType(QCPItemPosition::ptAxisRectRatio);
        rect->topLeft->setAxes(plot->xAxis, plot->yValue(DataPlot::Elevation)->axis());
        rect->topLeft->setCoords(-0.1, -0.1);

        rect->bottomRight->setType(QCPItemPosition::ptAxisRectRatio);
        rect->bottomRight->setAxes(plot->xAxis, plot->yValue(DataPlot::Elevation)->axis());
        rect->bottomRight->setCoords(
                    (plot->xValue()->value(dpTop, mMainWindow->units()) - xMin) / (xMax - xMin),
                    1.1);

        rect = new QCPItemRect(plot);

        rect->setPen(QPen(QBrush(Qt::lightGray), mMainWindow->lineThickness(), Qt::DashLine));
        rect->setBrush(QColor(0, 0, 0, 8));

        rect->topLeft->setType(QCPItemPosition::ptAxisRectRatio);
        rect->topLeft->setAxes(plot->xAxis, plot->yValue(DataPlot::Elevation)->axis());
        rect->topLeft->setCoords(
                    (plot->xValue()->value(dpBottom, mMainWindow->units()) - xMin) / (xMax - xMin),
                    -0.1);

        rect->bottomRight->setType(QCPItemPosition::ptAxisRectRatio);
        rect->bottomRight->setAxes(plot->xAxis, plot->yValue(DataPlot::Elevation)->axis());
        rect->bottomRight->setCoords(1.1, 1.1);
    }
}

bool SpeedScoring::getWindowBounds(
        const MainWindow::DataPoints &result,
        DataPoint &dpBottom,
        DataPoint &dpTop,
        const DataPoint &dpExit)
{
    bool found = false;
    int iStart = result.size() - 1;
    double maxScore = 0;

    for (int iEnd = result.size() - 1; iEnd >= 0; --iEnd)
    {
        // Get end point
        const DataPoint &dpEnd = result[iEnd];
        const double tStart = dpEnd.t - 3;

        // Move start point back
        for (; iStart >= 0; --iStart)
        {
            const DataPoint &dpStart = result[iStart];
            if (dpStart.t < tStart + TIME_DELTA) break;
        }

        // If no start point is found
        if (iStart < 0) break;

        // Check window conditions
        const DataPoint &dpStart = result[iStart];
        if (dpStart.t < 0) break;
        if (dpEnd.z < dpExit.z - mFromExit) continue;
        if (dpEnd.z < mWindowBottom) continue;
        if (dpStart.t < tStart - TIME_DELTA) continue;

        // Calculate score
        const double thisScore = (dpStart.z - dpEnd.z) / (dpEnd.t - dpStart.t);
        if (thisScore > maxScore)
        {
            dpBottom = dpEnd;
            dpTop = dpStart;
            maxScore = thisScore;
            found = true;
        }
    }

    return found;
}
