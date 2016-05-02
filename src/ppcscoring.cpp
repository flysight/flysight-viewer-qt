#include "ppcscoring.h"

#include "mainwindow.h"

PPCScoring::PPCScoring(
        MainWindow *mainWindow):
    ScoringMethod(mainWindow),
    mMainWindow(mainWindow),
    mMode(Time)
{

}

double PPCScoring::score(
        const QVector< DataPoint > &result)
{
    DataPoint dpBottom, dpTop;
    if (mMainWindow->getWindowBounds(result, dpBottom, dpTop))
    {
        switch (mMode)
        {
        case Time:
            return dpBottom.t - dpTop.t;
        case Distance:
            return dpBottom.x - dpTop.x;
        default: // Speed
            return (dpBottom.x - dpTop.x) / (dpBottom.t - dpTop.t);
        }
    }

    return 0;
}

QString PPCScoring::scoreAsText(
        double score)
{
    switch (mMode)
    {
    case Time:
        return QString::number(score) + QString(" s");
    case Distance:
        return (mMainWindow->units() == PlotValue::Metric) ?
                    QString::number(score / 1000) + QString(" km"):
                    QString::number(score * METERS_TO_FEET / 5280) + QString(" mi");
    default: // Speed
        return (mMainWindow->units() == PlotValue::Metric) ?
                    QString::number(score * MPS_TO_KMH) + QString(" km/h"):
                    QString::number(score * MPS_TO_MPH) + QString(" mph");
    }
}

void PPCScoring::setMode(
        Mode mode)
{
    mMode = mode;
    emit dataChanged();
}

void PPCScoring::prepareDataPlot(
        DataPlot *plot)
{
    // Add shading for scoring window
    if (plot->yValue(DataPlot::Elevation)->visible() && mMainWindow->isWindowValid())
    {
        const DataPoint &dpTop = mMainWindow->windowTopDP();
        const DataPoint &dpBottom = mMainWindow->windowBottomDP();

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
