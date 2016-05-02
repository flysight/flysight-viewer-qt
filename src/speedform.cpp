#include "speedform.h"
#include "ui_speedform.h"

#include <QPushButton>

#include "common.h"
#include "datapoint.h"
#include "mainwindow.h"
#include "plotvalue.h"
#include "speedscoring.h"

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
