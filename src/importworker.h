#ifndef IMPORTWORKER_H
#define IMPORTWORKER_H

#include <QObject>

class ImportWorker : public QObject
{
    Q_OBJECT
public:
    explicit ImportWorker();

signals:
    void importFile(QString fileName);

public slots:
    void process();
};

#endif // IMPORTWORKER_H
