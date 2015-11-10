#ifndef SCORINGVIEW_H
#define SCORINGVIEW_H

#include <QWidget>

namespace Ui {
    class ScoringView;
}

class MainWindow;

class ScoringView : public QWidget
{
    Q_OBJECT

public:
    explicit ScoringView(QWidget *parent = 0);
    ~ScoringView();

    virtual QSize sizeHint() const;

    void setMainWindow(MainWindow *mainWindow);

protected:
    virtual void keyPressEvent(QKeyEvent *);

private:
    Ui::ScoringView *ui;
    MainWindow      *mMainWindow;

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
};

#endif // SCORINGVIEW_H
