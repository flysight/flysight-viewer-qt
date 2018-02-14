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

class RealItem : public QTableWidgetItem
{
public:
    RealItem(const QString &text, int type = Type):
        QTableWidgetItem(text, type) {}

    bool operator<(const QTableWidgetItem &rhs) const
    {
        return (this->text().toDouble() < rhs.text().toDouble());
    }
};

class TimeItem : public QTableWidgetItem
{
public:
    TimeItem(const QDateTime &dateTime, int type = Type):
        QTableWidgetItem(dateTime.toLocalTime().toString("yyyy/MM/dd h:mm A"), type)
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
    mMainWindow(0),
    suspendItemChanged(false)
{
    ui->setupUi(this);

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
    suspendItemChanged = true;

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

    ui->tableWidget->setColumnCount(query.record().count() + 2);
    ui->tableWidget->setRowCount(0);

    ui->tableWidget->setColumnWidth(0, ui->tableWidget->horizontalHeader()->minimumSectionSize());
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);

    ui->tableWidget->setColumnWidth(1, 2 * ui->tableWidget->horizontalHeader()->minimumSectionSize());
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);

    ui->tableWidget->setColumnHidden(1, true);  // Hide checked column
    ui->tableWidget->setColumnHidden(2, true);  // Hide id column
    ui->tableWidget->setColumnHidden(3, true);  // Hide file_name column
    ui->tableWidget->setColumnHidden(8, true);  // Hide min_lat column
    ui->tableWidget->setColumnHidden(9, true);  // Hide max_lat column
    ui->tableWidget->setColumnHidden(10, true);  // Hide min_lon column
    ui->tableWidget->setColumnHidden(11, true);  // Hide max_lon column
    ui->tableWidget->setColumnHidden(15, true);  // Hide course column

    ui->tableWidget->setColumnHidden(18, true);  // Hide min_t column
    ui->tableWidget->setColumnHidden(19, true);  // Hide max_t column

    ui->tableWidget->setHorizontalHeaderLabels(QStringList()
                                               << tr("")
                                               << tr("")
                                               << tr("ID")
                                               << tr("File Name")
                                               << tr("Description")
                                               << tr("Start Time")
                                               << tr("Duration")
                                               << tr("Sample Period")
                                               << tr("Minimum Latitude")
                                               << tr("Maximum Latitude")
                                               << tr("Minimum Longitude")
                                               << tr("Maximum Longitude")
                                               << tr("Import Time")
                                               << tr("Exit Time")
                                               << tr("Ground Elevation")
                                               << tr("Course Angle")
                                               << tr("Wind Speed")
                                               << tr("Wind Direction")
                                               << tr("Range Lower")
                                               << tr("Range Upper"));

    int index = 0;
    while (query.next())
    {
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());

        QDateTime startTime = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
        QDateTime importTime = QDateTime::fromString(query.value(10).toString(), Qt::ISODate);
        QDateTime exitTime = QDateTime::fromString(query.value(11).toString(), Qt::ISODate);
        qint64 duration = query.value(4).toString().toLongLong();
        double ground = query.value(12).toString().toDouble();
        double course = query.value(13).toString().toDouble();

        double windE = query.value(14).toString().toDouble();
        double windN = query.value(15).toString().toDouble();

        double windSpeed = sqrt(windE * windE + windN * windN);
        double windDir = atan2(-windE, -windN) / PI * 180;
        if (windDir < 0) windDir += 360;

        QDateTime rangeLower = QDateTime::fromString(query.value(16).toString(), Qt::ISODate);
        QDateTime rangeUpper = QDateTime::fromString(query.value(17).toString(), Qt::ISODate);

        if (mMainWindow->trackName() == query.value(1).toString())
        {
            QTableWidgetItem *item = new QTableWidgetItem;
            item->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            ui->tableWidget->setItem(index, 0, item);
        }
        else
        {
            ui->tableWidget->setItem(index, 0, new QTableWidgetItem);
        }

        QTableWidgetItem *item = new QTableWidgetItem;
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(mMainWindow->trackChecked(
                                query.value(1).toString()) ? Qt::Checked : Qt::Unchecked);
        ui->tableWidget->setItem(index, 1, item);

        ui->tableWidget->setItem(index, 2, new RealItem(query.value(0).toString()));             // id
        ui->tableWidget->setItem(index, 3, new QTableWidgetItem(query.value(1).toString()));    // file_name
        ui->tableWidget->setItem(index, 4, new QTableWidgetItem(query.value(2).toString()));    // description
        ui->tableWidget->setItem(index, 5, new TimeItem(startTime));                            // start_time
        ui->tableWidget->setItem(index, 6, new DurationItem(duration));                         // duration
        ui->tableWidget->setItem(index, 7, new RealItem(query.value(5).toString()));             // sample_period
        ui->tableWidget->setItem(index, 8, new RealItem(query.value(6).toString()));             // min_lat
        ui->tableWidget->setItem(index, 9, new RealItem(query.value(7).toString()));             // max_lat
        ui->tableWidget->setItem(index, 10, new RealItem(query.value(8).toString()));            // min_lon
        ui->tableWidget->setItem(index, 11, new RealItem(query.value(9).toString()));            // max_lon
        ui->tableWidget->setItem(index, 12, new TimeItem(importTime));                          // import_time
        ui->tableWidget->setItem(index, 13, new TimeItem(exitTime));                            // exit_time
        ui->tableWidget->setItem(index, 14, new RealItem(QString::number(ground, 'f', 3)));     // ground
        ui->tableWidget->setItem(index, 15, new RealItem(QString::number(course, 'f', 5)));     // course
        ui->tableWidget->setItem(index, 16, new RealItem(QString::number(windSpeed, 'f', 2)));  // wind_speed
        ui->tableWidget->setItem(index, 17, new RealItem(QString::number(windDir, 'f', 5)));    // wind_dir
        ui->tableWidget->setItem(index, 18, new TimeItem(rangeLower));                          // t_min
        ui->tableWidget->setItem(index, 19, new TimeItem(rangeUpper));                          // t_max

        for (int j = 0; j < ui->tableWidget->columnCount(); ++j)
        {
            // Enable/disable editing
            QTableWidgetItem *item = ui->tableWidget->item(index, j);

            switch (j)
            {
            case 4:  // description
            case 14: // ground
            case 16: // wind_speed
            case 17: // wind_dir
                item->setFlags(item->flags() |  Qt::ItemIsEditable);
                break;
            default:
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                break;
            }
        }

        ++index;
    }

    ui->tableWidget->setSortingEnabled(true);

    suspendItemChanged = false;
}

void LogbookView::onDoubleClick(
        int row,
        int column)
{
    Q_UNUSED(column);

    // Get file name
    QTableWidgetItem *nameItem = ui->tableWidget->item(row, 3);
    if (!nameItem) return;

    if (mMainWindow->trackChecked(nameItem->text()))
    {
        mMainWindow->importFromCheckedTrack(nameItem->text());
    }
    else
    {
        mMainWindow->importFromDatabase(nameItem->text());
    }
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
        QTableWidgetItem *item = ui->tableWidget->item(row, 3);
        selectedFiles.append(item->text());
    }

    // Update main window
    mMainWindow->setSelectedTracks(selectedFiles);
}

void LogbookView::onItemChanged(
        QTableWidgetItem *item)
{
    if (suspendItemChanged) return;

    // Get file name
    QTableWidgetItem *nameItem = ui->tableWidget->item(item->row(), 3);
    if (!nameItem) return;

    if (item->column() == 1)
    {
        // Update check state
        mMainWindow->setTrackChecked(nameItem->text(),
                                     item->checkState() == Qt::Checked);
    }
    else if (item->column() == 4)
    {
        // Update description
        mMainWindow->setTrackDescription(nameItem->text(), item->text());
    }
    else if (item->column() == 14)
    {
        // Update ground elevation
        mMainWindow->setTrackGround(nameItem->text(), item->text().toDouble());
    }
    else if (item->column() == 16)
    {
        // Update wind speed
        mMainWindow->setTrackWindSpeed(nameItem->text(), item->text().toDouble());
    }
    else if (item->column() == 17)
    {
        // Update wind direction
        mMainWindow->setTrackWindDir(nameItem->text(), item->text().toDouble());
    }
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
