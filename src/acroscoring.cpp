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

#include "acroscoring.h"

#include "mainwindow.h"

AcroScoring::AcroScoring(
        MainWindow *mainWindow):
    ScoringMethod(mainWindow),
    mMainWindow(mainWindow),
    mSpeed(8),
    mAltitude(2286)
{

}

void AcroScoring::setSpeed(
        double speed)
{
    mSpeed = speed;
    emit scoringChanged();
}

void AcroScoring::setAltitude(
        double altitude)
{
    mAltitude = altitude;
    emit scoringChanged();
}

void AcroScoring::prepareDataPlot(
        DataPlot *plot)
{
    // Return now if plot empty
    if (mMainWindow->dataSize() == 0) return;

    DataPoint dpBottom, dpTop;
    bool success = false;

    switch (mMainWindow->windowMode())
    {
    case MainWindow::Actual:
        success = getWindowBounds(mMainWindow->data(), dpBottom, dpTop);
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

bool AcroScoring::getWindowBounds(
        const MainWindow::DataPoints &result,
        DataPoint &dpBottom,
        DataPoint &dpTop)
{
    const int iExit = qMax (0, findIndexBelowT(result, 0));

    int iStart;
    for (iStart = iExit; iStart < result.size(); ++iStart)
    {
        const DataPoint &dp = result[iStart];
        if (dp.velD > mSpeed) break;
    }

    if (iStart == result.size()) return false;

    const DataPoint &dp1 = result[iStart - 1];
    const DataPoint &dp2 = result[iStart];
    dpTop = DataPoint::interpolate(dp1, dp2,
        (mSpeed - dp1.velD) / (dp2.velD - dp1.velD));

    if ((dpTop.t < dp1.t) || (dpTop.t > dp2.t)) return false;

    int iEnd;
    for (iEnd = iStart; iEnd < result.size(); ++iEnd)
    {
        const DataPoint &dp = result[iEnd];
        if (dp.hMSL < dpTop.hMSL - mAltitude) break;
    }

    if (iEnd == result.size()) return false;

    const DataPoint &dp3 = result[iEnd - 1];
    const DataPoint &dp4 = result[iEnd];
    dpBottom = DataPoint::interpolate(dp3, dp4,
        (dpTop.hMSL - mAltitude - dp3.hMSL) / (dp4.hMSL - dp3.hMSL));

    if ((dpBottom.t < dp3.t) || (dpBottom.t > dp4.t)) return false;

    return true;
}

int AcroScoring::findIndexBelowT(
        const MainWindow::DataPoints &result,
        double t)
{
    int below = -1;
    int above = result.size();

    while (below + 1 != above)
    {
        int mid = (below + above) / 2;
        const DataPoint &dp = result[mid];

        if (dp.t < t) below = mid;
        else          above = mid;
    }

    return below;
}

