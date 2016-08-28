#include "LogbookView.h"
#include "ui_LogbookView.h"

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>

#include "mainwindow.h"

class IntItem : public QTableWidgetItem
{
public:
    IntItem(const QString &text, int type = Type):
        QTableWidgetItem(text, type) {}

    bool operator<(const QTableWidgetItem &rhs) const
    {
        return (this->text().toInt() < rhs.text().toInt());
    }
};

LogbookView::LogbookView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LogbookView),
    mMainWindow(0)
{
    ui->setupUi(this);

    updateView();

    connect(ui->tableWidget, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(onDoubleClick(int,int)));
}

LogbookView::~LogbookView()
{
    delete ui;
}

void LogbookView::setMainWindow(
        MainWindow *mainWindow)
{
    mMainWindow = mainWindow;
}

void LogbookView::updateView()
{
    QSqlDatabase db = QSqlDatabase::database("flysight");
    QSqlQuery query(db);

    if (!query.exec("select * from files"))
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(0, tr("Query failed"), err.text());
    }

    ui->tableWidget->setColumnCount(query.record().count() - 1);
    ui->tableWidget->setRowCount(0);

    for (int j = 1; j < query.record().count(); ++j)
    {
        ui->tableWidget->setHorizontalHeaderItem(j - 1, new QTableWidgetItem(query.record().field(j).name()));
    }

    int index = 0;
    while (query.next())
    {
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        for (int j = 1; j < query.record().count(); ++j)
        {
            switch (query.record().field(j).type())
            {
            case QVariant::Int:
                ui->tableWidget->setItem(index, j - 1, new IntItem(query.value(j).toString()));
                break;
            default:
                ui->tableWidget->setItem(index, j - 1, new QTableWidgetItem(query.value(j).toString()));
                break;
            }
        }
        ++index;
    }
}

void LogbookView::onDoubleClick(
        int row,
        int column)
{
    Q_UNUSED(column);

    mMainWindow->importFromDatabase(ui->tableWidget->item(row, 0)->text());
}
