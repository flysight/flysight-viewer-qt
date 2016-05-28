#ifndef WIDEOPENFORM_H
#define WIDEOPENFORM_H

#include <QWidget>

namespace Ui {
class WideOpenForm;
}

class WideOpenForm : public QWidget
{
    Q_OBJECT

public:
    explicit WideOpenForm(QWidget *parent = 0);
    ~WideOpenForm();

private:
    Ui::WideOpenForm *ui;
};

#endif // WIDEOPENFORM_H
