#include "speedform.h"
#include "ui_speedform.h"

#include <QDebug>

#include "common.h"
#include "datapoint.h"
#include "mainwindow.h"
#include "plotvalue.h"
#include "speedscoring.h"
#include "ppcupload.h"

SpeedForm::SpeedForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SpeedForm),
    mMainWindow(0)
{
    ui->setupUi(this);

    connect(ui->faiButton, SIGNAL(clicked()), this, SLOT(onFAIButtonClicked()));
    connect(ui->upButton, SIGNAL(clicked()), this, SLOT(onUpButtonClicked()));
    connect(ui->downButton, SIGNAL(clicked()), this, SLOT(onDownButtonClicked()));

    connect(ui->topEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
    connect(ui->bottomEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));

    // Connect optimization buttons
    connect(ui->actualButton, SIGNAL(clicked()), this, SLOT(onActualButtonClicked()));
    connect(ui->optimalButton, SIGNAL(clicked()), this, SLOT(onOptimalButtonClicked()));
    connect(ui->optimizeButton, SIGNAL(clicked()), this, SLOT(onOptimizeButtonClicked()));

    // Connect PPC button
    connect(ui->ppcButton, SIGNAL(clicked()), this, SLOT(onPpcButtonClicked()));
}

SpeedForm::~SpeedForm()
{
    delete ui;
}

QSize SpeedForm::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void SpeedForm::setMainWindow(
        MainWindow *mainWindow)
{
    mMainWindow = mainWindow;
}

void SpeedForm::updateView()
{
    // Update mode selection
    ui->actualButton->setChecked(mMainWindow->windowMode() == MainWindow::Actual);
    ui->optimalButton->setChecked(mMainWindow->windowMode() == MainWindow::Optimal);

    SpeedScoring *method = (SpeedScoring *) mMainWindow->scoringMethod(MainWindow::Speed);

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
        const double verticalSpeed = (top - bottom) / time;

        // Update display
        if (mMainWindow->units() == PlotValue::Metric)
        {
            ui->verticalSpeedEdit->setText(QString("%1").arg(verticalSpeed * MPS_TO_KMH));
            ui->verticalSpeedUnits->setText(tr("km/h"));
        }
        else
        {
            ui->verticalSpeedEdit->setText(QString("%1").arg(verticalSpeed * MPS_TO_MPH));
            ui->verticalSpeedUnits->setText(tr("mph"));
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
            ui->verticalSpeedUnits->setText(tr("km/h"));
        }
        else
        {
            ui->verticalSpeedUnits->setText(tr("mph"));
        }

        ui->verticalSpeedEdit->setText(tr("n/a"));

        ui->actualButton->setEnabled(false);
        ui->optimalButton->setEnabled(false);
        ui->optimizeButton->setEnabled(false);
        ui->ppcButton->setEnabled(false);
    }
}

void SpeedForm::onFAIButtonClicked()
{
    SpeedScoring *method = (SpeedScoring *) mMainWindow->scoringMethod(MainWindow::Speed);
    method->setWindow(1700, 2700);
}

void SpeedForm::onApplyButtonClicked()
{
    double bottom = ui->bottomEdit->text().toDouble();
    double top = ui->topEdit->text().toDouble();

    SpeedScoring *method = (SpeedScoring *) mMainWindow->scoringMethod(MainWindow::Speed);
    method->setWindow(bottom, top);

    mMainWindow->setFocus();
}

void SpeedForm::onUpButtonClicked()
{
    SpeedScoring *method = (SpeedScoring *) mMainWindow->scoringMethod(MainWindow::Speed);
    method->setWindow(method->windowBottom() + 10, method->windowTop() + 10);
}

void SpeedForm::onDownButtonClicked()
{
    SpeedScoring *method = (SpeedScoring *) mMainWindow->scoringMethod(MainWindow::Speed);
    method->setWindow(method->windowBottom() - 10, method->windowTop() - 10);
}

void SpeedForm::keyPressEvent(QKeyEvent *event)
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

void SpeedForm::onActualButtonClicked()
{
    mMainWindow->setWindowMode(MainWindow::Actual);
}

void SpeedForm::onOptimalButtonClicked()
{
    mMainWindow->setWindowMode(MainWindow::Optimal);
}

void SpeedForm::onOptimizeButtonClicked()
{
    SpeedScoring *method = (SpeedScoring *) mMainWindow->scoringMethod(MainWindow::Speed);

    // Perform optimization
    method->optimize();

    ui->optimalButton->setEnabled(true);

    // Switch to optimal view
    mMainWindow->setWindowMode(MainWindow::Optimal);
}

void SpeedForm::onPpcButtonClicked() {

    // Return if plot empty
    if (mMainWindow->dataSize() == 0) return;

    PPCUpload *uploader = new PPCUpload(mMainWindow);
    SpeedScoring *method = (SpeedScoring *) mMainWindow->scoringMethod(MainWindow::Speed);
    DataPoint dpBottom, dpTop;

    ui->faiButton->click();
    ui->actualButton->click();

    if (method->getWindowBounds(mMainWindow->data(), dpBottom, dpTop)) {

        const double time = dpBottom.t - dpTop.t;
        const double distance = MainWindow::getDistance(dpTop, dpBottom);
        const double windowTop = dpTop.z;
        const double windowBottom = dpBottom.z;

        uploader->upload("SS", windowTop, windowBottom, time, distance);
    }
}
