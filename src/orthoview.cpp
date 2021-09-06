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

#include "orthoview.h"

#include <QPointF>
#include <QTimer>
#include <QVector3D>

#include "common.h"
#include "mainwindow.h"

#define WINDOW_MARGIN 1.2
#define MIN_ARROW_LEN 0.2

OrthoView::OrthoView(QWidget *parent) :
    QCustomPlot(parent),
    mMainWindow(0),
    m_pan(false),
    m_azimuth(-PI/2),
    m_elevation(PI/2),
    m_scale(1)
{
    setMouseTracking(true);

    // Set up timer
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(1000);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(endTimer()));
}

QSize OrthoView::sizeHint() const
{
    // Keeps windows from being initialized as very short
    return QSize(175, 175);
}

void OrthoView::mousePressEvent(
        QMouseEvent *event)
{
    if (axisRect()->rect().contains(event->pos()))
    {
        m_beginPos = event->pos() - axisRect()->center();
        m_pan = true;
    }

    QCustomPlot::mousePressEvent(event);
}

void OrthoView::mouseReleaseEvent(
        QMouseEvent *event)
{
    m_pan = false;
    QCustomPlot::mouseReleaseEvent(event);
}

void OrthoView::mouseMoveEvent(
        QMouseEvent *event)
{
    if (m_pan)
    {
        QRect rect = axisRect()->rect();
        QPoint endPos = event->pos() - rect.center();

        double r = (double) qMin(rect.width(), rect.height()) / 2 / WINDOW_MARGIN;

        double a1 = (double) (m_beginPos.x() - rect.left()) / r;
        double a2 = (double) (endPos.x() - rect.left()) / r;
        double a = a2 - a1;

        double e1 = (double) (m_beginPos.y() - rect.bottom()) / r;
        double e2 = (double) (endPos.y() - rect.bottom()) / r;
        double e = e2 - e1;

        m_azimuth   -= a;
        m_elevation += e;

        while (m_azimuth < -PI) m_azimuth += 2 * PI;
        while (m_azimuth >  PI) m_azimuth -= 2 * PI;

        m_elevation = qMax(m_elevation, 0.);
        m_elevation = qMin(m_elevation, PI / 2);

        m_beginPos = endPos;

        updateView();
    }

    if (QCPCurve *curve = qobject_cast<QCPCurve *>(plottable(0)))
    {
        QSharedPointer<QCPCurveDataContainer> data = curve->data();

        double resultTime;
        double resultDistance = std::numeric_limits<double>::max();

        for (QCPCurveDataContainer::const_iterator it = data->constBegin();
             it != data->constEnd() && (it + 1) != data->constEnd();
             ++ it)
        {
            QPointF pt1 = QPointF(xAxis->coordToPixel(it->key),
                                  yAxis->coordToPixel(it->value));
            QPointF pt2 = QPointF(xAxis->coordToPixel((it + 1)->key),
                                  yAxis->coordToPixel((it + 1)->value));

            double mu;
            double dist = sqrt(distSqrToLine(pt1, pt2, event->pos(), mu));

            if (dist < resultDistance)
            {
                double t1 = it->t;
                double t2 = (it + 1)->t;

                resultTime = t1 + mu * (t2 - t1);
                resultDistance = dist;
            }
        }

        if (resultDistance < selectionTolerance())
        {
            mMainWindow->setMark(resultTime);
        }
        else
        {
            mMainWindow->clearMark();
        }
    }
}

void OrthoView::wheelEvent(
        QWheelEvent *event)
{
    // Adjust scale
    m_scale /= exp((double) -event->angleDelta().y() / 500);
    if (m_scale < 1) m_scale = 1;

    // Start timer for "zoom" display
    m_timer->start();

    // Update the view
    updateView();
}

void OrthoView::endTimer()
{
    updateView();
}

void OrthoView::updateView()
{
    clearPlottables();
    clearItems();

    // Return now if plot empty
    if (mMainWindow->dataSize() == 0) return;

    // Get plot range
    double lower = mMainWindow->rangeLower();
    double upper = mMainWindow->rangeUpper();

    // Calculate camera vectors
    QVector3D up(-sin(m_elevation) * cos(m_azimuth),
                 -sin(m_elevation) * sin(m_azimuth),
                  cos(m_elevation));
    QVector3D bk(cos(m_elevation) * cos(m_azimuth),
                 cos(m_elevation) * sin(m_azimuth),
                 sin(m_elevation));
    QVector3D rt = QVector3D::crossProduct(up, bk);

    QVector< double > t, x, y, z;

    double xMin, xMax;
    double yMin, yMax;
    double zMin, zMax;

    double uMin, uMax;
    double vMin, vMax;
    double wMin, wMax;

    bool first = true;

    for (int i = 0; i < mMainWindow->dataSize(); ++i)
    {
        const DataPoint &dp = mMainWindow->dataPoint(i);

        if (lower <= dp.t && dp.t <= upper)
        {
            t.append(dp.t);

            QVector3D cur;
            if (mMainWindow->units() == PlotValue::Metric)
            {
                cur = QVector3D(dp.x, dp.y, dp.z);
            }
            else
            {
                cur = QVector3D(dp.x, dp.y, dp.z) * METERS_TO_FEET;
            }

            x.append(QVector3D::dotProduct(cur, rt));
            y.append(QVector3D::dotProduct(cur, up));
            z.append(QVector3D::dotProduct(cur, bk));

            if (first)
            {
                xMin = xMax = x.back();
                yMin = yMax = y.back();
                zMin = zMax = z.back();

                uMin = uMax = dp.x;
                vMin = vMax = dp.y;
                wMin = wMax = dp.z;

                first = false;
            }
            else
            {
                if (x.back() < xMin) xMin = x.back();
                if (x.back() > xMax) xMax = x.back();

                if (y.back() < yMin) yMin = y.back();
                if (y.back() > yMax) yMax = y.back();

                if (z.back() < zMin) zMin = z.back();
                if (z.back() > zMax) zMax = z.back();

                if (dp.x < uMin) uMin = dp.x;
                if (dp.x > uMax) uMax = dp.x;

                if (dp.y < vMin) vMin = dp.y;
                if (dp.y > vMax) vMax = dp.y;

                if (dp.z < wMin) wMin = dp.z;
                if (dp.z > wMax) wMax = dp.z;
            }
        }
    }

    QCPCurve *curve = new QCPCurve(xAxis, yAxis);
    curve->setData(t, x, y);
    curve->setPen(QPen(Qt::black, mMainWindow->lineThickness()));

    QVector3D mid((uMin + uMax) / 2,
                  (vMin + vMax) / 2,
                  (wMin + wMax) / 2);

    double xMid = QVector3D::dotProduct(mid, rt);
    double yMid = QVector3D::dotProduct(mid, up);
    double zMid = QVector3D::dotProduct(mid, bk);

    double rMax = 0;
    for (int i = 0; i < x.size(); ++i)
    {
        const double dx = x[i] - xMid;
        const double dy = y[i] - yMid;
        const double dz = z[i] - zMid;
        const double r = dx * dx + dy * dy + dz * dz;
        if (r > rMax) rMax = r;
    }
    rMax = sqrt(rMax);

    setViewRange(xMid - rMax / m_scale, xMid + rMax / m_scale,
                 yMid - rMax / m_scale, yMid + rMax / m_scale);

    if (mMainWindow->mediaCursorRef() > 0)
    {
        const DataPoint &dp = mMainWindow->interpolateDataT(mMainWindow->mediaCursor());

        QVector< double > xMark, yMark, zMark;

        QVector3D cur;
        if (mMainWindow->units() == PlotValue::Metric)
        {
            cur = QVector3D(dp.x, dp.y, dp.z);
        }
        else
        {
            cur = QVector3D(dp.x, dp.y, dp.z) * METERS_TO_FEET;
        }

        xMark.append(QVector3D::dotProduct(cur, rt));
        yMark.append(QVector3D::dotProduct(cur, up));
        zMark.append(QVector3D::dotProduct(cur, bk));

        QCPGraph *graph = addGraph();
        graph->setData(xMark, yMark);
        graph->setPen(QPen(Qt::darkGray, mMainWindow->lineThickness()));
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle::ssDisc);
    }

    if (mMainWindow->markActive())
    {
        const DataPoint &dpEnd = mMainWindow->interpolateDataT(mMainWindow->markEnd());

        QVector< double > xMark, yMark, zMark;

        QVector3D cur;
        if (mMainWindow->units() == PlotValue::Metric)
        {
            cur = QVector3D(dpEnd.x, dpEnd.y, dpEnd.z);
        }
        else
        {
            cur = QVector3D(dpEnd.x, dpEnd.y, dpEnd.z) * METERS_TO_FEET;
        }

        xMark.append(QVector3D::dotProduct(cur, rt));
        yMark.append(QVector3D::dotProduct(cur, up));
        zMark.append(QVector3D::dotProduct(cur, bk));

        QCPGraph *graph = addGraph();
        graph->setData(xMark, yMark);
        graph->setPen(QPen(Qt::black, mMainWindow->lineThickness()));
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle::ssDisc);
    }

    if (mMainWindow->dataSize() > 0)
    {
        QVector< double > xMark, yMark, zMark;

        for (int i = 0; i < mMainWindow->waypointSize(); ++i)
        {
            const DataPoint &dp0 = mMainWindow->interpolateDataT(0);
            DataPoint dp = mMainWindow->waypoint(i);

            double distance = mMainWindow->getDistance(dp0, dp);
            double bearing = mMainWindow->getBearing(dp0, dp);

            dp.x = distance * sin(bearing);
            dp.y = distance * cos(bearing);

            QVector3D cur;
            if (mMainWindow->units() == PlotValue::Metric)
            {
                cur = QVector3D(dp.x, dp.y, dp.z);
            }
            else
            {
                cur = QVector3D(dp.x, dp.y, dp.z) * METERS_TO_FEET;
            }

            xMark.append(QVector3D::dotProduct(cur, rt));
            yMark.append(QVector3D::dotProduct(cur, up));
            zMark.append(QVector3D::dotProduct(cur, bk));
        }

        QCPGraph *graph = addGraph();
        graph->setData(xMark, yMark);
        graph->setPen(QPen(Qt::black, mMainWindow->lineThickness()));
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle::ssPlus);
    }

    if (m_timer->isActive())
    {
        // Add label to show zoom
        QCPItemText *textLabel = new QCPItemText(this);

        QPainter painter(this);
        double mmPerPix = (double) painter.device()->widthMM() / painter.device()->width();

        double xRatioPerPix = 1.0 / axisRect()->width();
        double xRatioPerMM = xRatioPerPix / mmPerPix;

        double yRatioPerPix = 1.0 / axisRect()->height();
        double yRatioPerMM = yRatioPerPix / mmPerPix;

        textLabel->setPositionAlignment(Qt::AlignBottom|Qt::AlignRight);
        textLabel->setTextAlignment(Qt::AlignRight);
        textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
        textLabel->position->setCoords(1 - 5 * xRatioPerMM,
                                       1 - 5 * yRatioPerMM);
        textLabel->setText(QString("Zoom = %1%").arg(m_scale * 100, 0, 'f', 0));
    }

    addOrientation();

    replot();
}

void OrthoView::addOrientation()
{
    QPainter painter(this);

    double mmPerPix = (double) painter.device()->widthMM() / painter.device()->width();
    double valPerPix = xAxis->range().size() / axisRect()->width();
    double valPerMM = valPerPix / mmPerPix;

    // Camera vectors
    QVector3D up(-sin(m_elevation) * cos(m_azimuth),
                 -sin(m_elevation) * sin(m_azimuth),
                  cos(m_elevation));
    QVector3D bk(cos(m_elevation) * cos(m_azimuth),
                 cos(m_elevation) * sin(m_azimuth),
                 sin(m_elevation));
    QVector3D rt = QVector3D::crossProduct(up, bk);

    // Transformed basis
    QVector3D o(-rt.x() - rt.y() - rt.z(),
                -up.x() - up.y() - up.z(),
                -bk.x() - bk.y() - bk.z());
    QVector3D x( rt.x() - rt.y() - rt.z(),
                 up.x() - up.y() - up.z(),
                 bk.x() - bk.y() - bk.z());
    QVector3D y(-rt.x() + rt.y() - rt.z(),
                -up.x() + up.y() - up.z(),
                -bk.x() + bk.y() - bk.z());
    QVector3D z(-rt.x() - rt.y() + rt.z(),
                -up.x() - up.y() + up.z(),
                -bk.x() - bk.y() + bk.z());

    // Corner in plot space
    QPointF corner(xAxis->range().upper - 10 * valPerMM,
                   yAxis->range().upper - 10 * valPerMM);

    // Add "grass"
    QVector< double > ts, xs, ys;
    ts.append(0);
    xs.append(corner.x() + 5 * valPerMM * o.x());
    ys.append(corner.y() + 5 * valPerMM * o.y());
    for (int i = 0; i <= 10; ++i)
    {
        double a = i / 10.0 * PI / 2.0;
        QVector3D u = o + (x - o) * cos(a) + (y - o) * sin(a);

        ts.append(i + 1);
        xs.append(corner.x() + 5 * valPerMM * u.x());
        ys.append(corner.y() + 5 * valPerMM * u.y());
    }
    ts.append(12);
    xs.append(corner.x() + 5 * valPerMM * o.x());
    ys.append(corner.y() + 5 * valPerMM * o.y());

    QCPCurve *curve = new QCPCurve(xAxis, yAxis);
    curve->setData(ts, xs, ys);
    curve->setPen(QPen(Qt::green, mMainWindow->lineThickness()));
    curve->setBrush(QBrush(QColor(0, 255, 0, 64)));

    // Add arrows
    QCPItemLine *arrow = new QCPItemLine(this);
    arrow->start->setCoords(corner + 5 * valPerMM * o.toPointF());
    arrow->end->setCoords(corner + 5 * valPerMM * x.toPointF());
    if (QVector2D(x - o).length() > MIN_ARROW_LEN)
        arrow->setHead(QCPLineEnding::esSpikeArrow);

    arrow = new QCPItemLine(this);
    arrow->start->setCoords(corner + 5 * valPerMM * o.toPointF());
    arrow->end->setCoords(corner + 5 * valPerMM * y.toPointF());
    if (QVector2D(y - o).length() > MIN_ARROW_LEN)
        arrow->setHead(QCPLineEnding::esSpikeArrow);

    arrow = new QCPItemLine(this);
    arrow->start->setCoords(corner + 5 * valPerMM * o.toPointF());
    arrow->end->setCoords(corner + 5 * valPerMM * z.toPointF());
    if (QVector2D(z - o).length() > MIN_ARROW_LEN)
        arrow->setHead(QCPLineEnding::esSpikeArrow);
}

void OrthoView::setViewRange(
        double xMin,
        double xMax,
        double yMin,
        double yMax)
{
    QPainter painter(this);

    double xMMperPix = (double) painter.device()->widthMM() / painter.device()->width();
    double yMMperPix = (double) painter.device()->heightMM() / painter.device()->height();

    QRect rect = axisRect()->rect();

    double xSpan = (xMax - xMin) * WINDOW_MARGIN;
    double ySpan = (yMax - yMin) * WINDOW_MARGIN;

    double xScale = xSpan / rect.width() / xMMperPix;
    double yScale = ySpan / rect.height() / yMMperPix;

    double scale = qMax(xScale, yScale);

    double xMid = (xMin + xMax) / 2;
    double yMid = (yMin + yMax) / 2;

    xMin = xMid - rect.width() * xMMperPix * scale / 2;
    xMax = xMid + rect.width() * xMMperPix * scale / 2;

    yMin = yMid - rect.height() * yMMperPix * scale / 2;
    yMax = yMid + rect.height() * yMMperPix * scale / 2;

    xAxis->setRange(xMin, xMax);
    yAxis->setRange(yMin, yMax);
}
