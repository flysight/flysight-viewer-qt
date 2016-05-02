#ifndef SCORINGMETHOD_H
#define SCORINGMETHOD_H

#include <QObject>
#include <QString>
#include <QVector>

#include "datapoint.h"

class DataPlot;

class ScoringMethod : public QObject
{
    Q_OBJECT
public:
    explicit ScoringMethod(QObject *parent = 0);

    virtual double score(const QVector< DataPoint > &result) = 0;
    virtual QString scoreAsText(double score) = 0;

    virtual void prepareDataPlot(DataPlot *plot) {}

signals:
    void dataChanged();

public slots:
};

#endif // SCORINGMETHOD_H
