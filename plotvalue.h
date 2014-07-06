#ifndef PLOTVALUE_H
#define PLOTVALUE_H

#include <QColor>
#include <QString>

class PlotValue: public QObject
{
    Q_OBJECT

public:
    typedef enum
    {
        Metric,
        Imperial
    } Units;

    virtual const QString title(Units units) = 0;
    virtual const QColor color() = 0;
};

class PlotElevation: public PlotValue
{
public:
    PlotElevation() {}
    const QString title(Units units)
    {
        if (units == Metric) return tr("Elevation (m)") ;
        else                 return tr("Elevation (ft)") ;
    }
    const QColor color() { return Qt::black ; }
};

class PlotVerticalSpeed: public PlotValue
{
public:
    PlotVerticalSpeed() {}
    const QString title(Units units)
    {
        if (units == Metric) return tr("Vertical Speed (km/h)") ;
        else                 return tr("Vertical Speed (mph)") ;
    }
    const QColor color() { return Qt::red ; }
};

class PlotHorizontalSpeed: public PlotValue
{
public:
    PlotHorizontalSpeed() {}
    const QString title(Units units)
    {
        if (units == Metric) return tr("Horizontal Speed (km/h)") ;
        else                 return tr("Horizontal Speed (mph)") ;
    }
    const QColor color() { return Qt::green ; }
};

class PlotTotalSpeed: public PlotValue
{
public:
    PlotTotalSpeed() {}
    const QString title(Units units)
    {
        if (units == Metric) return tr("Total Speed (km/h)") ;
        else                 return tr("Total Speed (mph)") ;
    }
    const QColor color() { return Qt::blue ; }
};

class PlotDiveAngle: public PlotValue
{
public:
    PlotDiveAngle() {}
    const QString title(Units units) { return tr("Dive Angle (deg)") ; }
    const QColor color() { return Qt::cyan ; }
};

class PlotCurvature: public PlotValue
{
public:
    PlotCurvature() {}
    const QString title(Units units) { return tr("Curvature (deg/s)") ; }
    const QColor color() { return Qt::magenta ; }
};

class PlotGlideRatio: public PlotValue
{
public:
    PlotGlideRatio() {}
    const QString title(Units units) { return tr("Glide Ratio") ; }
    const QColor color() { return Qt::yellow ; }
};

#endif // PLOTVALUE_H
