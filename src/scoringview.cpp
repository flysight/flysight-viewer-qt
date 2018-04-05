/***************************************************************************
**                                                                        **
**  FlySight Viewer                                                       **
**  Copyright 2018 Michael Cooper                                         **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>. **
**                                                                        **
****************************************************************************
**  Contact: Michael Cooper                                               **
**  Website: http://flysight.ca/                                          **
****************************************************************************/

#include "scoringview.h"
#include "ui_scoringview.h"

#include "mainwindow.h"
#include "performanceform.h"
#include "ppcform.h"
#include "scoringmethod.h"
#include "speedform.h"
#include "wideopendistanceform.h"
#include "wideopenspeedform.h"
#include "flareform.h"

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
    mFlareForm = new FlareForm(this);

    // Add options to combo box
    ui->modeComboBox->addItem("PPC");
    ui->modeComboBox->addItem("Speed Skydiving");
    ui->modeComboBox->addItem("Performance Records");
    ui->modeComboBox->addItem("WOWS Speed");
    ui->modeComboBox->addItem("WOWS Distance");
    ui->modeComboBox->addItem("Maximum Flare");

    // Add forms to stacked view
    ui->stackedWidget->addWidget(mPPCForm);
    ui->stackedWidget->addWidget(mSpeedForm);
    ui->stackedWidget->addWidget(mPerformanceForm);
    ui->stackedWidget->addWidget(mWideOpenSpeedForm);
    ui->stackedWidget->addWidget(mWideOpenDistanceForm);
    ui->stackedWidget->addWidget(mFlareForm);

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
    mFlareForm->setMainWindow(mainWindow);
}

void ScoringView::updateView()
{
    // Update forms
    switch (mMainWindow->scoringMode())
    {
    case MainWindow::PPC:
        mPPCForm->updateView();
        break;
    case MainWindow::Speed:
        mSpeedForm->updateView();
        break;
    case MainWindow::Performance:
        mPerformanceForm->updateView();
        break;
    case MainWindow::WideOpenSpeed:
        mWideOpenSpeedForm->updateView();
        break;
    case MainWindow::WideOpenDistance:
        mWideOpenDistanceForm->updateView();
        break;
    case MainWindow::Flare:
        mFlareForm->updateView();
        break;
    }
}

void ScoringView::changePage(
        int page)
{
    mMainWindow->setScoringMode((MainWindow::ScoringMode) page);
    ui->stackedWidget->setCurrentIndex(page);
}
