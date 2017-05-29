#include "flareform.h"
#include "ui_flareform.h"

#include <QPushButton>

#include "common.h"
#include "dataplot.h"
#include "datapoint.h"
#include "mainwindow.h"
#include "plotvalue.h"
#include "FlareScoring.h"

FlareForm::FlareForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FlareForm),
    mMainWindow(0)
{
    ui->setupUi(this);

    connect(ui->bottomEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));

    // Connect optimization buttons
    connect(ui->actualButton, SIGNAL(clicked()), this, SLOT(onActualButtonClicked()));
    connect(ui->optimalButton, SIGNAL(clicked()), this, SLOT(onOptimalButtonClicked()));
    connect(ui->optimizeButton, SIGNAL(clicked()), this, SLOT(onOptimizeButtonClicked()));
}

FlareForm::~FlareForm()
{
    delete ui;
}

QSize FlareForm::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void FlareForm::setMainWindow(
        MainWindow *mainWindow)
{
    mMainWindow = mainWindow;
}

void FlareForm::updateView()
{
    // Update mode selection
    ui->actualButton->setChecked(mMainWindow->windowMode() == MainWindow::Actual);
    ui->optimalButton->setChecked(mMainWindow->windowMode() == MainWindow::Optimal);

    FlareScoring *method = (FlareScoring *) mMainWindow->scoringMethod(MainWindow::Flare);

    const double bottom = method->windowBottom();

    // Update window bounds
    ui->bottomEdit->setText(QString("%1").arg(bottom));

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
        const double flare = dpTop.hMSL - dpBottom.hMSL;

        // Update display
        if (mMainWindow->units() == PlotValue::Metric)
        {
            ui->flareEdit->setText(QString("%1").arg(flare));
            ui->flareUnits->setText(tr("m"));
        }
        else
        {
            ui->flareEdit->setText(QString("%1").arg(flare * METERS_TO_FEET));
            ui->flareUnits->setText(tr("ft"));
        }
    }
    else
    {
        // Update display
        if (mMainWindow->units() == PlotValue::Metric)
        {
            ui->flareUnits->setText(tr("m"));
        }
        else
        {
            ui->flareUnits->setText(tr("ft"));
        }

        ui->flareEdit->setText(tr("n/a"));
    }
}

void FlareForm::onApplyButtonClicked()
{
    double bottom = ui->bottomEdit->text().toDouble();

    FlareScoring *method = (FlareScoring *) mMainWindow->scoringMethod(MainWindow::Flare);
    method->setWindowBottom(bottom);

    mMainWindow->setFocus();
}

void FlareForm::keyPressEvent(QKeyEvent *event)
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

void FlareForm::onActualButtonClicked()
{
    mMainWindow->setWindowMode(MainWindow::Actual);
}

void FlareForm::onOptimalButtonClicked()
{
    mMainWindow->setWindowMode(MainWindow::Optimal);
}

void FlareForm::onOptimizeButtonClicked()
{
    FlareScoring *method = (FlareScoring *) mMainWindow->scoringMethod(MainWindow::Flare);

    // Perform optimization
    method->optimize();

    // Switch to optimal view
    mMainWindow->setWindowMode(MainWindow::Optimal);
}
