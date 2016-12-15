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
