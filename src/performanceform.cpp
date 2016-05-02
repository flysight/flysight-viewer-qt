#include "performanceform.h"
#include "ui_performanceform.h"

#include <QPushButton>

#include "common.h"
#include "dataplot.h"
#include "datapoint.h"
#include "mainwindow.h"
#include "performancescoring.h"
#include "plotvalue.h"

PerformanceForm::PerformanceForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PerformanceForm),
    mMainWindow(0)
{
    ui->setupUi(this);

    connect(ui->startEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
    connect(ui->endEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
}

PerformanceForm::~PerformanceForm()
{
    delete ui;
}

QSize PerformanceForm::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void PerformanceForm::setMainWindow(
        MainWindow *mainWindow)
{
    mMainWindow = mainWindow;
}

void PerformanceForm::updateView()
{
    PerformanceScoring *method = (PerformanceScoring *) mMainWindow->scoringMethod(MainWindow::Performance);

    const double start = method->startTime();
    const double end = method->endTime();

    // Update window bounds
    ui->startEdit->setText(QString("%1").arg(start));
    ui->endEdit->setText(QString("%1").arg(end));

    DataPoint dpStart = mMainWindow->interpolateDataT(start);
    DataPoint dpEnd = mMainWindow->interpolateDataT(end);

    // Calculate results
    const double time = dpEnd.t - dpStart.t;
    const double vDistance = dpEnd.z - dpStart.z;
    const double hDistance = MainWindow::getDistance(dpStart, dpEnd);

    // Update display
    ui->timeEdit->setText(QString("%1").arg(time));
    ui->vDistanceEdit->setText(QString("%1").arg(vDistance));
    ui->hDistanceEdit->setText(QString("%1").arg(hDistance));
}

void PerformanceForm::onApplyButtonClicked()
{
    double start = ui->startEdit->text().toDouble();
    double end = ui->endEdit->text().toDouble();

    PerformanceScoring *method = (PerformanceScoring *) mMainWindow->scoringMethod(MainWindow::Performance);
    method->setRange(start, end);

    mMainWindow->setFocus();
}

void PerformanceForm::keyPressEvent(QKeyEvent *event)
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
