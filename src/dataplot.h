#ifndef DATAPLOT_H
#define DATAPLOT_H

#include "qcustomplot.h"

class MainWindow;

class DataPlot : public QCustomPlot
{
    Q_OBJECT

public:
    explicit DataPlot(QWidget *parent = 0);

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

    MainWindow *mMainWindow;

    void updateYRanges();

signals:
    void measure(double xBegin, double xEnd);

    void mark(double xMark);
    void clear();

public slots:
    void setRange(const QCPRange &range);
    void updatePlot();
};

#endif // DATAPLOT_H
