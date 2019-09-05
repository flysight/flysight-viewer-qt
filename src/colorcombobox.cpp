#include "colorcombobox.h"

ColorComboBox::ColorComboBox(QWidget *parent) : QComboBox(parent)
{
    populateList();
}

QSize ColorComboBox::sizeHint() const
{
    return minimumSizeHint();
}

QSize ColorComboBox::minimumSizeHint() const
{
    return QSize(0, QComboBox::minimumSizeHint().height());
}

QColor ColorComboBox::color() const
{
    return qvariant_cast<QColor>(itemData(currentIndex(), Qt::DecorationRole));
}

void ColorComboBox::setColor(QColor color)
{
    setCurrentIndex(findData(color, int(Qt::DecorationRole)));
}

void ColorComboBox::populateList()
{
    QStringList colorNames = QColor::colorNames();

    for (int i = 0; i < colorNames.size(); ++i) {
        QColor color(colorNames[i]);
        insertItem(i, colorNames[i]);
        setItemData(i, color, Qt::DecorationRole);
    }
}
