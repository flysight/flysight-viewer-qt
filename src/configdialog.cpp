#include "configdialog.h"
#include "ui_configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);

    // Add page
    ui->contentsWidget->addItems(
                QStringList() << tr("General"));
    ui->contentsWidget->addItems(
                QStringList() << tr("Wind"));

    // Add units
    ui->unitsCombo->addItems(
                QStringList() << tr("Metric") << tr("Imperial"));

    // Connect contents panel to stacked widget
    connect(ui->contentsWidget,
            SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));

    // Go to first page
    ui->contentsWidget->setCurrentRow(0);
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::changePage(
        QListWidgetItem *current,
        QListWidgetItem *previous)
{
    if (!current) current = previous;
    ui->pagesWidget->setCurrentIndex(
                ui->contentsWidget->row(current));
}

void ConfigDialog::setUnits(
        PlotValue::Units units)
{
    ui->unitsCombo->setCurrentIndex(units);
}

PlotValue::Units ConfigDialog::units() const
{
    return (PlotValue::Units) ui->unitsCombo->currentIndex();
}

void ConfigDialog::setDtWind(
        double dtWind)
{
    ui->dtWindEdit->setText(QString("%1").arg(dtWind));
}

double ConfigDialog::dtWind() const
{
    return ui->dtWindEdit->text().toDouble();
}
