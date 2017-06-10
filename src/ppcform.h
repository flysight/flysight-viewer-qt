#ifndef PPCFORM_H
#define PPCFORM_H

#include <QWidget>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextCodec>
#include <QtDebug>

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
    QNetworkAccessManager *naManager;

public slots:
    void updateView();

private slots:
    void onFAIButtonClicked();
    void onUpButtonClicked();
    void onDownButtonClicked();
    void onApplyButtonClicked();

    void onModeChanged();

    void onActualButtonClicked();
    void onOptimalButtonClicked();
    void onOptimizeButtonClicked();
    void onPpcButtonClicked();
    void finished(QNetworkReply *reply);
    bool getUserDetails(QString *name, QString *countrycode, QString *wingsuit, QString *place);
};


#endif // PPCFORM_H
