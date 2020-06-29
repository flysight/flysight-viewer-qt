/***************************************************************************
**                                                                        **
**  FlySight Viewer                                                       **
**  Copyright 2020 Michael Cooper                                         **
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

#ifndef MAPCORE_H
#define MAPCORE_H

#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QObject>
#include <QVariant>

#include "mapview.h"

/*
    An instance of this class gets published over the WebChannel and is then accessible to HTML clients.
*/

class MapCore : public QObject
{
    Q_OBJECT

public:
    MapCore(MapView *mapView, QObject *parent = nullptr)
        : QObject(parent), mMapView(mapView)
    {

    }

signals:
    void setData(QList<QVariant> data);
    void setBounds(QMap<QString, QVariant> sw, QMap<QString, QVariant> ne);

    void setMark(QMap<QString, QVariant> latLng);
    void clearMark();

    void setMediaCursor(QMap<QString, QVariant> latLng);
    void clearMediaCursor();

    void clearAnnotations();
    void addPolyline(QList<QVariant> data);
    void addPolygon(QList<QVariant> data);

public slots:
    void message(const QString &text)
    {
        QMessageBox::information(
                    mMapView,
                    tr("Message"),
                    text);
    }

    void mouseDown(QMap<QString, QVariant> latLng)
    {
        mMapView->mouseDown(latLng);
    }
    void mouseUp(QMap<QString, QVariant> latLng)
    {
        mMapView->mouseUp(latLng);
    }
    void mouseOver(QMap<QString, QVariant> latLng)
    {
        mMapView->mouseOver(latLng);
    }
    void mouseOut()
    {
        mMapView->mouseOut();
    }
    void mouseMove(QMap<QString, QVariant> latLng)
    {
        mMapView->mouseMove(latLng);
    }

    void boundsChanged(QMap<QString, QVariant> sw, QMap<QString, QVariant> ne)
    {
        mMapView->boundsChanged(sw, ne);
    }

private:
    MapView *mMapView;
};

#endif // MAPCORE_H
