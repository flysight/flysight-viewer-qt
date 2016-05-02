#include "scoringview.h"
#include "ui_scoringview.h"

#include "mainwindow.h"
#include "ppcform.h"
#include "speedform.h"

ScoringView::ScoringView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScoringView),
    mMainWindow(0)
{
    ui->setupUi(this);

    // Create child forms
    mPPCForm = new PPCForm(this);
    mSpeedForm = new SpeedForm(this);

    // Add options to combo box
    ui->modeComboBox->addItem("PPC");
    ui->modeComboBox->addItem("Speed");

    // Add forms to stacked view
    ui->stackedWidget->addWidget(mPPCForm);
    ui->stackedWidget->addWidget(mSpeedForm);

    // Connect mode combo to stacked widget
    connect(ui->modeComboBox, SIGNAL(activated(int)),
            this, SLOT(changePage(int)));

    // Connect optimization buttons
    connect(ui->actualButton, SIGNAL(clicked()), this, SLOT(onActualButtonClicked()));
    connect(ui->optimalButton, SIGNAL(clicked()), this, SLOT(onOptimalButtonClicked()));
    connect(ui->optimizeButton, SIGNAL(clicked()), this, SLOT(onOptimizeButtonClicked()));
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
}

void ScoringView::updateView()
{
    // Update mode selection
    ui->actualButton->setChecked(mMainWindow->windowMode() == MainWindow::Actual);
    ui->optimalButton->setChecked(mMainWindow->windowMode() == MainWindow::Optimal);

    // Update forms
    mPPCForm->updateView();
    mSpeedForm->updateView();
}

void ScoringView::changePage(
        int page)
{
    mMainWindow->setScoringMode((MainWindow::ScoringMode) page);
    ui->stackedWidget->setCurrentIndex(page);
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
    // Perform optimization
    mMainWindow->optimize();

    // Switch to optimal view
    mMainWindow->setWindowMode(MainWindow::Optimal);
}
