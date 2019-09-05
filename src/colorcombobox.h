#ifndef COLORCOMBOBOX_H
#define COLORCOMBOBOX_H

#include <QComboBox>

class ColorComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit ColorComboBox(QWidget *parent = 0);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public:
    QColor color() const;
    void setColor(QColor c);

private:
    void populateList();
};

#endif // COLORCOMBOBOX_H
