#include "flareform.h"
#include "ui_FlareForm.h"

#include <QPushButton>

#include "common.h"
#include "dataplot.h"
#include "datapoint.h"
#include "mainwindow.h"
#include "flarescoring.h"
#include "plotvalue.h"

FlareForm::FlareForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FlareForm),
    mMainWindow(0)
{
    ui->setupUi(this);

    connect(ui->startEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
    connect(ui->endEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
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
    FlareScoring *method = (FlareScoring *) mMainWindow->scoringMethod(MainWindow::Performance);

    const double start = method->startTime();
    const double end = method->endTime();

    // Update window bounds
    ui->startEdit->setText(QString("%1").arg(start));
    ui->endEdit->setText(QString("%1").arg(end));

    DataPoint dpStart = mMainWindow->interpolateDataT(start);
    DataPoint dpEnd = mMainWindow->interpolateDataT(end);

    // Calculate results
    const double time = dpEnd.t - dpStart.t;
    const double vDistance = dpStart.z - dpEnd.z;
    const double hDistance = MainWindow::getDistance(dpStart, dpEnd);

    // Update display
    ui->timeEdit->setText(QString("%1").arg(time, 0, 'f', 3));
    ui->vDistanceEdit->setText(QString("%1").arg(vDistance, 0, 'f', 3));
    ui->hDistanceEdit->setText(QString("%1").arg(hDistance, 0, 'f', 3));

    // Auxiliary values
    ui->startLatEdit->setText(QString("%1").arg(dpStart.lat, 0, 'f', 7));
    ui->startLonEdit->setText(QString("%1").arg(dpStart.lon, 0, 'f', 7));
    ui->startElevEdit->setText(QString("%1").arg(dpStart.hMSL, 0, 'f', 3));

    ui->endLatEdit->setText(QString("%1").arg(dpEnd.lat, 0, 'f', 7));
    ui->endLonEdit->setText(QString("%1").arg(dpEnd.lon, 0, 'f', 7));
    ui->endElevEdit->setText(QString("%1").arg(dpEnd.hMSL, 0, 'f', 3));
}

void FlareForm::onApplyButtonClicked()
{
    double start = ui->startEdit->text().toDouble();
    double end = ui->endEdit->text().toDouble();

    FlareScoring *method = (FlareScoring *) mMainWindow->scoringMethod(MainWindow::Performance);
    method->setRange(start, end);

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
