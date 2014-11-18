#ifndef SCRUBDIAL_H
#define SCRUBDIAL_H

#include <QDial>

class ScrubDial : public QDial
{
    Q_OBJECT
public:
    explicit ScrubDial(QWidget *parent = 0);

protected:
#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *e);
#endif

signals:

public slots:

};

#endif // SCRUBDIAL_H
