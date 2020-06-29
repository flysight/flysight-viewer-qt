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

#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QWebEngineView>

class MainWindow;

class QWebChannel;
class MapCore;

class MapView : public QWebEngineView
{
    Q_OBJECT
public:
    explicit MapView(QWidget *parent = 0);

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow) { mMainWindow = mainWindow; }

    double metersPerPixel() const;
    MapCore *mapCore() { return mMapCore; }

private:
    MainWindow  *mMainWindow;
    bool        mDragging;

    QWebChannel *mWebChannel;
    MapCore     *mMapCore;

    double       mLatMin, mLonMin;
    double       mLatMax, mLonMax;

    bool updateReference(QMap<QString, QVariant> latLng);
    void updateMarker(QMap<QString, QVariant> latLng);

public slots:
    void initView();
    void updateView();
    void updateCursor();

    void mouseDown(QMap<QString, QVariant> latLng);
    void mouseUp(QMap<QString, QVariant> latLng);
    void mouseOver(QMap<QString, QVariant> latLng);
    void mouseOut();
    void mouseMove(QMap<QString, QVariant> latLng);
    void boundsChanged(QMap<QString, QVariant> sw, QMap<QString, QVariant> ne);
};

#endif // MAPVIEW_H
