#ifndef PERFORMANCEFORM_H
#define PERFORMANCEFORM_H

#include <QWidget>

namespace Ui {
    class PerformanceForm;
}

class MainWindow;

class PerformanceForm : public QWidget
{
    Q_OBJECT

public:
    explicit PerformanceForm(QWidget *parent = 0);
    ~PerformanceForm();

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow);

protected:
    virtual void keyPressEvent(QKeyEvent *);

private:
    Ui::PerformanceForm *ui;
    MainWindow  *mMainWindow;

public slots:
    void updateView();

private slots:
    void onApplyButtonClicked();
};


#endif // PERFORMANCEFORM_H
