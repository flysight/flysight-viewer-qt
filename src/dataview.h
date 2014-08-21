#ifndef DATAVIEW_H
#define DATAVIEW_H

#include "qcustomplot.h"

class DataView : public QCustomPlot
{
    Q_OBJECT

public:
    explicit DataView(QWidget *parent = 0);

    virtual QSize sizeHint() const;

signals:
    
public slots:
    
};

#endif // DATAVIEW_H
