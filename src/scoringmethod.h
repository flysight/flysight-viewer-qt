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

    virtual double score(const QVector< DataPoint > &result) { return 0; }
    virtual QString scoreAsText(double score) { return QString(); }

    virtual void prepareDataPlot(DataPlot *plot) {}
    virtual void prepareMapView(MapView *view) {}

    virtual bool updateReference(double lat, double lon) {}
    virtual void closeReference() {}

    virtual void optimize() {}

protected:
    void optimize(MainWindow *mainWindow, double windowBottom);

private:
    const Genome &selectGenome(const GenePool &genePool, const int tournamentSize);

signals:
    void scoringChanged();

public slots:
};

#endif // SCORINGMETHOD_H
