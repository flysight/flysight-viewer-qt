/***************************************************************************
**                                                                        **
**  FlySight Viewer                                                       **
**  Copyright 2018 Michael Cooper, Matt Coffin                            **
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

#include "flarescoring.h"

#include "mainwindow.h"

FlareScoring::FlareScoring(
        MainWindow *mainWindow):
    ScoringMethod(mainWindow),
    mMainWindow(mainWindow),
    mWindowBottom(1000)
{

}

void FlareScoring::setWindowBottom(
        double windowBottom)
{
    mWindowBottom = windowBottom;
    emit scoringChanged();
}

double FlareScoring::score(
        const MainWindow::DataPoints &result)
{
    DataPoint dpBottom, dpTop;
    if (getWindowBounds(result, dpBottom, dpTop))
    {
        return dpTop.hMSL- dpBottom.hMSL;
    }

    return 0;
}

QString FlareScoring::scoreAsText(
        double score)
{
    return QString::number(score) + QString(" m");
}

void FlareScoring::prepareDataPlot(
        DataPlot *plot)
{
    // Return now if plot empty
    if (mMainWindow->dataSize() == 0) return;

    DataPoint dpBottom, dpTop;
    bool success;

    switch (mMainWindow->windowMode())
    {
    case MainWindow::Actual:
        success = getWindowBounds(mMainWindow->data(), dpBottom, dpTop);
        break;
    case MainWindow::Optimal:
        success = getWindowBounds(mMainWindow->optimal(), dpBottom, dpTop);
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
        yElev << plot->yValue(DataPlot::Elevation)->value(dpBottom, mMainWindow->units())
              << plot->yValue(DataPlot::Elevation)->value(dpBottom, mMainWindow->units());

        QCPGraph *graph = plot->addGraph(
                    plot->axisRect()->axis(QCPAxis::atBottom),
                    plot->yValue(DataPlot::Elevation)->axis());
        graph->setData(xElev, yElev);
        graph->setPen(QPen(QBrush(Qt::lightGray), mMainWindow->lineThickness(), Qt::DashLine));

        yElev.clear();
        yElev << plot->yValue(DataPlot::Elevation)->value(dpTop, mMainWindow->units())
              << plot->yValue(DataPlot::Elevation)->value(dpTop, mMainWindow->units());

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
                    (plot->xValue()->value(dpBottom, mMainWindow->units()) - xMin) / (xMax - xMin),
                    1.1);

        rect = new QCPItemRect(plot);

        rect->setPen(QPen(QBrush(Qt::lightGray), mMainWindow->lineThickness(), Qt::DashLine));
        rect->setBrush(QColor(0, 0, 0, 8));

        rect->topLeft->setType(QCPItemPosition::ptAxisRectRatio);
        rect->topLeft->setAxes(plot->xAxis, plot->yValue(DataPlot::Elevation)->axis());
        rect->topLeft->setCoords(
                    (plot->xValue()->value(dpTop, mMainWindow->units()) - xMin) / (xMax - xMin),
                    -0.1);

        rect->bottomRight->setType(QCPItemPosition::ptAxisRectRatio);
        rect->bottomRight->setAxes(plot->xAxis, plot->yValue(DataPlot::Elevation)->axis());
        rect->bottomRight->setCoords(1.1, 1.1);
    }
}

bool FlareScoring::getWindowBounds(
        const MainWindow::DataPoints &result,
        DataPoint &dpBottom,
        DataPoint &dpTop)
{
    int bottom, top;
    double hBottom, hTop;
    DataPoint dpPrev;

    for (int i = result.size() - 1; i >= 0; --i)
    {
        const DataPoint &dp = result[i];

        if ((i == result.size() - 1) || (dp.z < mWindowBottom))
        {
            dpTop = dpBottom = dpPrev = dp;
        }

        if (dp.hMSL < dpPrev.hMSL)
        {
            bottom = i;
            hBottom = dp.hMSL;
        }
        else
        {
            bottom = top = i;
            hBottom = hTop = dp.hMSL;
        }

        if (hTop - hBottom > dpTop.hMSL - dpBottom.hMSL)
        {
            dpTop = result[top];
            dpBottom = result[bottom];
        }

        dpPrev = dp;

        if (dp.t < 0) break;
    }

    return (result.size() > 0) && (dpTop.hMSL > dpBottom.hMSL);
}
