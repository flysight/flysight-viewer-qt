#include "scoringview.h"
#include "ui_scoringview.h"

#include "mainwindow.h"
#include "ppcform.h"

ScoringView::ScoringView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScoringView)
{
    ui->setupUi(this);

    // Create child forms
    mPPCForm = new PPCForm(this);

    // Add forms to stacked view
    ui->modeComboBox->addItem("PPC");
    ui->stackedWidget->addWidget(mPPCForm);

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
    // Initialize mode selection
    ui->modeComboBox->setCurrentIndex(0);
    ui->stackedWidget->setCurrentIndex(0);

    // Update forms
    mPPCForm->setMainWindow(mainWindow);
}

void ScoringView::updateView()
{
    mPPCForm->updateView();
}

void ScoringView::changePage(
        int page)
{
//    mMainWindow->setScoringMode((MainWindow::ScoringMode) page);
    ui->stackedWidget->setCurrentIndex(page);
}
