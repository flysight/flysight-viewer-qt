#include "wideopenform.h"
#include "ui_wideopenform.h"

#include "mainwindow.h"
#include "wideopenscoring.h"

WideOpenForm::WideOpenForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WideOpenForm),
    mMainWindow(0)
{
    ui->setupUi(this);

    connect(ui->endLatitudeEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
    connect(ui->endLongitudeEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
    connect(ui->bearingEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
    connect(ui->bottomEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
    connect(ui->laneWidthEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
    connect(ui->laneLengthEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));

    // Connect start/end buttons
    connect(ui->startGlobeButton, SIGNAL(clicked()), this, SLOT(onStartButtonClicked()));
    connect(ui->endGlobeButton, SIGNAL(clicked()), this, SLOT(onEndButtonClicked()));
}

WideOpenForm::~WideOpenForm()
{
    delete ui;
}

QSize WideOpenForm::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void WideOpenForm::setMainWindow(
        MainWindow *mainWindow)
{
    mMainWindow = mainWindow;
}

void WideOpenForm::updateView()
{
    WideOpenScoring *method = (WideOpenScoring *) mMainWindow->scoringMethod(MainWindow::WideOpen);

    const double endLatitude = method->endLatitude();
    const double endLongitude = method->endLongitude();

    const double bearing = method->bearing();

    const double bottom = method->bottom();
    const double laneWidth = method->laneWidth();
    const double laneLength = method->laneLength();

    // Update display
    ui->endLatitudeEdit->setText(QString("%1").arg(endLatitude, 0, 'f', 7));
    ui->endLongitudeEdit->setText(QString("%1").arg(endLongitude, 0, 'f', 7));

    ui->bearingEdit->setText(QString("%1").arg(bearing, 0, 'f', 5));

    ui->bottomEdit->setText(QString("%1").arg(bottom));
    ui->laneWidthEdit->setText(QString("%1").arg(laneWidth));
    ui->laneLengthEdit->setText(QString("%1").arg(laneLength));

    DataPoint dpBottom, dpTop;
    bool success;

    success = method->getWindowBounds(mMainWindow->data(), dpBottom, dpTop);

    if (success)
    {
        // Calculate results
        const double distance = MainWindow::getDistance(dpTop, dpBottom);

        // Update display
        ui->distanceEdit->setText(QString("%1").arg(distance / 1000));
        ui->speedEdit->setText(QString("%1").arg(dpBottom.t));
    }
    else
    {
        // Update display
        ui->distanceEdit->setText(tr("n/a"));
        ui->speedEdit->setText(tr("n/a"));
    }
}

void WideOpenForm::onApplyButtonClicked()
{
    const double endLatitude = ui->endLatitudeEdit->text().toDouble();
    const double endLongitude = ui->endLongitudeEdit->text().toDouble();

    const double bearing = ui->bearingEdit->text().toDouble();

    const double bottom = ui->bottomEdit->text().toDouble();
    const double laneWidth = ui->laneWidthEdit->text().toDouble();
    const double laneLength = ui->laneLengthEdit->text().toDouble();

    WideOpenScoring *method = (WideOpenScoring *) mMainWindow->scoringMethod(MainWindow::WideOpen);
    method->setEnd(endLatitude, endLongitude);
    method->setBearing(bearing);
    method->setBottom(bottom);
    method->setLaneWidth(laneWidth);
    method->setLaneLength(laneLength);

    mMainWindow->setFocus();
}

void WideOpenForm::onStartButtonClicked()
{
    WideOpenScoring *method = (WideOpenScoring *) mMainWindow->scoringMethod(MainWindow::WideOpen);
    method->setMapMode(WideOpenScoring::Start);
    setFocus();
}

void WideOpenForm::onEndButtonClicked()
{
    WideOpenScoring *method = (WideOpenScoring *) mMainWindow->scoringMethod(MainWindow::WideOpen);
    method->setMapMode(WideOpenScoring::End);
    setFocus();
}

void WideOpenForm::keyPressEvent(
        QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        // Cancel map selection
        WideOpenScoring *method = (WideOpenScoring *) mMainWindow->scoringMethod(MainWindow::WideOpen);
        method->setMapMode(WideOpenScoring::None);

        // Reset display
        updateView();

        // Release focus
        mMainWindow->setFocus();
    }

    QWidget::keyPressEvent(event);
}
