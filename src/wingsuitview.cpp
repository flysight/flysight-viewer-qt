#include "wingsuitview.h"
#include "ui_wingsuitview.h"

#include "common.h"
#include "mainwindow.h"

WingsuitView::WingsuitView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WingsuitView),
    mMainWindow(0)
{
    ui->setupUi(this);
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
