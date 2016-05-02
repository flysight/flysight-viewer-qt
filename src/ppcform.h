#ifndef PPCFORM_H
#define PPCFORM_H

#include <QWidget>

namespace Ui {
    class PPCForm;
}

class MainWindow;

class PPCForm : public QWidget
{
    Q_OBJECT

public:
    explicit PPCForm(QWidget *parent = 0);
    ~PPCForm();

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow);

protected:
    virtual void keyPressEvent(QKeyEvent *);

private:
    Ui::PPCForm *ui;
    MainWindow  *mMainWindow;

public slots:
    void updateView();

private slots:
    void onFAIButtonClicked();
    void onUpButtonClicked();
    void onDownButtonClicked();
    void onApplyButtonClicked();
    void onModeChanged();
};

#endif // PPCFORM_H
