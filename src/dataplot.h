#ifndef DATAPLOT_H
#define DATAPLOT_H

#include "qcustomplot.h"

class MainWindow;

class DataPlot : public QCustomPlot
{
    Q_OBJECT

public:
    typedef enum {
        Pan, Zoom, Measure, Zero, Ground
    } Tool;

    explicit DataPlot(QWidget *parent = 0);

    Tool tool() const { return m_tool; }
    void setTool(Tool tool) { m_tool = tool; }

    void setMainWindow(MainWindow *mainWindow) { mMainWindow = mainWindow; }

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void wheelEvent(QWheelEvent *event);

    void leaveEvent(QEvent *);

    void paintEvent(QPaintEvent *event);

private:
    QPoint m_cursorPos;
    QPoint m_beginPos;

    bool m_dragging;
    Tool m_tool;

    MainWindow *mMainWindow;

    void updateYRanges();

signals:
    void zoom(const QCPRange &range);
    void pan(double xBegin, double xEnd);
    void measure(double xBegin, double xEnd);
    void zero(double xMark);
    void ground(double xMark);

    void mark(double xMark);
    void clear();

    void expand(QPoint pos, QPoint angleDelta);

public slots:
    void setRange(const QCPRange &range);
};

#endif // DATAPLOT_H
