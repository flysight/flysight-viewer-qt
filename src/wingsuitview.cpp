#include "wingsuitview.h"
#include "ui_wingsuitview.h"

#include <QPushButton>

#include "common.h"
#include "mainwindow.h"

WingsuitView::WingsuitView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WingsuitView),
    mMainWindow(0)
{
    ui->setupUi(this);

    connect(ui->faiButton, SIGNAL(clicked()), this, SLOT(onFAIButtonClicked()));
    connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(onApplyButtonClicked()));
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
    // Update window bounds
    ui->bottomEdit->setText(QString("%1").arg(mMainWindow->windowBottom()));
    ui->topEdit->setText(QString("%1").arg(mMainWindow->windowTop()));
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
