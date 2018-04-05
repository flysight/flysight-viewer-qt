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

#ifndef GENOME_H
#define GENOME_H

#include <QVector>

#include "datapoint.h"
#include "mainwindow.h"

class Genome:
        public QVector< double >
{
public:
    Genome();
    Genome(const QVector< double > &rhs);
    Genome(const Genome &p1, const Genome &p2, int k);
    Genome(int genomeSize, int k, double minLift, double maxLift);

    void mutate(int k, int kMin, double minLift, double maxLift);
    void truncate(int k);
    MainWindow::DataPoints simulate(double h, double a, double c,
                                  double planformArea, double mass,
                                  const DataPoint &dp0, double windowBottom);

private:
    static double dtheta_dt(double theta, double v, double x, double y, double lift,
                            double planformArea, double mass);
    static double dv_dt(double theta, double v, double x, double y, double drag,
                        double planformArea, double mass);
    static double dx_dt(double theta, double v, double x, double y);
    static double dy_dt(double theta, double v, double x, double y);

    static double lift(double cl);
    static double drag(double cl, double a, double c);
};

#endif // GENOME_H
