#ifndef PLOTVALUE_H
#define PLOTVALUE_H

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

    QCPAxis *addAxis(QCustomPlot *plot, Units units) const
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
        if (units == Metric) return DataPoint::elevation(dp);
        else                 return DataPoint::elevation(dp) * METERS_TO_FEET;
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
        if (units == Metric) return DataPoint::verticalSpeed(dp) * MPS_TO_KMH;
        else                 return DataPoint::verticalSpeed(dp) * MPS_TO_MPH;
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
        if (units == Metric) return DataPoint::horizontalSpeed(dp) * MPS_TO_KMH;
        else                 return DataPoint::horizontalSpeed(dp) * MPS_TO_MPH;
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
        if (units == Metric) return DataPoint::totalSpeed(dp) * MPS_TO_KMH;
        else                 return DataPoint::totalSpeed(dp) * MPS_TO_MPH;
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
        return DataPoint::diveAngle(dp);
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
        return DataPoint::curvature(dp);
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
        return DataPoint::glideRatio(dp);
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
        if (units == Metric) return dp.DataPoint::horizontalAccuracy(dp);
        else                 return dp.DataPoint::horizontalAccuracy(dp) * METERS_TO_FEET;
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
        if (units == Metric) return DataPoint::verticalAccuracy(dp);
        else                 return DataPoint::verticalAccuracy(dp) * METERS_TO_FEET;
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
        if (units == Metric) return DataPoint::speedAccuracy(dp) * MPS_TO_KMH;
        else                 return DataPoint::speedAccuracy(dp) * MPS_TO_MPH;
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
        return DataPoint::numberOfSatellites(dp);
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
        return DataPoint::time(dp);
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
        if (units == Metric) return DataPoint::distance2D(dp);
        else                 return DataPoint::distance2D(dp) * METERS_TO_FEET;
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
        if (units == Metric) return DataPoint::distance3D(dp);
        else                 return DataPoint::distance3D(dp) * METERS_TO_FEET;
    }
};

class PlotWindSpeed: public PlotValue
{
    Q_OBJECT

public:
    PlotWindSpeed() {}
    const QString title(Units units) const
    {
        if (units == Metric) return tr("Wind Speed (km/h)");
        else                 return tr("Wind Speed (mph)");
    }
    const QColor color() const { return Qt::darkRed; }
    double value(const DataPoint &dp, Units units) const
    {
        if (units == Metric) return DataPoint::windSpeed(dp) * MPS_TO_KMH;
        else                 return DataPoint::windSpeed(dp) * MPS_TO_MPH;
    }
};

class PlotWindDirection: public PlotValue
{
    Q_OBJECT

public:
    PlotWindDirection() {}
    const QString title(Units units) const
    {
        Q_UNUSED(units);
        return tr("Wind Direction (deg)");
    }
    const QColor color() const { return Qt::darkGreen; }
    double value(const DataPoint &dp, Units units) const
    {
        Q_UNUSED(units);
        return DataPoint::windDirection(dp);
    }
};

class PlotAircraftSpeed: public PlotValue
{
    Q_OBJECT

public:
    PlotAircraftSpeed() {}
    const QString title(Units units) const
    {
        if (units == Metric) return tr("Aircraft Speed (km/h)");
        else                 return tr("Aircraft Speed (mph)");
    }
    const QColor color() const { return Qt::darkBlue; }
    double value(const DataPoint &dp, Units units) const
    {
        if (units == Metric) return DataPoint::aircraftSpeed(dp) * MPS_TO_KMH;
        else                 return DataPoint::aircraftSpeed(dp) * MPS_TO_MPH;
    }
};

class PlotWindError: public PlotValue
{
    Q_OBJECT

public:
    PlotWindError() {}
    const QString title(Units units) const
    {
        if (units == Metric) return tr("Wind Error (km/h)");
        else                 return tr("Wind Error (mph)");
    }
    const QColor color() const { return Qt::lightGray; }
    double value(const DataPoint &dp, Units units) const
    {
        if (units == Metric) return DataPoint::windError(dp) * MPS_TO_KMH;
        else                 return DataPoint::windError(dp) * MPS_TO_MPH;
    }
};

class PlotAcceleration: public PlotValue
{
    Q_OBJECT

public:
    PlotAcceleration() {}
    const QString title(Units units) const
    {
        Q_UNUSED(units);
        return tr("Acceleration (m/s^2)");
    }
    const QColor color() const { return Qt::darkRed; }
    double value(const DataPoint &dp, Units units) const
    {
        Q_UNUSED(units);
        return DataPoint::acceleration(dp);
    }
};

class PlotTotalEnergy: public PlotValue
{
    Q_OBJECT

public:
    PlotTotalEnergy() {}
    const QString title(Units units) const
    {
        Q_UNUSED(units);
        return tr("Total Energy (J/kg)");
    }
    const QColor color() const { return Qt::darkGreen; }
    double value(const DataPoint &dp, Units units) const
    {
        Q_UNUSED(units);
        return DataPoint::totalEnergy(dp);
    }
};

class PlotEnergyRate: public PlotValue
{
    Q_OBJECT

public:
    PlotEnergyRate() {}
    const QString title(Units units) const
    {
        Q_UNUSED(units);
        return tr("Energy Rate (J/kg/s)");
    }
    const QColor color() const { return Qt::darkBlue; }
    double value(const DataPoint &dp, Units units) const
    {
        Q_UNUSED(units);
        return DataPoint::energyRate(dp);
    }
};

class PlotLift: public PlotValue
{
    Q_OBJECT

public:
    PlotLift() {}
    const QString title(Units units) const
    {
        Q_UNUSED(units);
        return tr("Lift Coefficient");
    }
    const QColor color() const { return Qt::darkGreen; }
    double value(const DataPoint &dp, Units units) const
    {
        Q_UNUSED(units);
        return DataPoint::liftCoefficient(dp);
    }
};

class PlotDrag: public PlotValue
{
    Q_OBJECT

public:
    PlotDrag() {}
    const QString title(Units units) const
    {
        Q_UNUSED(units);
        return tr("Drag Coefficient");
    }
    const QColor color() const { return Qt::darkBlue; }
    double value(const DataPoint &dp, Units units) const
    {
        Q_UNUSED(units);
        return DataPoint::dragCoefficient(dp);
    }
};

#endif // PLOTVALUE_H
