#ifndef ORTHOVIEW_H
#define ORTHOVIEW_H

#include "qcustomplot.h"

class MainWindow;

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

private:
    MainWindow *mMainWindow;

    QPoint      m_beginPos;
    bool        m_pan;

    double      m_azimuth;
    double      m_elevation;

    void setViewRange(double xMin, double xMax,
                      double yMin, double yMax);

public slots:
    void updateView();
};

#endif // ORTHOVIEW_H
