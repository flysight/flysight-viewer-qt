#include "wideopenform.h"
#include "ui_wideopenform.h"

WideOpenForm::WideOpenForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WideOpenForm)
{
    ui->setupUi(this);
}

WideOpenForm::~WideOpenForm()
{
    delete ui;
}
