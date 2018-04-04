#ifndef ORTHOVIEW_H
#define ORTHOVIEW_H

#include "QCustomPlot/qcustomplot.h"

class MainWindow;
class QTimer;

class OrthoView : public QCustomPlot
{
    Q_OBJECT

public:
    explicit OrthoView(QWidget *parent = 0);

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow) { mMainWindow = mainWindow; }

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    MainWindow *mMainWindow;

    QPoint      m_beginPos;
    bool        m_pan;

    double      m_azimuth;
    double      m_elevation;
    double      m_scale;

    QTimer     *m_timer;

    void addOrientation();
    void setViewRange(double xMin, double xMax,
                      double yMin, double yMax);

public slots:
    void updateView();
    void endTimer();
};

#endif // ORTHOVIEW_H
