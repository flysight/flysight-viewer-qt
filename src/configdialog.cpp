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
    ui->contentsWidget->addItems(
                QStringList() << tr("Aerodynamics"));

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

void ConfigDialog::setTemperature(
        double temperature)
{
    ui->temperatureEdit->setText(QString("%1").arg(temperature - 273.15));
}

double ConfigDialog::temperature() const
{
    return ui->temperatureEdit->text().toDouble() + 273.15;
}

void ConfigDialog::setMass(
        double mass)
{
    ui->massEdit->setText(QString("%1").arg(mass));
}

double ConfigDialog::mass() const
{
    return ui->massEdit->text().toDouble();
}

void ConfigDialog::setPlanformArea(
        double area)
{
    ui->areaEdit->setText(QString("%1").arg(area));
}

double ConfigDialog::planformArea() const
{
    return ui->areaEdit->text().toDouble();
}

void ConfigDialog::setWingSpan(
        double span)
{
    ui->spanEdit->setText(QString("%1").arg(span));
}

double ConfigDialog::wingSpan() const
{
    return ui->spanEdit->text().toDouble();
}
