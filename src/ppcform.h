#ifndef PPCFORM_H
#define PPCFORM_H

#include <QWidget>

namespace Ui {
    class PPCForm;
}

class DataPlot;
class DataPoint;
class MainWindow;

class PPCForm : public QWidget
{
    Q_OBJECT

public:
    explicit PPCForm(QWidget *parent = 0);
    ~PPCForm();

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow);

    double score(const QVector< DataPoint > &result);
    QString scoreAsText(double score);

    void prepareDataPlot(DataPlot *plot);

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
};

#endif // PPCFORM_H
