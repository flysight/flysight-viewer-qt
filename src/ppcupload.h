/***************************************************************************
**                                                                        **
**  FlySight Viewer                                                       **
**  Copyright 2018 Klaus Rheinwald                                        **
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
**  FlySight Contact: Michael Cooper                                      **
**  FlySight Website: http://flysight.ca/                                 **
**  PPC Contact:      Klaus Rheinwald                                     **
**  PPC Website:      http://ppc.paralog.net/                             **
****************************************************************************/

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
    void upload(const QString type, const double windowTop, const double windowBottom, const double time, const double distance);

signals:

private:
    QNetworkAccessManager *naManager;
    MainWindow  *mMainWindow;

private slots:
    void finished(QNetworkReply *reply);
    void usersFinished(QNetworkReply *reply);
    void suitsFinished(QNetworkReply *reply);
    void placeFinished(QNetworkReply *reply);
    bool getUserDetails(QString *name, QString *countrycode, bool getEquipment, QString *equipment, QString *place);
};
#endif // PPCUPLOAD_H
