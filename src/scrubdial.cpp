#include "scrubdial.h"

#ifndef QT_NO_WHEELEVENT
    #include <QWheelEvent>
#endif

ScrubDial::ScrubDial(QWidget *parent) :
    QDial(parent)
{
}

#ifndef QT_NO_WHEELEVENT
void ScrubDial::wheelEvent(QWheelEvent * e)
{
    QDial::wheelEvent(e);

    if (e->isAccepted())
    {
        emit sliderMoved(sliderPosition());
    }
}
#endif
