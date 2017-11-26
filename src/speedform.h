#ifndef SPEEDFORM_H
#define SPEEDFORM_H

#include <QWidget>

namespace Ui {
    class SpeedForm;
}

class MainWindow;

class SpeedForm : public QWidget
{
    Q_OBJECT

public:
    explicit SpeedForm(QWidget *parent = 0);
    ~SpeedForm();

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow);

    QString scoreAsText(double score);

protected:
    virtual void keyPressEvent(QKeyEvent *);

private:
    Ui::SpeedForm *ui;
    MainWindow  *mMainWindow;

public slots:
    void updateView();

private slots:
    void onFAIButtonClicked();
    void onUpButtonClicked();
    void onDownButtonClicked();
    void onApplyButtonClicked();

    void onActualButtonClicked();
    void onOptimalButtonClicked();
    void onOptimizeButtonClicked();

    void onPpcButtonClicked();
};


#endif // SPEEDFORM_H
