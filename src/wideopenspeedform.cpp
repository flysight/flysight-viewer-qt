#include "wideopenspeedform.h"
#include "ui_wideopenspeedform.h"

#include "GeographicLib/Constants.hpp"
#include "GeographicLib/Geodesic.hpp"
#include "GeographicLib/Gnomonic.hpp"
#include "GeographicLib/Rhumb.hpp"

#include "mainwindow.h"
#include "wideopenspeedscoring.h"

using namespace GeographicLib;

WideOpenSpeedForm::WideOpenSpeedForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WideOpenSpeedForm),
    mMainWindow(0)
{
    ui->setupUi(this);

    connect(ui->endLatitudeEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
    connect(ui->endLongitudeEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
    connect(ui->bearingEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
    connect(ui->bottomEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
    connect(ui->laneWidthEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
    connect(ui->laneLengthEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));

    // Connect start/end buttons
    connect(ui->startGlobeButton, SIGNAL(clicked()), this, SLOT(onStartButtonClicked()));
    connect(ui->endGlobeButton, SIGNAL(clicked()), this, SLOT(onEndButtonClicked()));
}

WideOpenSpeedForm::~WideOpenSpeedForm()
{
    delete ui;
}

QSize WideOpenSpeedForm::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void WideOpenSpeedForm::setMainWindow(
        MainWindow *mainWindow)
{
    mMainWindow = mainWindow;
}

void WideOpenSpeedForm::updateView()
{
    WideOpenSpeedScoring *method = (WideOpenSpeedScoring *) mMainWindow->scoringMethod(MainWindow::WideOpenSpeed);

    const double endLatitude = method->endLatitude();
    const double endLongitude = method->endLongitude();

    const double bearing = method->bearing();

    const double bottom = method->bottom();
    const double laneWidth = method->laneWidth();
    const double laneLength = method->laneLength();

    // Update display
    ui->endLatitudeEdit->setText(QString("%1").arg(endLatitude, 0, 'f', 7));
    ui->endLongitudeEdit->setText(QString("%1").arg(endLongitude, 0, 'f', 7));

    ui->bearingEdit->setText(QString("%1").arg(bearing, 0, 'f', 5));

    ui->bottomEdit->setText(QString("%1").arg(bottom));
    ui->laneWidthEdit->setText(QString("%1").arg(laneWidth));
    ui->laneLengthEdit->setText(QString("%1").arg(laneLength));

    // Find reference point for distance
    DataPoint dpTop;
    dpTop.hasGeodetic = true;
    Geodesic::WGS84().Direct(endLatitude, endLongitude, bearing, laneLength, dpTop.lat, dpTop.lon);

    // Find exit point
    DataPoint dp0 = mMainWindow->interpolateDataT(0);

    // Find where we cross the bottom
    DataPoint dpBottom;
    bool success = method->getWindowBounds(mMainWindow->data(), dpBottom);

    if (dp0.z < bottom)
    {
        // Update display
        ui->speedEdit->setText(tr("exit below bottom"));
    }
    else if (!success)
    {
        // Update display
        ui->speedEdit->setText(tr("ends above bottom"));
    }
    else
    {
        // Calculate time
        int i, start = mMainWindow->findIndexBelowT(0) + 1;
        double d1, t;
        DataPoint dp1;

        for (i = start; i < mMainWindow->dataSize(); ++i)
        {
            const DataPoint &dp2 = mMainWindow->dataPoint(i);

            // Calculate distance
            double lat0, lon0;
            intercept(dpTop.lat, dpTop.lon, endLatitude, endLongitude, dp2.lat, dp2.lon, lat0, lon0);

            double d2;
            Geodesic::WGS84().Inverse(dpTop.lat, dpTop.lon, lat0, lon0, d2);

            if (i > start && d1 < laneLength && d2 >= laneLength)
            {
                t = dp1.t + (dp2.t - dp1.t) / (d2 - d1) * (laneLength - d1);
                break;
            }

            d1 = d2;
            dp1 = dp2;
        }

        if (i < mMainWindow->dataSize())
        {
            DataPoint dp = mMainWindow->interpolateDataT(t);

            if (dp.t < dpBottom.t)
            {
                ui->speedEdit->setText(dp.dateTime.toString("hh:mm:ss.zzz"));
            }
            else
            {
                ui->speedEdit->setText(tr("hit bottom first"));
            }
        }
        else
        {
            ui->speedEdit->setText(tr("did not finish"));
        }
    }
}

// Compute the geodesic intercept from a point at (latb1, lonb1) to a geodesic
// between (lata1, lona1) and (lata2, lona2).
//
// From https://sourceforge.net/p/geographiclib/discussion/1026621/thread/21aaff9f/#8a93
// See http://arxiv.org/pdf/1102.1215v1.pdf

void WideOpenSpeedForm::intercept(
        double lata1,
        double lona1,
        double lata2,
        double lona2,
        double latb1,
        double lonb1,
        double &lat0,
        double &lon0)
{
    class vector3 {
    public:
        double _x, _y, _z;
        vector3(double x, double y, double z = 1) throw()
            : _x(x)
            , _y(y)
            , _z(z) {}
        vector3 cross(const vector3& b) const throw() {
            return vector3(_y * b._z - _z * b._y,
                _z * b._x - _x * b._z,
                _x * b._y - _y * b._x);
        }
        void norm() throw() {
            _x /= _z;
            _y /= _z;
            _z = 1;
        }
    };

    const Geodesic geod = Geodesic::WGS84();
    const Gnomonic gn(geod);

    // Possibly need to deal with longitudes wrapping around
    lat0 = (lata1 + lata2) / 2;
    lon0 = (lona1 + lona2) / 2;

    for (int i = 0; i < 10; ++i)
    {
        double xa1, ya1, xa2, ya2;
        double xb1, yb1;

        // Convert to Gnomonic projection
        gn.Forward(lat0, lon0, lata1, lona1, xa1, ya1);
        gn.Forward(lat0, lon0, lata2, lona2, xa2, ya2);
        gn.Forward(lat0, lon0, latb1, lonb1, xb1, yb1);

        // See Hartley and Zisserman, Multiple View Geometry, Sec. 2.2.1
        vector3 va1(xa1, ya1);
        vector3 va2(xa2, ya2);

        // la is homogeneous representation of line A1,A2
        vector3 la = va1.cross(va2);

        // lb is homogeneous representation of line thru B1 perpendicular to la
        vector3 lb(la._y, -la._x, la._x * yb1 - la._y * xb1);

        // p0 is homogeneous representation of intersection of la and lb
        vector3 p0 = la.cross(lb);
        p0.norm();

        double lat1, lon1;
        gn.Reverse(lat0, lon0, p0._x, p0._y, lat1, lon1);

        lat0 = lat1;
        lon0 = lon1;
    }
}

void WideOpenSpeedForm::onApplyButtonClicked()
{
    const double endLatitude = ui->endLatitudeEdit->text().toDouble();
    const double endLongitude = ui->endLongitudeEdit->text().toDouble();

    const double bearing = ui->bearingEdit->text().toDouble();

    const double bottom = ui->bottomEdit->text().toDouble();
    const double laneWidth = ui->laneWidthEdit->text().toDouble();
    const double laneLength = ui->laneLengthEdit->text().toDouble();

    WideOpenSpeedScoring *method = (WideOpenSpeedScoring *) mMainWindow->scoringMethod(MainWindow::WideOpenSpeed);
    method->setEnd(endLatitude, endLongitude);
    method->setBearing(bearing);
    method->setBottom(bottom);
    method->setLaneWidth(laneWidth);
    method->setLaneLength(laneLength);

    mMainWindow->setFocus();
}

void WideOpenSpeedForm::onStartButtonClicked()
{
    WideOpenSpeedScoring *method = (WideOpenSpeedScoring *) mMainWindow->scoringMethod(MainWindow::WideOpenSpeed);
    method->setMapMode(WideOpenSpeedScoring::Start);
    setFocus();
}

void WideOpenSpeedForm::onEndButtonClicked()
{
    WideOpenSpeedScoring *method = (WideOpenSpeedScoring *) mMainWindow->scoringMethod(MainWindow::WideOpenSpeed);
    method->setMapMode(WideOpenSpeedScoring::End);
    setFocus();
}

void WideOpenSpeedForm::keyPressEvent(
        QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        // Cancel map selection
        WideOpenSpeedScoring *method = (WideOpenSpeedScoring *) mMainWindow->scoringMethod(MainWindow::WideOpenSpeed);
        method->setMapMode(WideOpenSpeedScoring::None);

        // Reset display
        updateView();

        // Release focus
        mMainWindow->setFocus();
    }

    QWidget::keyPressEvent(event);
}
