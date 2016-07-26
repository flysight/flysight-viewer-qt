#ifndef WIDEOPENDISTANCEFORM_H
#define WIDEOPENDISTANCEFORM_H

#include <QWidget>

namespace Ui {
class WideOpenDistanceForm;
}

class MainWindow;

class WideOpenDistanceForm : public QWidget
{
    Q_OBJECT

public:
    explicit WideOpenDistanceForm(QWidget *parent = 0);
    ~WideOpenDistanceForm();

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow);

protected:
    virtual void keyPressEvent(QKeyEvent *);

private:
    Ui::WideOpenDistanceForm *ui;
    MainWindow  *mMainWindow;

public slots:
    void updateView();

private slots:
    void onApplyButtonClicked();
    void onStartButtonClicked();
    void onEndButtonClicked();
};

#endif // WIDEOPENDISTANCEFORM_H
