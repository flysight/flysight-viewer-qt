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

#include "configdialog.h"
#include "ui_configdialog.h"

#include <QComboBox>
#include <QSettings>

#include "colorcombobox.h"
#include "dataplot.h"
#include "mainwindow.h"

#define PLOT_COLUMN_COLOUR  0
#define PLOT_COLUMN_MIN     1
#define PLOT_COLUMN_MAX     2
#define PLOT_NUM_COLUMNS    3

ConfigDialog::ConfigDialog(MainWindow *mainWindow) :
    QDialog(mainWindow),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);

    // Restore window geometry
    QSettings settings("FlySight", "Viewer");
    settings.beginGroup("configDialog");
    restoreGeometry(settings.value("geometry").toByteArray());
    settings.endGroup();

    // Add page
    ui->contentsWidget->addItems(
                QStringList() << tr("General"));
    ui->contentsWidget->addItems(
                QStringList() << tr("Import"));
    ui->contentsWidget->addItems(
                QStringList() << tr("Aerodynamics"));
    ui->contentsWidget->addItems(
                QStringList() << tr("Plots"));

    // Add units
    ui->unitsCombo->addItems(
                QStringList() << tr("Metric") << tr("Imperial"));

    // Update plot widget
    updatePlots();

    // Update plots when units are changed
    connect(ui->unitsCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updatePlots()));

    // Connect contents panel to stacked widget
    connect(ui->contentsWidget,
            SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));

    // Update reference when selection is changed
    connect(ui->autoReferenceButton, SIGNAL(clicked(bool)),
            this, SLOT(updateReference()));
    connect(ui->fixedReferenceButton, SIGNAL(clicked(bool)),
            this, SLOT(updateReference()));

    // Go to first page
    ui->contentsWidget->setCurrentRow(0);
}

ConfigDialog::~ConfigDialog()
{
    // Save window geometry
    QSettings settings("FlySight", "Viewer");
    settings.beginGroup("configDialog");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();

    delete ui;
}

void ConfigDialog::on_browseButton_clicked()
{
    // Get new settings file
    QString rootFolder = QFileDialog::getExistingDirectory(
                this,
                "",
                ui->logbookFolderEdit->text());

    if (!rootFolder.isEmpty())
    {
        // Update database path
        ui->logbookFolderEdit->setText(rootFolder);
    }
}

void ConfigDialog::on_defaultsButton_clicked()
{
    MainWindow *mainWindow = (MainWindow *) parent();

    // Color list
    QStringList colorNames = QColor::colorNames();

    for (int i = 0; i < DataPlot::yaLast; ++i)
    {
        if (QComboBox *combo = (QComboBox *) ui->plotTable->cellWidget(i, PLOT_COLUMN_COLOUR))
        {
            PlotValue *yValue = mainWindow->plotArea()->yValue(i);
            foreach (const QString &colorName, colorNames)
            {
                const QColor &color(colorName);
                if (color == yValue->defaultColor())
                {
                    combo->setCurrentText(colorName);
                }
            }
        }
    }
}

void ConfigDialog::changePage(
        QListWidgetItem *current,
        QListWidgetItem *previous)
{
    if (!current) current = previous;
    ui->pagesWidget->setCurrentIndex(
                ui->contentsWidget->row(current));
}

void ConfigDialog::updatePlots()
{
    MainWindow *mainWindow = (MainWindow *) parent();

    // Color list
    QStringList colorNames = QColor::colorNames();

    // Set up plots widget
    ui->plotTable->setColumnCount(PLOT_NUM_COLUMNS);
    ui->plotTable->setRowCount(DataPlot::yaLast);

    ui->plotTable->setHorizontalHeaderLabels(
                QStringList() << tr("Colour") << tr("Minimum") << tr("Maximum"));

    QStringList verticalHeaderLabels;

    for (int i = 0; i < DataPlot::yaLast; ++i)
    {
        PlotValue *yValue = mainWindow->plotArea()->yValue(i);

        verticalHeaderLabels << yValue->title(units());

        if (!ui->plotTable->cellWidget(i, PLOT_COLUMN_COLOUR))
        {
            ColorComboBox *combo = new ColorComboBox;
            combo->setColor(yValue->color());
            ui->plotTable->setCellWidget(i, PLOT_COLUMN_COLOUR, combo);
        }

        // Conversion factor from internal units to display units
        const double factor = yValue->factor(units());

        QTableWidgetItem *item;
        if (!(item = ui->plotTable->item(i, PLOT_COLUMN_MIN)))
        {
            item = new QTableWidgetItem;
            ui->plotTable->setItem(i, PLOT_COLUMN_MIN, item);
        }
        item->setText(QString::number(yValue->minimum() * factor));
        item->setCheckState(yValue->useMinimum() ? Qt::Checked : Qt::Unchecked);

        if (!(item = ui->plotTable->item(i, PLOT_COLUMN_MAX)))
        {
            item = new QTableWidgetItem;
            ui->plotTable->setItem(i, PLOT_COLUMN_MAX, item);
        }
        item->setText(QString::number(yValue->maximum() * factor));
        item->setCheckState(yValue->useMaximum() ? Qt::Checked : Qt::Unchecked);
    }

    ui->plotTable->setVerticalHeaderLabels(verticalHeaderLabels);
}

void ConfigDialog::updateReference()
{
        ui->fixedReferenceEdit->setEnabled(ui->fixedReferenceButton->isChecked());
    ui->fixedReferenceUnits->setEnabled(ui->fixedReferenceButton->isChecked());
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

void ConfigDialog::setGroundReference(
        MainWindow::GroundReference ref)
{
    ui->autoReferenceButton->setChecked(ref == MainWindow::Automatic);
    ui->fixedReferenceButton->setChecked(ref == MainWindow::Fixed);
    updateReference();
}

double ConfigDialog::fixedReference() const
{
    return ui->fixedReferenceEdit->text().toDouble();
}

void ConfigDialog::setFixedReference(
        double elevation)
{
    ui->fixedReferenceEdit->setText(QString("%1").arg(elevation));
}

MainWindow::GroundReference ConfigDialog::groundReference() const
{
    if (ui->autoReferenceButton->isChecked()) return MainWindow::Automatic;
    else                                      return MainWindow::Fixed;
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

void ConfigDialog::setMinDrag(
        double minDrag)
{
    ui->minDragEdit->setText(QString("%1").arg(minDrag));
}

double ConfigDialog::minDrag() const
{
    return ui->minDragEdit->text().toDouble();
}

void ConfigDialog::setMinLift(
        double minLift)
{
    ui->minLiftEdit->setText(QString("%1").arg(minLift));
}

double ConfigDialog::minLift() const
{
    return ui->minLiftEdit->text().toDouble();
}

void ConfigDialog::setMaxLift(
        double maxLift)
{
    ui->maxLiftEdit->setText(QString("%1").arg(maxLift));
}

double ConfigDialog::maxLift() const
{
    return ui->maxLiftEdit->text().toDouble();
}

void ConfigDialog::setMaxLD(
        double maxLD)
{
    ui->maxLDEdit->setText(QString("%1").arg(maxLD));
}

double ConfigDialog::maxLD() const
{
    return ui->maxLDEdit->text().toDouble();
}

void ConfigDialog::setSimulationTime(
        int simulationTime)
{
    ui->simTimeSpinBox->setValue(simulationTime);
}

int ConfigDialog::simulationTime() const
{
    return ui->simTimeSpinBox->value();
}

QColor ConfigDialog::plotColor(
        int i) const
{
    ColorComboBox *combo;
    combo = static_cast< ColorComboBox* >(ui->plotTable->cellWidget(i, PLOT_COLUMN_COLOUR));
    return combo->color();
}

double ConfigDialog::plotMinimum(
        int i) const
{
    return ui->plotTable->item(i, PLOT_COLUMN_MIN)->text().toDouble();
}

double ConfigDialog::plotMaximum(
        int i) const
{
    return ui->plotTable->item(i, PLOT_COLUMN_MAX)->text().toDouble();
}

bool ConfigDialog::plotUseMinimum(
        int i) const
{
    return ui->plotTable->item(i, PLOT_COLUMN_MIN)->checkState() == Qt::Checked;
}

bool ConfigDialog::plotUseMaximum(
        int i) const
{
    return ui->plotTable->item(i, PLOT_COLUMN_MAX)->checkState() == Qt::Checked;
}

void ConfigDialog::setLineThickness(
        double width)
{
    ui->lineThicknessEdit->setText(QString::number(width));
}

double ConfigDialog::lineThickness() const
{
    return ui->lineThicknessEdit->text().toDouble();
}

void ConfigDialog::setWindSpeed(
        double speed)
{
    ui->windSpeedEdit->setText(QString::number(speed));
}

double ConfigDialog::windSpeed() const
{
    return ui->windSpeedEdit->text().toDouble();
}

void ConfigDialog::setWindUnits(
        QString units)
{
    ui->windUnitsLabel->setText(units);
}

void ConfigDialog::setWindDirection(
        double dir)
{
    ui->windDirectionEdit->setText(QString::number(dir));
}

double ConfigDialog::windDirection() const
{
    return ui->windDirectionEdit->text().toDouble();
}

void ConfigDialog::setUseDatabase(
        bool use)
{
    ui->useLogbookButton->setChecked(use);
    ui->dontUseLogbookButton->setChecked(!use);
}

bool ConfigDialog::useDatabase() const
{
    return (ui->useLogbookButton->isChecked());
}

void ConfigDialog::setDatabasePath(
        QString databasePath)
{
    // Update database path
    ui->logbookFolderEdit->setText(databasePath);
}

QString ConfigDialog::databasePath() const
{
    return ui->logbookFolderEdit->text();
}
