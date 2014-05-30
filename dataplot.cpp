#include "dataplot.h"

DataPlot::DataPlot(QWidget *parent) :
    QCustomPlot(parent),
    m_dragging(None)
{
    setMouseTracking(true);
}

void DataPlot::mousePressEvent(
        QMouseEvent *event)
{
    if (mAxisRect.contains(event->pos()))
    {
        m_beginPos = event->pos();

        if (event->modifiers() & Qt::ControlModifier)
        {
            m_dragging = Zoom;
        }
        else
        {
            m_dragging = Pan;
        }

        update();
    }

    QCustomPlot::mousePressEvent(event);
}

void DataPlot::mouseReleaseEvent(
        QMouseEvent *event)
{
    QPoint endPos = event->pos();

    QCPRange range(qMin(xAxis->pixelToCoord(m_beginPos.x()),
                        xAxis->pixelToCoord(endPos.x())),
                   qMax(xAxis->pixelToCoord(m_beginPos.x()),
                        xAxis->pixelToCoord(endPos.x())));

    if (m_dragging == Zoom)
    {
        emit zoom(range);
    }

    if (m_dragging != None)
    {
        m_dragging = None;
        replot();
    }

    QCustomPlot::mouseReleaseEvent(event);
}

void DataPlot::mouseMoveEvent(
        QMouseEvent *event)
{
    m_cursorPos = event->pos();

    if (m_dragging == Pan)
    {
        QPoint endPos = m_cursorPos;

        emit pan(xAxis->pixelToCoord(m_beginPos.x()),
                 xAxis->pixelToCoord(endPos.x()));

        m_beginPos = endPos;
    }

    if (mAxisRect.contains(event->pos()))
    {
        emit mark(xAxis->pixelToCoord(m_cursorPos.x()));
    }
    else
    {
        emit clear();
    }

    update();

    QCustomPlot::mouseMoveEvent(event);
}

void DataPlot::wheelEvent(
        QWheelEvent *event)
{
    if (!(event->modifiers() & Qt::ControlModifier))
    {
        if (mAxisRect.contains(event->pos()))
        {
            double multiplier = exp((double) -event->angleDelta().y() / 500);

            double x = xAxis->pixelToCoord(event->pos().x());

            QCPRange range = xAxis->range();

            range = QCPRange(
                        x + (range.lower - x) * multiplier,
                        x + (range.upper - x) * multiplier);

            emit zoom(range);
        }
    }
    else
    {
        emit expand(event->pos(), event->angleDelta());
    }
}

void DataPlot::leaveEvent(
        QEvent *)
{
    emit clear();
}

void DataPlot::paintEvent(
        QPaintEvent *event)
{
    QCustomPlot::paintEvent(event);

    if (m_dragging == Zoom)
    {
        QPainter painter(this);

        painter.setPen(QPen(Qt::black));
        painter.drawLine(m_beginPos.x(), mAxisRect.top(), m_beginPos.x(), mAxisRect.bottom());
        if (mAxisRect.left() <= m_cursorPos.x() && m_cursorPos.x() <= mAxisRect.right())
        {
            painter.drawLine(m_cursorPos.x(), mAxisRect.top(), m_cursorPos.x(), mAxisRect.bottom());
        }

        QRect shading(
                    qMin(m_beginPos.x(), m_cursorPos.x()),
                    mAxisRect.top(),
                    qAbs(m_beginPos.x() - m_cursorPos.x()),
                    mAxisRect.height());

        painter.fillRect(shading & mAxisRect, QColor(181, 217, 42, 64));
    }
    else
    {
        if (mAxisRect.contains(m_cursorPos))
        {
            QPainter painter(this);

            painter.setPen(QPen(Qt::black));
            painter.drawLine(m_cursorPos.x(), mAxisRect.top(), m_cursorPos.x(), mAxisRect.bottom());
            painter.drawLine(mAxisRect.left(), m_cursorPos.y(), mAxisRect.right(), m_cursorPos.y());
        }
    }
}
