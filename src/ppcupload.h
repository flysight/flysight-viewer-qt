#ifndef PPCUPLOAD_H
#define PPCUPLOAD_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "mainwindow.h"

class PPCUpload : public QObject
{
    Q_OBJECT
public:
    explicit PPCUpload(MainWindow *mainWindow, QObject *parent = 0);
    void upload(
            const QString type,
            const double windowTop, const double windowBottom,
            const double time, const double distance, const double horizontalSpeed, const double verticalSpeed);

signals:

private:
    QNetworkAccessManager *naManager;
    MainWindow  *mMainWindow;

private slots:
    void finished(QNetworkReply *reply);
    void usersFinished(QNetworkReply *reply);
    void suitsFinished(QNetworkReply *reply);
    void placeFinished(QNetworkReply *reply);
    bool getUserDetails(QString *name, QString *countrycode, QString *wingsuit, QString *place);
};
#endif // PPCUPLOAD_H
