#ifndef WIDEOPENSPEEDFORM_H
#define WIDEOPENSPEEDFORM_H

#include <QWidget>

namespace Ui {
class WideOpenSpeedForm;
}

class MainWindow;

class WideOpenSpeedForm : public QWidget
{
    Q_OBJECT

public:
    explicit WideOpenSpeedForm(QWidget *parent = 0);
    ~WideOpenSpeedForm();

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow);

protected:
    virtual void keyPressEvent(QKeyEvent *);

private:
    Ui::WideOpenSpeedForm *ui;
    MainWindow  *mMainWindow;

public slots:
    void updateView();

private slots:
    void onApplyButtonClicked();
    void onStartButtonClicked();
    void onEndButtonClicked();
};

#endif // WIDEOPENSPEEDFORM_H
