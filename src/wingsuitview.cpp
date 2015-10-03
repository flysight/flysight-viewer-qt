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

    connect(ui->topEdit, SIGNAL(returnPressed()), this, SLOT(onApplyButtonClicked()));
    connect(ui->bottomEdit, SIGNAL(returnPressed()), this, SLOT(onApplyButtonClicked()));
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
    const double bottom = mMainWindow->windowBottom();
    const double top = mMainWindow->windowTop();

    // Update window bounds
    ui->bottomEdit->setText(QString("%1").arg(bottom));
    ui->topEdit->setText(QString("%1").arg(top));

    int iBottom, iTop;
    bool armed = false;

    // Find end of window
    for (int i = mMainWindow->dataSize() - 1; i >= 0; --i)
    {
        const DataPoint &dp = mMainWindow->dataPoint(i);

        if (dp.alt < bottom)
        {
            iBottom = i;
        }

        if (dp.alt < top)
        {
            iTop = i;
        }
        else
        {
            armed = true;
        }

        if (armed && DataPoint::energyRate(dp) > 0) break;
    }

    if (armed)
    {
        // Calculate bottom of window
        const DataPoint &dp1 = mMainWindow->dataPoint(iBottom - 1);
        const DataPoint &dp2 = mMainWindow->dataPoint(iBottom);
        DataPoint dpBottom = DataPoint::interpolate(dp1, dp2, (bottom - dp1.alt) / (dp2.alt - dp1.alt));

        // Calculate top of window
        const DataPoint &dp3 = mMainWindow->dataPoint(iTop - 1);
        const DataPoint &dp4 = mMainWindow->dataPoint(iTop);
        DataPoint dpTop = DataPoint::interpolate(dp3, dp4, (top - dp3.alt) / (dp4.alt - dp3.alt));

        // Calculate results
        const double time = dpBottom.t - dpTop.t;
        const double distance = mMainWindow->getDistance(dpBottom, dpTop) ;
        const double speed = distance / time;

        // Update display
        if (mMainWindow->units() == PlotValue::Metric)
        {
            ui->timeEdit->setText(QString("%1").arg(time));
            ui->distanceEdit->setText(QString("%1").arg(distance / 1000));
            ui->distanceUnits->setText(tr("km"));
            ui->speedEdit->setText(QString("%1").arg(speed * MPS_TO_KMH));
            ui->speedUnits->setText(tr("km/h"));
        }
        else
        {
            ui->timeEdit->setText(QString("%1").arg(time));
            ui->distanceEdit->setText(QString("%1").arg(distance * METERS_TO_FEET / 5280));
            ui->distanceUnits->setText(tr("mi"));
            ui->speedEdit->setText(QString("%1").arg(speed * MPS_TO_MPH));
            ui->speedUnits->setText(tr("mph"));
        }
    }
    else
    {
        // Update display
        ui->timeEdit->setText(tr("n/a"));
        ui->distanceEdit->setText(tr("n/a"));
        ui->speedEdit->setText(tr("n/a"));
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
