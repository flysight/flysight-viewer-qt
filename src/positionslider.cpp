#include "positionslider.h"

#ifndef QT_NO_WHEELEVENT
    #include <QWheelEvent>
#endif

PositionSlider::PositionSlider(QWidget *parent) :
    QSlider(parent)
{
}

#ifndef QT_NO_WHEELEVENT
void PositionSlider::wheelEvent(QWheelEvent * e)
{
    QSlider::wheelEvent(e);

    if (e->isAccepted())
    {
        emit sliderMoved(sliderPosition());
    }
}
#endif
