#include "ppcform.h"
#include "ui_ppcform.h"

#include <QDebug>

#include "common.h"
#include "dataplot.h"
#include "datapoint.h"
#include "mainwindow.h"
#include "plotvalue.h"
#include "ppcscoring.h"
#include "ppcupload.h"

PPCForm::PPCForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PPCForm),
    mMainWindow(0)
{
    ui->setupUi(this);

    connect(ui->faiButton, SIGNAL(clicked()), this, SLOT(onFAIButtonClicked()));
    connect(ui->upButton, SIGNAL(clicked()), this, SLOT(onUpButtonClicked()));
    connect(ui->downButton, SIGNAL(clicked()), this, SLOT(onDownButtonClicked()));

    connect(ui->topEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
    connect(ui->bottomEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));

    connect(ui->timeButton, SIGNAL(clicked()), this, SLOT(onModeChanged()));
    connect(ui->distanceButton, SIGNAL(clicked()), this, SLOT(onModeChanged()));
    connect(ui->hSpeedButton, SIGNAL(clicked()), this, SLOT(onModeChanged()));

    // Connect optimization buttons
    connect(ui->actualButton, SIGNAL(clicked()), this, SLOT(onActualButtonClicked()));
    connect(ui->optimalButton, SIGNAL(clicked()), this, SLOT(onOptimalButtonClicked()));
    connect(ui->optimizeButton, SIGNAL(clicked()), this, SLOT(onOptimizeButtonClicked()));

    // Connect PPC button
    connect(ui->ppcButton, SIGNAL(clicked()), this, SLOT(onPpcButtonClicked()));
}

PPCForm::~PPCForm()
{
    delete ui;
}

QSize PPCForm::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void PPCForm::setMainWindow(
        MainWindow *mainWindow)
{
    mMainWindow = mainWindow;
}

void PPCForm::updateView()
{
    // Update mode selection
    ui->actualButton->setChecked(mMainWindow->windowMode() == MainWindow::Actual);
    ui->optimalButton->setChecked(mMainWindow->windowMode() == MainWindow::Optimal);

    PPCScoring *method = (PPCScoring *) mMainWindow->scoringMethod(MainWindow::PPC);

    const double bottom = method->windowBottom();
    const double top = method->windowTop();

    // Update window bounds
    ui->bottomEdit->setText(QString("%1").arg(bottom));
    ui->topEdit->setText(QString("%1").arg(top));

    DataPoint dpBottom, dpTop;
    bool success;

    switch (mMainWindow->windowMode())
    {
    case MainWindow::Actual:
        success = method->getWindowBounds(mMainWindow->data(), dpBottom, dpTop);
        break;
    case MainWindow::Optimal:
        success = method->getWindowBounds(mMainWindow->optimal(), dpBottom, dpTop);
        break;
    }

    if (success)
    {
        // Calculate results
        const double time = dpBottom.t - dpTop.t;
        const double distance = MainWindow::getDistance(dpTop, dpBottom);
        const double horizontalSpeed = distance / time;

        // Update display
        if (mMainWindow->units() == PlotValue::Metric)
        {
            ui->timeEdit->setText(QString("%1").arg(time));
            ui->distanceEdit->setText(QString("%1").arg(distance / 1000));
            ui->distanceUnits->setText(tr("km"));
            ui->horizontalSpeedEdit->setText(QString("%1").arg(horizontalSpeed * MPS_TO_KMH));
            ui->horizontalSpeedUnits->setText(tr("km/h"));
        }
        else
        {
            ui->timeEdit->setText(QString("%1").arg(time));
            ui->distanceEdit->setText(QString("%1").arg(distance * METERS_TO_FEET / 5280));
            ui->distanceUnits->setText(tr("mi"));
            ui->horizontalSpeedEdit->setText(QString("%1").arg(horizontalSpeed * MPS_TO_MPH));
            ui->horizontalSpeedUnits->setText(tr("mph"));
        }
        ui->actualButton->setEnabled(true);
        ui->optimizeButton->setEnabled(true);
        ui->ppcButton->setEnabled(true);
    }
    else
    {
        // Update display
        if (mMainWindow->units() == PlotValue::Metric)
        {
            ui->distanceUnits->setText(tr("km"));
            ui->horizontalSpeedUnits->setText(tr("km/h"));
        }
        else
        {
            ui->distanceUnits->setText(tr("mi"));
            ui->horizontalSpeedUnits->setText(tr("mph"));
        }

        ui->timeEdit->setText(tr("n/a"));
        ui->distanceEdit->setText(tr("n/a"));
        ui->horizontalSpeedEdit->setText(tr("n/a"));

        ui->actualButton->setEnabled(false);
        ui->optimalButton->setEnabled(false);
        ui->optimizeButton->setEnabled(false);
        ui->ppcButton->setEnabled(false);
    }
}

void PPCForm::onFAIButtonClicked()
{
    PPCScoring *method = (PPCScoring *) mMainWindow->scoringMethod(MainWindow::PPC);
    method->setWindow(2000, 3000);
}

void PPCForm::onApplyButtonClicked()
{
    double bottom = ui->bottomEdit->text().toDouble();
    double top = ui->topEdit->text().toDouble();

    PPCScoring *method = (PPCScoring *) mMainWindow->scoringMethod(MainWindow::PPC);
    method->setWindow(bottom, top);

    mMainWindow->setFocus();
}

void PPCForm::onUpButtonClicked()
{
    PPCScoring *method = (PPCScoring *) mMainWindow->scoringMethod(MainWindow::PPC);
    method->setWindow(method->windowBottom() + 10, method->windowTop() + 10);
}

void PPCForm::onDownButtonClicked()
{
    PPCScoring *method = (PPCScoring *) mMainWindow->scoringMethod(MainWindow::PPC);
    method->setWindow(method->windowBottom() - 10, method->windowTop() - 10);
}

void PPCForm::keyPressEvent(QKeyEvent *event)
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

void PPCForm::onModeChanged()
{
    PPCScoring *method = (PPCScoring *) mMainWindow->scoringMethod(MainWindow::PPC);

    if (ui->timeButton->isChecked())          method->setMode(PPCScoring::Time);
    else if (ui->distanceButton->isChecked()) method->setMode(PPCScoring::Distance);
    else                                      method->setMode(PPCScoring::Speed);
}

void PPCForm::onActualButtonClicked()
{
    mMainWindow->setWindowMode(MainWindow::Actual);
}

void PPCForm::onOptimalButtonClicked()
{
    mMainWindow->setWindowMode(MainWindow::Optimal);
}

void PPCForm::onOptimizeButtonClicked()
{
    PPCScoring *method = (PPCScoring *) mMainWindow->scoringMethod(MainWindow::PPC);

    // Perform optimization
    method->optimize();

    ui->optimalButton->setEnabled(true);

    // Switch to optimal view
    mMainWindow->setWindowMode(MainWindow::Optimal);
}

void PPCForm::onPpcButtonClicked() {

    // Return if plot empty
    if (mMainWindow->dataSize() == 0) return;

    PPCUpload *uploader = new PPCUpload(mMainWindow);
    PPCScoring *method = (PPCScoring *) mMainWindow->scoringMethod(MainWindow::PPC);
    DataPoint dpBottom, dpTop;

    ui->faiButton->click();
    ui->actualButton->click();

    if (method->getWindowBounds(mMainWindow->data(), dpBottom, dpTop)) {
        const double time = dpBottom.t - dpTop.t;
        const double distance = MainWindow::getDistance(dpTop, dpBottom);
        const double windowTop = dpTop.z;
        const double windowBottom = dpBottom.z;

        uploader->upload("WS", windowTop, windowBottom, time, distance);
    }
}

