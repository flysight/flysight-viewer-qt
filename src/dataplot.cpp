#include "dataplot.h"

DataPlot::DataPlot(QWidget *parent) :
    QCustomPlot(parent),
    m_dragging(false),
    m_tool(Pan)
{
    setMouseTracking(true);
}

void DataPlot::mousePressEvent(
        QMouseEvent *event)
{
    if (axisRect()->rect().contains(event->pos()))
    {
        m_beginPos = event->pos();
        m_dragging = true;
        update();
    }

    QCustomPlot::mousePressEvent(event);
}

void DataPlot::mouseReleaseEvent(
        QMouseEvent *event)
{
    QPoint endPos = event->pos();

    if (m_dragging && m_tool == Zoom)
    {
        QCPRange range(qMin(xAxis->pixelToCoord(m_beginPos.x()),
                            xAxis->pixelToCoord(endPos.x())),
                       qMax(xAxis->pixelToCoord(m_beginPos.x()),
                            xAxis->pixelToCoord(endPos.x())));

        emit zoom(range);
    }
    if (m_dragging && m_tool == Zero)
    {
        emit zero(xAxis->pixelToCoord(endPos.x()));
    }
    if (m_dragging && m_tool == Ground)
    {
        emit ground(xAxis->pixelToCoord(endPos.x()));
    }

    if (m_dragging)
    {
        m_dragging = false;
        replot();
    }

    QCustomPlot::mouseReleaseEvent(event);
}

void DataPlot::mouseMoveEvent(
        QMouseEvent *event)
{
    m_cursorPos = event->pos();

    if (m_dragging && m_tool == Pan)
    {
        emit pan(xAxis->pixelToCoord(m_beginPos.x()),
                 xAxis->pixelToCoord(m_cursorPos.x()));

        m_beginPos = m_cursorPos;
    }

    if (axisRect()->rect().contains(event->pos()))
    {
        if (m_dragging && m_tool == Measure)
        {
            emit measure(xAxis->pixelToCoord(m_beginPos.x()),
                         xAxis->pixelToCoord(m_cursorPos.x()));
        }
        else
        {
            emit mark(xAxis->pixelToCoord(m_cursorPos.x()));
        }
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
        if (axisRect()->rect().contains(event->pos()))
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

    if (m_dragging && (m_tool == Zoom || m_tool == Measure))
    {
        QPainter painter(this);

        painter.setPen(QPen(Qt::black));
        painter.drawLine(m_beginPos.x(), axisRect()->rect().top(), m_beginPos.x(), axisRect()->rect().bottom());
        if (axisRect()->rect().left() <= m_cursorPos.x() && m_cursorPos.x() <= axisRect()->rect().right())
        {
            painter.drawLine(m_cursorPos.x(), axisRect()->rect().top(), m_cursorPos.x(), axisRect()->rect().bottom());
        }

        QRect shading(
                    qMin(m_beginPos.x(), m_cursorPos.x()),
                    axisRect()->rect().top(),
                    qAbs(m_beginPos.x() - m_cursorPos.x()),
                    axisRect()->rect().height());

        painter.fillRect(shading & axisRect()->rect(), QColor(181, 217, 42, 64));
    }
    else
    {
        if (axisRect()->rect().contains(m_cursorPos))
        {
            QPainter painter(this);

            painter.setPen(QPen(Qt::black));
            painter.drawLine(m_cursorPos.x(), axisRect()->rect().top(), m_cursorPos.x(), axisRect()->rect().bottom());
            painter.drawLine(axisRect()->rect().left(), m_cursorPos.y(), axisRect()->rect().right(), m_cursorPos.y());
        }
    }
}
