#include "scoringview.h"
#include "ui_scoringview.h"

#include "mainwindow.h"
#include "performanceform.h"
#include "ppcform.h"
#include "scoringmethod.h"
#include "speedform.h"
#include "wideopenform.h"

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
    mWideOpenForm = new WideOpenForm(this);

    // Add options to combo box
    ui->modeComboBox->addItem("PPC");
    ui->modeComboBox->addItem("Speed");
    ui->modeComboBox->addItem("Records");
    ui->modeComboBox->addItem("Wide Open");

    // Add forms to stacked view
    ui->stackedWidget->addWidget(mPPCForm);
    ui->stackedWidget->addWidget(mSpeedForm);
    ui->stackedWidget->addWidget(mPerformanceForm);
    ui->stackedWidget->addWidget(mWideOpenForm);

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
    mWideOpenForm->setMainWindow(mainWindow);
}

void ScoringView::updateView()
{
    // Update forms
    mPPCForm->updateView();
    mSpeedForm->updateView();
    mPerformanceForm->updateView();
    mWideOpenForm->updateView();
}

void ScoringView::changePage(
        int page)
{
    mMainWindow->setScoringMode((MainWindow::ScoringMode) page);
    ui->stackedWidget->setCurrentIndex(page);
}
