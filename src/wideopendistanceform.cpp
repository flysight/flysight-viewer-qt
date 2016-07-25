#include "wideopendistanceform.h"
#include "ui_wideopendistanceform.h"

#include "GeographicLib/Constants.hpp"
#include "GeographicLib/Geodesic.hpp"

#include "geographicutil.h"
#include "mainwindow.h"
#include "wideopendistancescoring.h"

using namespace GeographicLib;
using namespace GeographicUtil;

WideOpenDistanceForm::WideOpenDistanceForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WideOpenDistanceForm),
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

WideOpenDistanceForm::~WideOpenDistanceForm()
{
    delete ui;
}

QSize WideOpenDistanceForm::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void WideOpenDistanceForm::setMainWindow(
        MainWindow *mainWindow)
{
    mMainWindow = mainWindow;
}

void WideOpenDistanceForm::updateView()
{
    WideOpenDistanceScoring *method = (WideOpenDistanceScoring *) mMainWindow->scoringMethod(MainWindow::WideOpenDistance);

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

    if (mMainWindow->dataSize() == 0)
    {
        // Update display
        ui->distanceEdit->setText(tr("no data"));
    }
    else if (dp0.z < bottom)
    {
        // Update display
        ui->distanceEdit->setText(tr("set exit"));
    }
    else if (!success)
    {
        // Update display
        ui->distanceEdit->setText(tr("incomplete data"));
    }
    else
    {
        // Get projected point
        double lat0, lon0;
        intercept(dpTop.lat, dpTop.lon, endLatitude, endLongitude, dpBottom.lat, dpBottom.lon, lat0, lon0);

        // Distance from top
        double topDist;
        Geodesic::WGS84().Inverse(dpTop.lat, dpTop.lon, lat0, lon0, topDist);

        // Distance from bottom
        double bottomDist;
        Geodesic::WGS84().Inverse(endLatitude, endLongitude, lat0, lon0, bottomDist);

        double s12;
        if (topDist > bottomDist)
        {
            ui->distanceEdit->setText(QString("%1").arg(topDist / 1000, 0, 'f', 3));
        }
        else
        {
            ui->distanceEdit->setText(QString("%1").arg((laneLength - bottomDist) / 1000, 0, 'f', 3));
        }
    }
}

void WideOpenDistanceForm::onApplyButtonClicked()
{
    const double endLatitude = ui->endLatitudeEdit->text().toDouble();
    const double endLongitude = ui->endLongitudeEdit->text().toDouble();

    const double bearing = ui->bearingEdit->text().toDouble();

    const double bottom = ui->bottomEdit->text().toDouble();
    const double laneWidth = ui->laneWidthEdit->text().toDouble();
    const double laneLength = ui->laneLengthEdit->text().toDouble();

    WideOpenDistanceScoring *method = (WideOpenDistanceScoring *) mMainWindow->scoringMethod(MainWindow::WideOpenDistance);
    method->setEnd(endLatitude, endLongitude);
    method->setBearing(bearing);
    method->setBottom(bottom);
    method->setLaneWidth(laneWidth);
    method->setLaneLength(laneLength);

    mMainWindow->setFocus();
}

void WideOpenDistanceForm::onStartButtonClicked()
{
    WideOpenDistanceScoring *method = (WideOpenDistanceScoring *) mMainWindow->scoringMethod(MainWindow::WideOpenDistance);
    method->setMapMode(WideOpenDistanceScoring::Start);
    setFocus();
}

void WideOpenDistanceForm::onEndButtonClicked()
{
    WideOpenDistanceScoring *method = (WideOpenDistanceScoring *) mMainWindow->scoringMethod(MainWindow::WideOpenDistance);
    method->setMapMode(WideOpenDistanceScoring::End);
    setFocus();
}

void WideOpenDistanceForm::keyPressEvent(
        QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        // Cancel map selection
        WideOpenDistanceScoring *method = (WideOpenDistanceScoring *) mMainWindow->scoringMethod(MainWindow::WideOpenDistance);
        method->setMapMode(WideOpenDistanceScoring::None);

        // Reset display
        updateView();

        // Release focus
        mMainWindow->setFocus();
    }

    QWidget::keyPressEvent(event);
}
