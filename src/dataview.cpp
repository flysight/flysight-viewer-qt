#include "dataview.h"

DataView::DataView(QWidget *parent) :
    QCustomPlot(parent)
{

}

QSize DataView::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}
