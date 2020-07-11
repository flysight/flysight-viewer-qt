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

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QColor>
#include <QDialog>
#include <QListWidget>

#include "mainwindow.h"
#include "plotvalue.h"

namespace Ui {
class ConfigDialog;
}

class MainWindow;

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(MainWindow *mainWindow);
    ~ConfigDialog();

    void setUnits(PlotValue::Units units);
    PlotValue::Units units() const;

    void setGroundReference(MainWindow::GroundReference ref);
    MainWindow::GroundReference groundReference() const;

    double fixedReference() const;
    void setFixedReference(double elevation);

    void setMass(double mass);
    double mass() const;

    void setPlanformArea(double area);
    double planformArea() const;

    void setMinDrag(double minDrag);
    double minDrag() const;

    void setMinLift(double minLift);
    double minLift() const;

    void setMaxLift(double maxLift);
    double maxLift() const;

    void setMaxLD(double maxLD);
    double maxLD() const;

    void setSimulationTime(int simulationTime);
    int simulationTime() const;

    QColor plotColor(int i) const;

    double plotMinimum(int i) const;
    double plotMaximum(int i) const;

    bool plotUseMinimum(int i) const;
    bool plotUseMaximum(int i) const;

    void setLineThickness(double with);
    double lineThickness() const;

    void setWindSpeed(double speed);
    double windSpeed() const;

    void setWindUnits(QString units);

    void setWindDirection(double dir);
    double windDirection() const;

    void setUseDatabase(bool use);
    bool useDatabase() const;

    void setDatabasePath(QString databasePath);
    QString databasePath() const;

private:
    Ui::ConfigDialog *ui;

private slots:
    void on_browseButton_clicked();
    void on_defaultsButton_clicked();

    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void updatePlots();
    void updateReference();
};

#endif // CONFIGDIALOG_H
