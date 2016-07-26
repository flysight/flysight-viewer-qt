#include "scoringview.h"
#include "ui_scoringview.h"

#include "mainwindow.h"
#include "performanceform.h"
#include "ppcform.h"
#include "scoringmethod.h"
#include "speedform.h"
#include "wideopendistanceform.h"
#include "wideopenspeedform.h"

ScoringView::ScoringView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScoringView),
    mMainWindow(0)
{
    ui->setupUi(this);

    // Create child forms
    mPPCForm = new PPCForm(this);
    mSpeedForm = new SpeedForm(this);
    mPerformanceForm = new PerformanceForm(this);
    mWideOpenSpeedForm = new WideOpenSpeedForm(this);
    mWideOpenDistanceForm = new WideOpenDistanceForm(this);

    // Add options to combo box
    ui->modeComboBox->addItem("PPC");
    ui->modeComboBox->addItem("Speed");
    ui->modeComboBox->addItem("Records");
    ui->modeComboBox->addItem("WOWS Speed");
    ui->modeComboBox->addItem("WOWS Distance");

    // Add forms to stacked view
    ui->stackedWidget->addWidget(mPPCForm);
    ui->stackedWidget->addWidget(mSpeedForm);
    ui->stackedWidget->addWidget(mPerformanceForm);
    ui->stackedWidget->addWidget(mWideOpenSpeedForm);
    ui->stackedWidget->addWidget(mWideOpenDistanceForm);

    // Connect mode combo to stacked widget
    connect(ui->modeComboBox, SIGNAL(activated(int)),
            this, SLOT(changePage(int)));
}

ScoringView::~ScoringView()
{
    delete ui;
}

void ScoringView::setMainWindow(
        MainWindow *mainWindow)
{
    mMainWindow = mainWindow;

    // Initialize mode selection
    ui->modeComboBox->setCurrentIndex(mMainWindow->scoringMode());
    ui->stackedWidget->setCurrentIndex(mMainWindow->scoringMode());

    // Update forms
    mPPCForm->setMainWindow(mainWindow);
    mSpeedForm->setMainWindow(mainWindow);
    mPerformanceForm->setMainWindow(mainWindow);
    mWideOpenSpeedForm->setMainWindow(mainWindow);
    mWideOpenDistanceForm->setMainWindow(mainWindow);
}

void ScoringView::updateView()
{
    // Update forms
    mPPCForm->updateView();
    mSpeedForm->updateView();
    mPerformanceForm->updateView();
    mWideOpenSpeedForm->updateView();
    mWideOpenDistanceForm->updateView();
}

void ScoringView::changePage(
        int page)
{
    mMainWindow->setScoringMode((MainWindow::ScoringMode) page);
    ui->stackedWidget->setCurrentIndex(page);
}
