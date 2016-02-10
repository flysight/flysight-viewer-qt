#include "scoringview.h"
#include "ui_scoringview.h"

#include <QPushButton>

#include "common.h"
#include "datapoint.h"
#include "mainwindow.h"
#include "plotvalue.h"

ScoringView::ScoringView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScoringView),
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

ScoringView::~ScoringView()
{
    delete ui;
}

QSize ScoringView::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void ScoringView::setMainWindow(
        MainWindow *mainWindow)
{
    mMainWindow = mainWindow;

    const double bottom = mMainWindow->windowBottom();
    const double top = mMainWindow->windowTop();

    // Update window bounds
    ui->bottomEdit->setText(QString("%1").arg(bottom));
    ui->topEdit->setText(QString("%1").arg(top));
}

void ScoringView::updateView()
{
    ui->actualButton->setChecked(mMainWindow->windowMode() == MainWindow::Actual);
    ui->optimalButton->setChecked(mMainWindow->windowMode() == MainWindow::Optimal);

    const double bottom = mMainWindow->windowBottom();
    const double top = mMainWindow->windowTop();

    if (mMainWindow->isWindowValid())
    {
        // Get window bounds
        const DataPoint &dpBottom = mMainWindow->windowBottomDP();
        const DataPoint &dpTop = mMainWindow->windowTopDP();

        // Calculate results
        const double time = dpBottom.t - dpTop.t;
        const double dx = dpBottom.x - dpTop.x;
        const double dy = dpBottom.y - dpTop.y;
        const double distance = sqrt(dx * dx + dy * dy) ;
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
        if (mMainWindow->units() == PlotValue::Metric)
        {
            ui->distanceUnits->setText(tr("km"));
            ui->horizontalSpeedUnits->setText(tr("km/h"));
            ui->verticalSpeedUnits->setText(tr("km/h"));
        }
        else
        {
            ui->distanceUnits->setText(tr("mi"));
            ui->horizontalSpeedUnits->setText(tr("mph"));
            ui->verticalSpeedUnits->setText(tr("mph"));
        }

        ui->timeEdit->setText(tr("n/a"));
        ui->distanceEdit->setText(tr("n/a"));
        ui->horizontalSpeedEdit->setText(tr("n/a"));
        ui->verticalSpeedEdit->setText(tr("n/a"));
    }
}

void ScoringView::onFAIButtonClicked()
{
    mMainWindow->setWindow(2000, 3000);

    const double bottom = mMainWindow->windowBottom();
    const double top = mMainWindow->windowTop();

    // Update window bounds
    ui->bottomEdit->setText(QString("%1").arg(bottom));
    ui->topEdit->setText(QString("%1").arg(top));
}

void ScoringView::onApplyButtonClicked()
{
    double bottom = ui->bottomEdit->text().toDouble();
    double top = ui->topEdit->text().toDouble();

    mMainWindow->setWindow(bottom, top);
    mMainWindow->setFocus();
}

void ScoringView::onUpButtonClicked()
{
    mMainWindow->setWindow(
                mMainWindow->windowBottom() + 10,
                mMainWindow->windowTop() + 10);

    const double bottom = mMainWindow->windowBottom();
    const double top = mMainWindow->windowTop();

    // Update window bounds
    ui->bottomEdit->setText(QString("%1").arg(bottom));
    ui->topEdit->setText(QString("%1").arg(top));
}

void ScoringView::onDownButtonClicked()
{
    mMainWindow->setWindow(
                mMainWindow->windowBottom() - 10,
                mMainWindow->windowTop() - 10);

    const double bottom = mMainWindow->windowBottom();
    const double top = mMainWindow->windowTop();

    // Update window bounds
    ui->bottomEdit->setText(QString("%1").arg(bottom));
    ui->topEdit->setText(QString("%1").arg(top));
}

void ScoringView::keyPressEvent(QKeyEvent *event)
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

void ScoringView::onActualButtonClicked()
{
    mMainWindow->setWindowMode(MainWindow::Actual);
}

void ScoringView::onOptimalButtonClicked()
{
    mMainWindow->setWindowMode(MainWindow::Optimal);
}

void ScoringView::onOptimizeButtonClicked()
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
    else if (ui->vSpeedButton->isChecked())
    {
        mMainWindow->optimize(MainWindow::VerticalSpeed);
    }

    // Switch to optimal view
    mMainWindow->setWindowMode(MainWindow::Optimal);
}
