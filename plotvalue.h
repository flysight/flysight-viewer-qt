#ifndef PLOTVALUE_H
#define PLOTVALUE_H

#include <QColor>
#include <QString>

class PlotValue: public QObject
{
    Q_OBJECT

public:
    virtual const QString titleMetric() = 0;
    virtual const QString titleImperial() = 0;
    virtual const QColor  color() = 0;
};

class PlotElevation: public PlotValue
{
public:
    PlotElevation() {}
    const QString titleMetric() { return tr("Elevation (m)") ; }
    const QString titleImperial() { return tr("Elevation (ft)") ; }
    const QColor  color() { return Qt::black ; }
};

class PlotVerticalSpeed: public PlotValue
{
public:
    PlotVerticalSpeed() {}
    const QString titleMetric() { return tr("Vertical Speed (km/h)") ; }
    const QString titleImperial() { return tr("Vertical Speed (mph)") ; }
    const QColor  color() { return Qt::red ; }
};

class PlotHorizontalSpeed: public PlotValue
{
public:
    PlotHorizontalSpeed() {}
    const QString titleMetric() { return tr("Horizontal Speed (km/h)") ; }
    const QString titleImperial() { return tr("Horizontal Speed (mph)") ; }
    const QColor  color() { return Qt::green ; }
};

class PlotTotalSpeed: public PlotValue
{
public:
    PlotTotalSpeed() {}
    const QString titleMetric() { return tr("Total Speed (km/h)") ; }
    const QString titleImperial() { return tr("Total Speed (mph)") ; }
    const QColor  color() { return Qt::blue ; }
};

class PlotDiveAngle: public PlotValue
{
public:
    PlotDiveAngle() {}
    const QString titleMetric() { return tr("Dive Angle (deg)") ; }
    const QString titleImperial() { return tr("Dive Angle (deg)") ; }
    const QColor  color() { return Qt::cyan ; }
};

class PlotCurvature: public PlotValue
{
public:
    PlotCurvature() {}
    const QString titleMetric() { return tr("Curvature (deg/s)") ; }
    const QString titleImperial() { return tr("Curvature (deg/s)") ; }
    const QColor  color() { return Qt::magenta ; }
};

class PlotGlideRatio: public PlotValue
{
public:
    PlotGlideRatio() {}
    const QString titleMetric() { return tr("Glide Ratio") ; }
    const QString titleImperial() { return tr("Glide Ratio") ; }
    const QColor  color() { return Qt::yellow ; }
};

#endif // PLOTVALUE_H
