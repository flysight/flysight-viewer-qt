#include "speedscoring.h"

#include "mainwindow.h"

SpeedScoring::SpeedScoring(
        MainWindow *mainWindow):
    ScoringMethod(mainWindow),
    mMainWindow(mainWindow),
    mWindowTop(2700),
    mWindowBottom(1700)
{

}

void SpeedScoring::setWindow(
        double windowBottom,
        double windowTop)
{
    mWindowBottom = windowBottom;
    mWindowTop = windowTop;
    emit dataChanged();
}

double SpeedScoring::score(
        const QVector< DataPoint > &result)
{
    DataPoint dpBottom, dpTop;
    if (getWindowBounds(result, dpBottom, dpTop))
    {
        return (mWindowTop - mWindowBottom) / (dpBottom.t - dpTop.t);
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
        plot->addItem(rect);

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
        plot->addItem(rect);

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
        const QVector< DataPoint > &result,
        DataPoint &dpBottom,
        DataPoint &dpTop)
{
    bool foundBottom = false;
    bool foundTop = false;
    int bottom, top;

    for (int i = result.size() - 1; i >= 0; --i)
    {
        const DataPoint &dp = result[i];

        if (dp.z < mWindowBottom)
        {
            bottom = i;
            foundBottom = true;
        }

        if (dp.z < mWindowTop)
        {
            top = i;
            foundTop = false;
        }

        if (dp.z > mWindowTop)
        {
            foundTop = true;
        }

        if (dp.t < 0) break;
    }

    if (foundBottom && foundTop)
    {
        // Calculate bottom of window
        const DataPoint &dp1 = result[bottom - 1];
        const DataPoint &dp2 = result[bottom];
        dpBottom = DataPoint::interpolate(dp1, dp2, (mWindowBottom - dp1.z) / (dp2.z - dp1.z));

        // Calculate top of window
        const DataPoint &dp3 = result[top - 1];
        const DataPoint &dp4 = result[top];
        dpTop = DataPoint::interpolate(dp3, dp4, (mWindowTop - dp3.z) / (dp4.z - dp3.z));

        return true;
    }
    else
    {
        return false;
    }
}
