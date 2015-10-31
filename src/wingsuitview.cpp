#include "wingsuitview.h"
#include "ui_wingsuitview.h"

#include <QPushButton>

#include "common.h"
#include "datapoint.h"
#include "mainwindow.h"
#include "plotvalue.h"

WingsuitView::WingsuitView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WingsuitView),
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
}

WingsuitView::~WingsuitView()
{
    delete ui;
}

QSize WingsuitView::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void WingsuitView::updateView()
{
    ui->actualButton->setChecked(mMainWindow->windowMode() == MainWindow::Actual);
    ui->optimalButton->setChecked(mMainWindow->windowMode() == MainWindow::Optimal);

    const double bottom = mMainWindow->windowBottom();
    const double top = mMainWindow->windowTop();

    // Update window bounds
    ui->bottomEdit->setText(QString("%1").arg(bottom));
    ui->topEdit->setText(QString("%1").arg(top));

    if (mMainWindow->isWindowValid())
    {
        // Get window bounds
        const DataPoint &dpBottom = mMainWindow->windowBottomDP();
        const DataPoint &dpTop = mMainWindow->windowTopDP();

        // Calculate results
        const double time = dpBottom.t - dpTop.t;
        const double distance = mMainWindow->getDistance(dpBottom, dpTop) ;
        const double horizontalSpeed = distance / time;
        const double verticalSpeed = (top - bottom) / time;

        // Update display
        if (mMainWindow->units() == PlotValue::Metric)
        {
            ui->timeEdit->setText(QString("%1").arg(time));
            ui->distanceEdit->setText(QString("%1").arg(distance / 1000));
            ui->distanceUnits->setText(tr("km"));
            ui->horizontalSpeedEdit->setText(QString("%1").arg(horizontalSpeed * MPS_TO_KMH));
            ui->horizontalSpeedUnits->setText(tr("km/h"));
            ui->verticalSpeedEdit->setText(QString("%1").arg(verticalSpeed * MPS_TO_KMH));
            ui->verticalSpeedUnits->setText(tr("km/h"));
        }
        else
        {
            ui->timeEdit->setText(QString("%1").arg(time));
            ui->distanceEdit->setText(QString("%1").arg(distance * METERS_TO_FEET / 5280));
            ui->distanceUnits->setText(tr("mi"));
            ui->horizontalSpeedEdit->setText(QString("%1").arg(horizontalSpeed * MPS_TO_MPH));
            ui->horizontalSpeedUnits->setText(tr("mph"));
            ui->verticalSpeedEdit->setText(QString("%1").arg(verticalSpeed * MPS_TO_MPH));
            ui->verticalSpeedUnits->setText(tr("mph"));
        }
    }
    else
    {
        // Update display
        ui->timeEdit->setText(tr("n/a"));
        ui->distanceEdit->setText(tr("n/a"));
        ui->horizontalSpeedEdit->setText(tr("n/a"));
        ui->verticalSpeedEdit->setText(tr("n/a"));
    }
}

void WingsuitView::onFAIButtonClicked()
{
    mMainWindow->setWindow(2000, 3000);
}

void WingsuitView::onApplyButtonClicked()
{
    double bottom = ui->bottomEdit->text().toDouble();
    double top = ui->topEdit->text().toDouble();

    mMainWindow->setWindow(bottom, top);
    mMainWindow->setFocus();
}

void WingsuitView::onUpButtonClicked()
{
    mMainWindow->setWindow(
                mMainWindow->windowBottom() + 10,
                mMainWindow->windowTop() + 10);
}

void WingsuitView::onDownButtonClicked()
{
    mMainWindow->setWindow(
                mMainWindow->windowBottom() - 10,
                mMainWindow->windowTop() - 10);
}

void WingsuitView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        const double bottom = mMainWindow->windowBottom();
        const double top = mMainWindow->windowTop();

        // Update window bounds
        ui->bottomEdit->setText(QString("%1").arg(bottom));
        ui->topEdit->setText(QString("%1").arg(top));

        // Release focus
        mMainWindow->setFocus();
    }

    QWidget::keyPressEvent(event);
}

void WingsuitView::onActualButtonClicked()
{
    mMainWindow->setWindowMode(MainWindow::Actual);
}

void WingsuitView::onOptimalButtonClicked()
{
    mMainWindow->setWindowMode(MainWindow::Optimal);
}
