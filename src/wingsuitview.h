#ifndef WINGSUITVIEW_H
#define WINGSUITVIEW_H

#include <QWidget>

namespace Ui {
    class WingsuitView;
}

class MainWindow;

class WingsuitView : public QWidget
{
    Q_OBJECT

public:
    explicit WingsuitView(QWidget *parent = 0);
    ~WingsuitView();

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow) { mMainWindow = mainWindow; }

private:
    Ui::WingsuitView *ui;
    MainWindow       *mMainWindow;

public slots:
    void updateView();

private slots:
    void onFAIButtonClicked();
    void onApplyButtonClicked();
};

#endif // WINGSUITVIEW_H
