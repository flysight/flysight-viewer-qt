#ifndef LOGBOOKVIEW_H
#define LOGBOOKVIEW_H

#include <QWidget>

namespace Ui {
    class LogbookView;
}

class MainWindow;

class LogbookView : public QWidget
{
    Q_OBJECT

public:
    explicit LogbookView(QWidget *parent = 0);
    ~LogbookView();

    void setMainWindow(MainWindow *mainWindow);

private:
    Ui::LogbookView *ui;
    MainWindow      *mMainWindow;
};

#endif // LOGBOOKVIEW_H
