#ifndef WIDEOPENFORM_H
#define WIDEOPENFORM_H

#include <QWidget>

namespace Ui {
class WideOpenForm;
}

class MainWindow;

class WideOpenForm : public QWidget
{
    Q_OBJECT

public:
    explicit WideOpenForm(QWidget *parent = 0);
    ~WideOpenForm();

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow);

protected:
    virtual void keyPressEvent(QKeyEvent *);

private:
    Ui::WideOpenForm *ui;
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

#endif // WIDEOPENFORM_H
