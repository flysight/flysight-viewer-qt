#ifndef POSITIONSLIDER_H
#define POSITIONSLIDER_H

#include <QSlider>

class PositionSlider : public QSlider
{
    Q_OBJECT
public:
    explicit PositionSlider(QWidget *parent = 0);

protected:
#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *e);
#endif

signals:

public slots:

};

#endif // POSITIONSLIDER_H
