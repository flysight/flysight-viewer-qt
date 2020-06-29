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

#ifndef SCORINGMETHOD_H
#define SCORINGMETHOD_H

#include <QObject>
#include <QString>
#include <QVector>

#include "datapoint.h"
#include "genome.h"

class DataPlot;
class MainWindow;
class MapView;

typedef QPair< double, Genome > Score;
typedef QVector< Score > GenePool;

static bool operator<(const Score &s1, const Score &s2)
{
    return s1.first > s2.first;
}

class ScoringMethod : public QObject
{
    Q_OBJECT
public:
    explicit ScoringMethod(QObject *parent = 0);

    virtual double score(const MainWindow::DataPoints &result) { return 0; }
    virtual QString scoreAsText(double score) { return QString(); }

    virtual void prepareDataPlot(DataPlot *plot) {}
    virtual void prepareMapView(MapView *view) {}

    virtual bool updateReference(double lat, double lon) { return false; }
    virtual void closeReference() {}

    virtual void optimize() {}

    virtual void readSettings() {}
    virtual void writeSettings() {}

protected:
    void optimize(MainWindow *mainWindow, double windowBottom);

private:
    const Genome &selectGenome(const GenePool &genePool, const int tournamentSize);

signals:
    void scoringChanged();

public slots:
};

#endif // SCORINGMETHOD_H
