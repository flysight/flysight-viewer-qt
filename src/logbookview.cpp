#include "LogbookView.h"
#include "ui_LogbookView.h"

#include <QDateTime>
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

class TimeItem : public QTableWidgetItem
{
public:
    TimeItem(const QDateTime &dateTime, int type = Type):
        QTableWidgetItem(dateTime.toString("yyyy/MM/dd h:mm A"), type)
    {
        setData(Qt::UserRole, dateTime);
    }

    bool operator<(const QTableWidgetItem &rhs) const
    {
        return (this->data(Qt::UserRole) < rhs.data(Qt::UserRole));
    }
};

class DurationItem : public QTableWidgetItem
{
public:
    DurationItem(qint64 duration, int type = Type):
        QTableWidgetItem(duration < 3600000 ? QString("%1:%2").arg(duration / 60000)
                                                              .arg((duration / 1000) % 60, 2, 10, QChar('0')) :
                                              QString("%1:%2:%3").arg(duration / 3600000)
                                                                 .arg((duration / 60000) % 60, 2, 10, QChar('0'))
                                                                 .arg((duration / 1000) % 60, 2, 10, QChar('0')),
                         type)
    {
        setData(Qt::UserRole, duration);
    }

    bool operator<(const QTableWidgetItem &rhs) const
    {
        return (this->data(Qt::UserRole) < rhs.data(Qt::UserRole));
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
    ui->tableWidget->setSortingEnabled(false);

    QSqlDatabase db = QSqlDatabase::database("flysight");
    QSqlQuery query(db);

    if (!query.exec("select * from files"))
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(0, tr("Query failed"), err.text());
    }

    ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    ui->tableWidget->setColumnCount(query.record().count() + 1);
    ui->tableWidget->setRowCount(0);

    ui->tableWidget->setColumnWidth(0, ui->tableWidget->horizontalHeader()->minimumSectionSize());
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);

    ui->tableWidget->setColumnHidden(1, true);  // Hide id column
    ui->tableWidget->setColumnHidden(2, true);  // Hide file_name column
    ui->tableWidget->setColumnHidden(6, true);  // Hide min_lat column
    ui->tableWidget->setColumnHidden(7, true);  // Hide max_lat column
    ui->tableWidget->setColumnHidden(8, true);  // Hide min_lon column
    ui->tableWidget->setColumnHidden(9, true);  // Hide max_lon column

    ui->tableWidget->setHorizontalHeaderLabels(QStringList()
                                               << tr("")
                                               << tr("ID")
                                               << tr("File Name")
                                               << tr("Start Time")
                                               << tr("Duration")
                                               << tr("Sample Period")
                                               << tr("Min Latitude")
                                               << tr("Max Latitude")
                                               << tr("Min Longitude")
                                               << tr("Max Longitude")
                                               << tr("Import Time"));

    int index = 0;
    while (query.next())
    {
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());

        QDateTime startTime = QDateTime::fromString(query.value(2).toString(), "yyyy-MM-dd HH:mm:ss.zzz");
        QDateTime importTime = QDateTime::fromString(query.value(9).toString(), "yyyy-MM-dd HH:mm:ss.zzz");
        qint64 duration = query.value(3).toString().toLongLong();

        if (mMainWindow && mMainWindow->trackName() == query.value(1).toString())
        {
            QTableWidgetItem *item = new QTableWidgetItem;
            item->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            ui->tableWidget->setItem(index, 0, item);
        }

        ui->tableWidget->setItem(index, 1, new IntItem(query.value(0).toString()));             // id
        ui->tableWidget->setItem(index, 2, new QTableWidgetItem(query.value(1).toString()));    // file_name
        ui->tableWidget->setItem(index, 3, new TimeItem(startTime));                            // start_time
        ui->tableWidget->setItem(index, 4, new DurationItem(duration));                         // duration
        ui->tableWidget->setItem(index, 5, new IntItem(query.value(4).toString()));             // sample_period
        ui->tableWidget->setItem(index, 6, new IntItem(query.value(5).toString()));             // min_lat
        ui->tableWidget->setItem(index, 7, new IntItem(query.value(6).toString()));             // max_lat
        ui->tableWidget->setItem(index, 8, new IntItem(query.value(7).toString()));             // min_lon
        ui->tableWidget->setItem(index, 9, new IntItem(query.value(8).toString()));             // max_lon
        ui->tableWidget->setItem(index, 10, new TimeItem(importTime));                          // import_time

        ++index;
    }

    ui->tableWidget->setSortingEnabled(true);
}

void LogbookView::onDoubleClick(
        int row,
        int column)
{
    Q_UNUSED(column);

    mMainWindow->importFromDatabase(ui->tableWidget->item(row, 2)->text());
}
