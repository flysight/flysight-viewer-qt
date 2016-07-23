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

    static void intercept(double lata1, double lona1, double lata2, double lona2,
                          double latb1, double lonb1, double &lat0, double &lon0);

public slots:
    void updateView();

private slots:
    void onApplyButtonClicked();
    void onStartButtonClicked();
    void onEndButtonClicked();
};

#endif // WIDEOPENDISTANCEFORM_H
