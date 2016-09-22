#include "wideopenspeedform.h"
#include "ui_wideopenspeedform.h"

#include "GeographicLib/Constants.hpp"
#include "GeographicLib/Geodesic.hpp"

#include "geographicutil.h"
#include "mainwindow.h"
#include "plotvalue.h"
#include "wideopenspeedscoring.h"

using namespace GeographicLib;
using namespace GeographicUtil;

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

    // Get unit text and factor
    QString unitText = PlotDistance2D().unitText(mMainWindow->units());
    const double factor = PlotDistance2D().factor(mMainWindow->units());

    ui->bottomUnits->setText(unitText);
    ui->laneWidthUnits->setText(unitText);
    ui->laneLengthUnits->setText(unitText);

    ui->bottomEdit->setText(QString("%1").arg(bottom * factor, 0, 'f', 0));
    ui->laneWidthEdit->setText(QString("%1").arg(laneWidth * factor, 0, 'f', 0));
    ui->laneLengthEdit->setText(QString("%1").arg(laneLength * factor, 0, 'f', 0));

    // Find reference point for distance
    DataPoint dpTop;
    dpTop.hasGeodetic = true;
    Geodesic::WGS84().Direct(endLatitude, endLongitude, bearing, laneLength, dpTop.lat, dpTop.lon);

    // Find exit point
    DataPoint dp0 = mMainWindow->interpolateDataT(0);

    // Get distance from exit point to reference
    double exitDist;
    Geodesic::WGS84().Inverse(endLatitude, endLongitude, dp0.lat, dp0.lon, exitDist);

    // Find where we cross the bottom
    DataPoint dpBottom;
    bool success = method->getWindowBounds(mMainWindow->data(), dpBottom);

    // Invalidate finish point
    method->invalidateFinish();

    if (mMainWindow->dataSize() == 0)
    {
        // Update display
        ui->speedEdit->setText(tr("no data"));
    }
    else if (dp0.z < bottom)
    {
        // Update display
        ui->speedEdit->setText(tr("set exit"));
    }
    else if (exitDist > laneLength * 10)
    {
        // Update display
        ui->speedEdit->setText(tr("set reference"));
    }
    else if (!success)
    {
        // Update display
        ui->speedEdit->setText(tr("incomplete data"));
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

            // Get projected point
            double lat0, lon0;
            intercept(dpTop.lat, dpTop.lon, endLatitude, endLongitude, dp2.lat, dp2.lon, lat0, lon0);

            // Distance from top
            double topDist;
            Geodesic::WGS84().Inverse(dpTop.lat, dpTop.lon, lat0, lon0, topDist);

            // Distance from bottom
            double bottomDist;
            Geodesic::WGS84().Inverse(endLatitude, endLongitude, lat0, lon0, bottomDist);

            double d2;
            if (topDist > bottomDist)
            {
                d2 = topDist;
            }
            else
            {
                d2 = laneLength - bottomDist;
            }

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
            method->setFinishPoint(dp);

            if (dp.t <= dpBottom.t)
            {
                ui->speedEdit->setText(dp.dateTime.toString("hh:mm:ss.zzz"));
            }
            else
            {
                ui->speedEdit->setText(tr("did not finish"));
            }
        }
        else
        {
            ui->speedEdit->setText(tr("did not finish"));
        }
    }
}

void WideOpenSpeedForm::onApplyButtonClicked()
{
    const double endLatitude = ui->endLatitudeEdit->text().toDouble();
    const double endLongitude = ui->endLongitudeEdit->text().toDouble();
    const double bearing = ui->bearingEdit->text().toDouble();

    // Get factor
    const double factor = PlotDistance2D().factor(mMainWindow->units());

    const double bottom = ui->bottomEdit->text().toDouble() / factor;
    const double laneWidth = ui->laneWidthEdit->text().toDouble() / factor;
    const double laneLength = ui->laneLengthEdit->text().toDouble() / factor;

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
