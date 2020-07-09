/***************************************************************************
**                                                                        **
**  FlySight Viewer                                                       **
**  Copyright 2018 Michael Cooper                                         **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>. **
**                                                                        **
****************************************************************************
**  Contact: Michael Cooper                                               **
**  Website: http://flysight.ca/                                          **
****************************************************************************/

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
    // Return now if plot empty
    if (mMainWindow->dataSize() == 0) return;

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

    // Get unit text and factor
    QString unitText = PlotDistance2D().unitText(mMainWindow->units());
    const double factor = PlotDistance2D().factor(mMainWindow->units());

    ui->bottomUnits->setText(unitText);
    ui->laneWidthUnits->setText(unitText);
    ui->laneLengthUnits->setText(unitText);

    ui->bottomEdit->setText(QString("%1").arg(bottom * factor, 0, 'f', 0));
    ui->laneWidthEdit->setText(QString("%1").arg(laneWidth * factor, 0, 'f', 0));
    ui->laneLengthEdit->setText(QString("%1").arg(laneLength * factor, 0, 'f', 0));

    ui->distanceUnits->setText((mMainWindow->units() == PlotValue::Metric) ? tr("km") : tr("mi"));

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
        if (topDist > bottomDist) s12 = topDist;
        else                      s12 = laneLength - bottomDist;

        ui->distanceEdit->setText(QString("%1").arg(
                                      (mMainWindow->units() == PlotValue::Metric) ?
                                          s12 / 1000 :
                                          s12 * METERS_TO_FEET / 5280,
                                      0, 'f', 3));
    }
}

void WideOpenDistanceForm::onApplyButtonClicked()
{
    const double endLatitude = ui->endLatitudeEdit->text().toDouble();
    const double endLongitude = ui->endLongitudeEdit->text().toDouble();
    const double bearing = ui->bearingEdit->text().toDouble();

    // Get factor
    const double factor = PlotDistance2D().factor(mMainWindow->units());

    const double bottom = ui->bottomEdit->text().toDouble() / factor;
    const double laneWidth = ui->laneWidthEdit->text().toDouble() / factor;
    const double laneLength = ui->laneLengthEdit->text().toDouble() / factor;

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
    mMainWindow->setMapMode(MainWindow::SetStart);
    setFocus();
}

void WideOpenDistanceForm::onEndButtonClicked()
{
    mMainWindow->setMapMode(MainWindow::SetEnd);
    setFocus();
}

void WideOpenDistanceForm::keyPressEvent(
        QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        // Cancel map selection
        mMainWindow->setMapMode(MainWindow::Default);

        // Reset display
        updateView();
    }

    QWidget::keyPressEvent(event);
}
