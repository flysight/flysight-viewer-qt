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

#ifndef LOGBOOKVIEW_H
#define LOGBOOKVIEW_H

#include <QWidget>

namespace Ui {
    class LogbookView;
}

class MainWindow;
class QTableWidgetItem;

class LogbookView : public QWidget
{
    Q_OBJECT

public:
    explicit LogbookView(QWidget *parent = 0);
    ~LogbookView();

    void setMainWindow(MainWindow *mainWindow);

protected:
    virtual void keyPressEvent(QKeyEvent *);

private:
    Ui::LogbookView *ui;
    MainWindow      *mMainWindow;

    bool             suspendItemChanged;

public slots:
    void updateView();

private slots:
    void onDoubleClick(int row, int column);
    void onSelectionChanged();
    void onItemChanged(QTableWidgetItem *item);
    void onSearchTextChanged(const QString &text);
    void onSearchTextReturn();
};

#endif // LOGBOOKVIEW_H
