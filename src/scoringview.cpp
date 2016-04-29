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
    mPPCForm->updateView();
    mSpeedForm->updateView();
}

void ScoringView::changePage(
        int page)
{
    mMainWindow->setScoringMode((MainWindow::ScoringMode) page);
    ui->stackedWidget->setCurrentIndex(page);
}

double ScoringView::score(
        const QVector< DataPoint > &result)
{
    switch (mMainWindow->scoringMode())
    {
    case MainWindow::PPC:
        return mPPCForm->score(result);
    case MainWindow::Speed:
        return mSpeedForm->score(result);
    default:
        Q_ASSERT(false);    // should never be called
        return 0;
    }
}
