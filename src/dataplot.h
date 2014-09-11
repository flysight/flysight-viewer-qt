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
    void setRange(const QCPRange &range);

public slots:
    void setRange(double lower, double upper);
    void updatePlot();
};

#endif // DATAPLOT_H
