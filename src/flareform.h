#ifndef FLAREFORM_H
#define FLAREFORM_H

#include <QWidget>

namespace Ui {
    class FlareForm;
}

class MainWindow;

class FlareForm : public QWidget
{
    Q_OBJECT

public:
    explicit FlareForm(QWidget *parent = 0);
    ~FlareForm();

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow);

private:
    Ui::FlareForm *ui;
    MainWindow  *mMainWindow;

public slots:
    void updateView();

private slots:
    void onActualButtonClicked();
    void onOptimalButtonClicked();
    void onOptimizeButtonClicked();
};


#endif // FLAREFORM_H
