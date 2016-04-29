#include "ppcform.h"
#include "ui_ppcform.h"

#include <QPushButton>

#include "common.h"
#include "datapoint.h"
#include "mainwindow.h"
#include "plotvalue.h"

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

    connect(ui->actualButton, SIGNAL(clicked()), this, SLOT(onActualButtonClicked()));
    connect(ui->optimalButton, SIGNAL(clicked()), this, SLOT(onOptimalButtonClicked()));
    connect(ui->optimizeButton, SIGNAL(clicked()), this, SLOT(onOptimizeButtonClicked()));
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
    const double bottom = mMainWindow->windowBottom();
    const double top = mMainWindow->windowTop();

    // Update window bounds
    ui->bottomEdit->setText(QString("%1").arg(bottom));
    ui->topEdit->setText(QString("%1").arg(top));

    ui->actualButton->setChecked(mMainWindow->windowMode() == MainWindow::Actual);
    ui->optimalButton->setChecked(mMainWindow->windowMode() == MainWindow::Optimal);

    if (mMainWindow->isWindowValid())
    {
        // Get window bounds
        const DataPoint &dpBottom = mMainWindow->windowBottomDP();
        const DataPoint &dpTop = mMainWindow->windowTopDP();

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
    }
}

void PPCForm::onFAIButtonClicked()
{
    mMainWindow->setWindow(2000, 3000);
}

void PPCForm::onApplyButtonClicked()
{
    double bottom = ui->bottomEdit->text().toDouble();
    double top = ui->topEdit->text().toDouble();

    mMainWindow->setWindow(bottom, top);
    mMainWindow->setFocus();
}

void PPCForm::onUpButtonClicked()
{
    mMainWindow->setWindow(
                mMainWindow->windowBottom() + 10,
                mMainWindow->windowTop() + 10);
}

void PPCForm::onDownButtonClicked()
{
    mMainWindow->setWindow(
                mMainWindow->windowBottom() - 10,
                mMainWindow->windowTop() - 10);
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
    if (ui->timeButton->isChecked())
    {
        mMainWindow->optimize(MainWindow::Time);
    }
    else if (ui->distanceButton->isChecked())
    {
        mMainWindow->optimize(MainWindow::Distance);
    }
    else if (ui->hSpeedButton->isChecked())
    {
        mMainWindow->optimize(MainWindow::HorizontalSpeed);
    }

    // Switch to optimal view
    mMainWindow->setWindowMode(MainWindow::Optimal);
}

double PPCForm::score(
        const QVector< DataPoint > &result)
{
    DataPoint dpBottom, dpTop;
    if (mMainWindow->getWindowBounds(result, dpBottom, dpTop))
    {
        if (ui->timeButton->isChecked())
        {
            return dpBottom.t - dpTop.t;
        }
        else if (ui->distanceButton->isChecked())
        {
            return dpBottom.x - dpTop.x;
        }
        else if (ui->hSpeedButton->isChecked())
        {
            return (dpBottom.x - dpTop.x) / (dpBottom.t - dpTop.t);
        }
    }
    else
    {
        return 0;
    }
}
