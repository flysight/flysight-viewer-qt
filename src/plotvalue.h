#ifndef PLOTVALUE_H
#define PLOTVALUE_H

#include <math.h>

#include <QColor>
#include <QSettings>
#include <QString>

#include "datapoint.h"
#include "qcustomplot.h"

#define METERS_TO_FEET 3.28084
#define MPS_TO_MPH     2.23694
#define MPS_TO_KMH     3.6

class PlotValue: public QObject
{
    Q_OBJECT

public:
    typedef enum
    {
        Metric = 0,
        Imperial
    } Units;

    PlotValue(bool visible = false): mVisible(visible) {}

    virtual const QString title(Units units) const = 0;
    virtual const QColor color() const = 0;
    virtual double value(const DataPoint &dp, Units units) const = 0;

    QCPAxis *addAxis(QCustomPlot *plot, Units units)
    {
        QCPAxis *axis = plot->axisRect()->addAxis(QCPAxis::atLeft);
        axis->setLabelColor(color());
        axis->setTickLabelColor(color());
        axis->setBasePen(QPen(color()));
        axis->setTickPen(QPen(color()));
        axis->setSubTickPen(QPen(color()));
        axis->setLabel(title(units));
        return axis;
    }

    bool visible() const { return mVisible; }
    void setVisible(bool visible) { mVisible = visible; }

    void readSettings()
    {
        QSettings settings("FlySight", "Viewer");
        settings.beginGroup("plotValue/" + key());
        mVisible = settings.value("visible", mVisible).toBool();
        settings.endGroup();
    }

    void writeSettings() const
    {
        QSettings settings("FlySight", "Viewer");
        settings.beginGroup("plotValue/" + key());
        settings.setValue("visible", mVisible);
        settings.endGroup();
    }

private:
    bool mVisible;

    const QString key() const
    {
        return metaObject()->className();
    }
};

class PlotElevation: public PlotValue
{
    Q_OBJECT

public:
    PlotElevation(): PlotValue(true) {}
    const QString title(Units units) const
    {
        if (units == Metric) return tr("Elevation (m)");
        else                 return tr("Elevation (ft)");
    }
    const QColor color() const { return Qt::black; }
    double value(const DataPoint &dp, Units units) const
    {
        if (units == Metric) return dp.alt;
        else                 return dp.alt * METERS_TO_FEET;
    }
};

class PlotVerticalSpeed: public PlotValue
{
    Q_OBJECT

public:
    PlotVerticalSpeed() {}
    const QString title(Units units) const
    {
        if (units == Metric) return tr("Vertical Speed (km/h)");
        else                 return tr("Vertical Speed (mph)");
    }
    const QColor color() const { return Qt::green; }
    double value(const DataPoint &dp, Units units) const
    {
        if (units == Metric) return dp.velD * MPS_TO_KMH;
        else                 return dp.velD * MPS_TO_MPH;
    }
};

class PlotHorizontalSpeed: public PlotValue
{
    Q_OBJECT

public:
    PlotHorizontalSpeed() {}
    const QString title(Units units) const
    {
        if (units == Metric) return tr("Horizontal Speed (km/h)");
        else                 return tr("Horizontal Speed (mph)");
    }
    const QColor color() const { return Qt::red; }
    double value(const DataPoint &dp, Units units) const
    {
        if (units == Metric) return sqrt(dp.velE * dp.velE + dp.velN * dp.velN) * MPS_TO_KMH;
        else                 return sqrt(dp.velE * dp.velE + dp.velN * dp.velN) * MPS_TO_MPH;
    }
};

class PlotTotalSpeed: public PlotValue
{
    Q_OBJECT

public:
    PlotTotalSpeed() {}
    const QString title(Units units) const
    {
        if (units == Metric) return tr("Total Speed (km/h)");
        else                 return tr("Total Speed (mph)");
    }
    const QColor color() const { return Qt::blue; }
    double value(const DataPoint &dp, Units units) const
    {
        if (units == Metric) return sqrt(dp.velE * dp.velE + dp.velN * dp.velN + dp.velD * dp.velD) * MPS_TO_KMH;
        else                 return sqrt(dp.velE * dp.velE + dp.velN * dp.velN + dp.velD * dp.velD) * MPS_TO_MPH;
    }
};

class PlotDiveAngle: public PlotValue
{
    Q_OBJECT

public:
    PlotDiveAngle() {}
    const QString title(Units units) const
    {
        Q_UNUSED(units);
        return tr("Dive Angle (deg)");
    }
    const QColor color() const { return Qt::magenta; }
    double value(const DataPoint &dp, Units units) const
    {
        Q_UNUSED(units);
        const double pi = 3.14159265359;
        return atan2(dp.velD, sqrt(dp.velE * dp.velE + dp.velN * dp.velN)) / pi * 180;
    }
};

class PlotCurvature: public PlotValue
{
    Q_OBJECT

public:
    PlotCurvature() {}
    const QString title(Units units) const
    {
        Q_UNUSED(units);
        return tr("Dive Rate (deg/s)");
    }
    const QColor color() const { return Qt::darkYellow; }
    double value(const DataPoint &dp, Units units) const
    {
        Q_UNUSED(units);
        return dp.curv;
    }
};

class PlotGlideRatio: public PlotValue
{
    Q_OBJECT

public:
    PlotGlideRatio() {}
    const QString title(Units units) const
    {
        Q_UNUSED(units);
        return tr("Glide Ratio");
    }
    const QColor color() const { return Qt::darkCyan; }
    double value(const DataPoint &dp, Units units) const
    {
        Q_UNUSED(units);
        if (dp.velD != 0) return sqrt(dp.velE * dp.velE + dp.velN * dp.velN) / dp.velD;
        else              return 0;
    }
};

class PlotHorizontalAccuracy: public PlotValue
{
    Q_OBJECT

public:
    PlotHorizontalAccuracy() {}
    const QString title(Units units) const
    {
        if (units == Metric) return tr("Horizontal Accuracy (m)");
        else                 return tr("Horizontal Accuracy (ft)");
    }
    const QColor color() const { return Qt::darkRed; }
    double value(const DataPoint &dp, Units units) const
    {
        if (units == Metric) return dp.hAcc;
        else                 return dp.hAcc * METERS_TO_FEET;
    }
};

class PlotVerticalAccuracy: public PlotValue
{
    Q_OBJECT

public:
    PlotVerticalAccuracy() {}
    const QString title(Units units) const
    {
        if (units == Metric) return tr("Vertical Accuracy (m)");
        else                 return tr("Vertical Accuracy (ft)");
    }
    const QColor color() const { return Qt::darkGreen; }
    double value(const DataPoint &dp, Units units) const
    {
        if (units == Metric) return dp.vAcc;
        else                 return dp.vAcc * METERS_TO_FEET;
    }
};

class PlotSpeedAccuracy: public PlotValue
{
    Q_OBJECT

public:
    PlotSpeedAccuracy() {}
    const QString title(Units units) const
    {
        if (units == Metric) return tr("Speed Accuracy (km/h)");
        else                 return tr("Speed Accuracy (mph)");
    }
    const QColor color() const { return Qt::darkBlue; }
    double value(const DataPoint &dp, Units units) const
    {
        if (units == Metric) return dp.sAcc * MPS_TO_KMH;
        else                 return dp.sAcc * MPS_TO_MPH;
    }
};

class PlotNumberOfSatellites: public PlotValue
{
    Q_OBJECT

public:
    PlotNumberOfSatellites() {}
    const QString title(Units units) const
    {
        Q_UNUSED(units);
        return tr("Number of Satellites");
    }
    const QColor color() const { return Qt::darkMagenta; }
    double value(const DataPoint &dp, Units units) const
    {
        Q_UNUSED(units);
        return dp.numSV;
    }
};

class PlotTime: public PlotValue
{
    Q_OBJECT

public:
    PlotTime() {}
    const QString title(Units units) const
    {
        Q_UNUSED(units);
        return tr("Time (s)");
    }
    const QColor color() const { return Qt::black; }
    double value(const DataPoint &dp, Units units) const
    {
        Q_UNUSED(units);
        return dp.t;
    }
};

class PlotDistance2D: public PlotValue
{
    Q_OBJECT

public:
    PlotDistance2D() {}
    const QString title(Units units) const
    {
        if (units == Metric) return tr("Horizontal Distance (m)");
        else                 return tr("Horizontal Distance (ft)");
    }
    const QColor color() const { return Qt::black; }
    double value(const DataPoint &dp, Units units) const
    {
        if (units == Metric) return dp.dist2D;
        else                 return dp.dist2D * METERS_TO_FEET;
    }
};

class PlotDistance3D: public PlotValue
{
    Q_OBJECT

public:
    PlotDistance3D() {}
    const QString title(Units units) const
    {
        if (units == Metric) return tr("Total Distance (m)");
        else                 return tr("Total Distance (ft)");
    }
    const QColor color() const { return Qt::black; }
    double value(const DataPoint &dp, Units units) const
    {
        if (units == Metric) return dp.dist3D;
        else                 return dp.dist3D * METERS_TO_FEET;
    }
};

#endif // PLOTVALUE_H
