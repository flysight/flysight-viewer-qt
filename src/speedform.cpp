#include "speedform.h"
#include "ui_speedform.h"

#include <QPushButton>

#include "common.h"
#include "datapoint.h"
#include "mainwindow.h"
#include "plotvalue.h"

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
    mMainWindow->setWindow(1700, 2700);
}

void SpeedForm::onApplyButtonClicked()
{
    double bottom = ui->bottomEdit->text().toDouble();
    double top = ui->topEdit->text().toDouble();

    mMainWindow->setWindow(bottom, top);
    mMainWindow->setFocus();
}

void SpeedForm::onUpButtonClicked()
{
    mMainWindow->setWindow(
                mMainWindow->windowBottom() + 10,
                mMainWindow->windowTop() + 10);
}

void SpeedForm::onDownButtonClicked()
{
    mMainWindow->setWindow(
                mMainWindow->windowBottom() - 10,
                mMainWindow->windowTop() - 10);
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
