#include "logbookview.h"
#include "ui_logbookview.h"

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
    connect(ui->tableWidget, SIGNAL(itemSelectionChanged()),
            this, SLOT(onSelectionChanged()));
    connect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
            this, SLOT(onItemChanged(QTableWidgetItem*)));
    connect(ui->searchEdit, SIGNAL(textChanged(QString)),
            this, SLOT(onSearchTextChanged(QString)));
    connect(ui->searchEdit, SIGNAL(returnPressed()),
            this, SLOT(onSearchTextReturn()));
}

LogbookView::~LogbookView()
{
    delete ui;
}

void LogbookView::setMainWindow(
        MainWindow *mainWindow)
{
    mMainWindow = mainWindow;
    mMainWindow->setSelectedTracks(QVector< QString >());
}

void LogbookView::updateView()
{
    ui->tableWidget->setSortingEnabled(false);

    QSqlDatabase db = QSqlDatabase::database("flysight");
    QSqlQuery query(db);

    QString whereText;
    if (!this->ui->searchEdit->text().isEmpty())
    {
        QStringList searchItems = this->ui->searchEdit->text().split(QRegExp("\\s"));
        for (int i = 0; i < searchItems.length(); ++i)
        {
            if (i == 0) whereText += "where ";
            else        whereText += "and ";

            whereText += "lower(description) like lower('%" + searchItems[i] + "%')";
        }
    }

    if (!query.exec(QString("select * from files %1").arg(whereText)))
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
    ui->tableWidget->setColumnHidden(7, true);  // Hide min_lat column
    ui->tableWidget->setColumnHidden(8, true);  // Hide max_lat column
    ui->tableWidget->setColumnHidden(9, true);  // Hide min_lon column
    ui->tableWidget->setColumnHidden(10, true);  // Hide max_lon column

    ui->tableWidget->setHorizontalHeaderLabels(QStringList()
                                               << tr("")
                                               << tr("ID")
                                               << tr("File Name")
                                               << tr("Description")
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

        QDateTime startTime = QDateTime::fromString(query.value(3).toString(), "yyyy-MM-dd HH:mm:ss.zzz");
        QDateTime importTime = QDateTime::fromString(query.value(10).toString(), "yyyy-MM-dd HH:mm:ss.zzz");
        qint64 duration = query.value(4).toString().toLongLong();

        if (mMainWindow && mMainWindow->trackName() == query.value(1).toString())
        {
            QTableWidgetItem *item = new QTableWidgetItem;
            item->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            ui->tableWidget->setItem(index, 0, item);
        }
        else
        {
            ui->tableWidget->setItem(index, 0, new QTableWidgetItem);
        }

        ui->tableWidget->setItem(index, 1, new IntItem(query.value(0).toString()));             // id
        ui->tableWidget->setItem(index, 2, new QTableWidgetItem(query.value(1).toString()));    // file_name
        ui->tableWidget->setItem(index, 3, new QTableWidgetItem(query.value(2).toString()));    // description
        ui->tableWidget->setItem(index, 4, new TimeItem(startTime));                            // start_time
        ui->tableWidget->setItem(index, 5, new DurationItem(duration));                         // duration
        ui->tableWidget->setItem(index, 6, new IntItem(query.value(5).toString()));             // sample_period
        ui->tableWidget->setItem(index, 7, new IntItem(query.value(6).toString()));             // min_lat
        ui->tableWidget->setItem(index, 8, new IntItem(query.value(7).toString()));             // max_lat
        ui->tableWidget->setItem(index, 9, new IntItem(query.value(8).toString()));             // min_lon
        ui->tableWidget->setItem(index, 10, new IntItem(query.value(9).toString()));            // max_lon
        ui->tableWidget->setItem(index, 11, new TimeItem(importTime));                          // import_time

        for (int j = 0; j < 12; ++j)
        {
            // Disable editing on every column except description
            QTableWidgetItem *item = ui->tableWidget->item(index, j);
            if (j == 3) item->setFlags(item->flags() |  Qt::ItemIsEditable);
            else        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }

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

void LogbookView::onSelectionChanged()
{
    QList< QTableWidgetItem* > selectedItems = ui->tableWidget->selectedItems();

    // Get a list of selected rows
    QSet< int > selectedRows;
    foreach (QTableWidgetItem *item, selectedItems)
    {
        selectedRows.insert(item->row());
    }

    // Get a list of selected files
    QVector< QString > selectedFiles;
    foreach (int row, selectedRows)
    {
        QTableWidgetItem *item = ui->tableWidget->item(row, 2);
        selectedFiles.append(item->text());
    }

    // Update main window
    mMainWindow->setSelectedTracks(selectedFiles);
}

void LogbookView::onItemChanged(
        QTableWidgetItem *item)
{
    // Return if not editing description
    if (item->column() != 3) return;

    // Get file name
    QTableWidgetItem *nameItem = ui->tableWidget->item(item->row(), 2);
    if (!nameItem) return;

    // Update description
    mMainWindow->setTrackDescription(nameItem->text(), item->text());
}

void LogbookView::onSearchTextChanged(
        const QString &text)
{
    Q_UNUSED(text);
    updateView();
}

void LogbookView::onSearchTextReturn()
{
    // Give focus to the main window
    mMainWindow->setFocus();
}

void LogbookView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && ui->searchEdit->hasFocus())
    {
        // Clear search text
        ui->searchEdit->clear();

        // Give focus to the main window
        mMainWindow->setFocus();
    }

    QWidget::keyPressEvent(event);
}
