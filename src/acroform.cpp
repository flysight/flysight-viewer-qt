/***************************************************************************
**                                                                        **
**  FlySight Viewer                                                       **
**  Copyright 2018 Michael Cooper, Matt Coffin                            **
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

#include "acroform.h"
#include "ui_acroform.h"

#include "common.h"
#include "datapoint.h"
#include "mainwindow.h"
#include "plotvalue.h"
#include "acroscoring.h"

AcroForm::AcroForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AcroForm),
    mMainWindow(0)
{
    ui->setupUi(this);

    connect(ui->faiButton, SIGNAL(clicked()), this, SLOT(onFAIButtonClicked()));

    connect(ui->speedEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
    connect(ui->altitudeEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
}

AcroForm::~AcroForm()
{
    delete ui;
}

QSize AcroForm::sizeHint() const
{
    // Keeps windows from being initialized as very short
    return QSize(175, 175);
}

void AcroForm::setMainWindow(
        MainWindow *mainWindow)
{
    mMainWindow = mainWindow;
}

void AcroForm::updateView()
{
    if (mMainWindow->dataSize() == 0) return;

    AcroScoring *method = (AcroScoring *) mMainWindow->scoringMethod(MainWindow::Acro);

    const double speed = method->speed();
    const double altitude = method->altitude();

    // Update window bounds
    ui->speedEdit->setText(QString("%1").arg(speed));
    ui->altitudeEdit->setText(QString("%1").arg(altitude * METERS_TO_FEET));

    DataPoint dpBottom, dpTop;
    bool success = false;

    switch (mMainWindow->windowMode())
    {
    case MainWindow::Actual:
        success = method->getWindowBounds(mMainWindow->data(), dpBottom, dpTop);
        break;
    }

    if (success)
    {
        // Update display
        ui->timeEdit->setText(QString("%1").arg(dpBottom.t - dpTop.t, 0, 'f', 1));
    }
    else
    {
        // Update display
        ui->timeEdit->setText(tr("n/a"));
    }
}

void AcroForm::onFAIButtonClicked()
{
    AcroScoring *method = (AcroScoring *) mMainWindow->scoringMethod(MainWindow::Acro);
    method->setSpeed(10);
    method->setAltitude(2286);
}

void AcroForm::onApplyButtonClicked()
{
    double speed = ui->speedEdit->text().toDouble();
    double altitude = ui->altitudeEdit->text().toDouble() / METERS_TO_FEET;

    AcroScoring *method = (AcroScoring *) mMainWindow->scoringMethod(MainWindow::Acro);
    method->setSpeed(speed);
    method->setAltitude(altitude);

    mMainWindow->setFocus();
}

void AcroForm::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        // Reset window bounds
        updateView();

        // Release focus
        mMainWindow->setFocus();
    }

    QWidget::keyPressEvent(event);
}
