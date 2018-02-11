#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCryptographicHash>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSettings>
#include <QShortcut>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTextStream>
#include <QThread>

#include <math.h>

#include "GeographicLib/Geodesic.hpp"

#include "common.h"
#include "configdialog.h"
#include "dataview.h"
#include "flarescoring.h"
#include "importworker.h"
#include "liftdragplot.h"
#include "logbookview.h"
#include "mapview.h"
#include "orthoview.h"
#include "performancescoring.h"
#include "playbackview.h"
#include "ppcscoring.h"
#include "scoringview.h"
#include "speedscoring.h"
#include "videoview.h"
#include "wideopendistancescoring.h"
#include "wideopenspeedscoring.h"
#include "windplot.h"

using namespace GeographicLib;

MainWindow::MainWindow(
        QWidget *parent):

    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    mMarkActive(false),
    m_viewDataRotation(0),
    m_units(PlotValue::Imperial),
    mWindowMode(Actual),
    mScoringView(0),
    m_mass(70),
    m_planformArea(2),
    m_minDrag(0.05),
    m_minLift(0.0),
    m_maxLift(0.5),
    m_maxLD(3.0),
    m_simulationTime(120),
    mLineThickness(0),
    mWindE(0),
    mWindN(0),
    mWindAdjustment(false),
    mScoringMode(PPC),
    mGroundReference(Automatic),
    mFixedReference(0)
{
    m_ui->setupUi(this);

    // Initialize scoring methods
    mScoringMethods.append(new PPCScoring(this));
    mScoringMethods.append(new SpeedScoring(this));
    mScoringMethods.append(new PerformanceScoring(this));
    mScoringMethods.append(new WideOpenSpeedScoring(this));
    mScoringMethods.append(new WideOpenDistanceScoring(this));
    mScoringMethods.append(new FlareScoring(this));

    // Read scoring method settings
    for (int i = PPC; i < smLast; ++i)
    {
        mScoringMethods[i]->readSettings();
    }

    // Connect scoring method signals
    for (int i = PPC; i < smLast; ++i)
    {
        connect(mScoringMethods[i], SIGNAL(scoringChanged()),
                this, SIGNAL(dataChanged()));
    }

    // Ensure that closeEvent is called
    connect(m_ui->actionExit, SIGNAL(triggered()),
            this, SLOT(close()));

    // Read settings
    readSettings();

    // Initialize database
    initDatabase();

    // Intitialize plot area
    initPlot();

    // Initialize 3D views
    initViews();
    initOrthoView();

    // Initialize map view
    initMapView();

    // Initialize wind view
    initWindView();

    // Initialize scoring view
    initScoringView();

    // Initialize lift/drag view
    initLiftDragView();

    // Initialize playback controls
    initPlaybackView();

    // Initialize logbook view
    initLogbookView();

    // Restore window state
    QSettings settings("FlySight", "Viewer");
    settings.beginGroup("mainWindow");
        restoreGeometry(settings.value("geometry").toByteArray());
        restoreState(settings.value("state").toByteArray());
    settings.endGroup();

    // Set default tool
    setTool(Pan);

    // Redraw plots
    emit dataChanged();

    // Create interprocess import worker
    QThread *thread = new QThread;
    ImportWorker *worker = new ImportWorker;
    worker->moveToThread(thread);

    // Attach interprocess import worker
    connect(thread, SIGNAL(started()), worker, SLOT(process()));
    connect(worker, SIGNAL(importFile(QString)), this, SLOT(importFile(QString)));

    // Start worker thread
    thread->start();

    // Set up zoom timer
    zoomTimer = new QTimer(this);
    zoomTimer->setSingleShot(true);

    connect(zoomTimer, SIGNAL(timeout()), this, SLOT(saveZoom()));

    // Disable zoom undo/redo controls
    m_ui->actionUndoZoom->setEnabled(false);
    m_ui->actionRedoZoom->setEnabled(false);
    m_ui->actionZoomToExtent->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::writeSettings()
{
    QSettings settings("FlySight", "Viewer");

    settings.beginGroup("mainWindow");
        settings.setValue("geometry", saveGeometry());
        settings.setValue("state", saveState());
        settings.setValue("units", m_units);
        settings.setValue("mass", m_mass);
        settings.setValue("planformArea", m_planformArea);
        settings.setValue("minDrag", m_minDrag);
        settings.setValue("minLift", m_minLift);
        settings.setValue("maxLift", m_maxLift);
        settings.setValue("maxLD", m_maxLD);
        settings.setValue("simulationTime", m_simulationTime);
        settings.setValue("lineThickness", mLineThickness);
        settings.setValue("windE", mWindE);
        settings.setValue("windN", mWindN);
        settings.setValue("scoringMode", mScoringMode);
        settings.setValue("groundReference", mGroundReference);
        settings.setValue("fixedReference", mFixedReference);
        settings.setValue("databasePath", mDatabasePath);
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings("FlySight", "Viewer");

    settings.beginGroup("mainWindow");
        m_units = (PlotValue::Units) settings.value("units", m_units).toInt();
        m_mass = settings.value("mass", m_mass).toDouble();
        m_planformArea = settings.value("planformArea", m_planformArea).toDouble();
        m_minDrag = settings.value("minDrag", m_minDrag).toDouble();
        m_minLift = settings.value("minLift", m_minLift).toDouble();
        m_maxLift = settings.value("maxLift", m_maxLift).toDouble();
        m_maxLD = settings.value("maxLD", m_maxLD).toDouble();
        m_simulationTime = settings.value("simulationTime", m_simulationTime).toInt();
        mLineThickness = settings.value("lineThickness", mLineThickness).toDouble();
        mWindE = settings.value("windE", mWindE).toDouble();
        mWindN = settings.value("windN", mWindN).toDouble();
        mScoringMode = (ScoringMode) settings.value("scoringMode", mScoringMode).toInt();
    	mGroundReference = (GroundReference) settings.value("groundReference", mGroundReference).toInt();
	    mFixedReference = settings.value("fixedReference", mFixedReference).toDouble();
        mDatabasePath = settings.value("databasePath",
                                       QStandardPaths::writableLocation(
                                           QStandardPaths::DocumentsLocation)).toString();
    settings.endGroup();
}

void MainWindow::initDatabase()
{
    QDir(mDatabasePath).mkpath("FlySight");
    QString path = QDir(mDatabasePath).filePath("FlySight/FlySight.db");

    mDatabase = QSqlDatabase::addDatabase("QSQLITE", "flysight");
    mDatabase.setDatabaseName(path);

    if (!mDatabase.open())
    {
        QSqlError err = mDatabase.lastError();
        QMessageBox::critical(0, tr("Failed to open database"), err.text());
        return;
    }

    if (!mDatabase.tables().contains("files"))
    {
        // Create table
        QSqlQuery query(mDatabase);
        if (!query.exec(QString("create table files ("
                                    "id integer primary key, "
                                    "file_name text, "
                                    "description text, "
                                    "start_time text, "
                                    "duration integer, "
                                    "sample_period integer, "
                                    "min_lat integer, "
                                    "max_lat integer, "
                                    "min_lon integer, "
                                    "max_lon integer, "
                                    "import_time text)")))
        {
            QSqlError err = query.lastError();
            QMessageBox::critical(0, tr("Query failed"), err.text());
        }
    }

    // Add exit, ground and course
    QSqlQuery query(mDatabase);
    query.exec("alter table files add column exit text");
    query.exec("alter table files add column ground real");
    query.exec("alter table files add column course real");

    // Add wind speed and direction
    query.exec("alter table files add column wind_e real");
    query.exec("alter table files add column wind_n real");

    // Add zoom range
    query.exec("alter table files add column t_min real");
    query.exec("alter table files add column t_max real");
}

void MainWindow::initPlot()
{
    updateBottomActions();
    updateLeftActions();

    m_ui->plotArea->setMainWindow(this);

    connect(this, SIGNAL(dataChanged()),
            m_ui->plotArea, SLOT(updatePlot()));
    connect(this, SIGNAL(cursorChanged()),
            m_ui->plotArea, SLOT(updateCursor()));
}

void MainWindow::initViews()
{
    initSingleView(tr("Top View"), "topView", m_ui->actionShowTopView, DataView::Top);
    initSingleView(tr("Side View"), "leftView", m_ui->actionShowLeftView, DataView::Left);
    initSingleView(tr("Front View"), "frontView", m_ui->actionShowFrontView, DataView::Front);
}

void MainWindow::initSingleView(
        const QString &title,
        const QString &objectName,
        QAction *actionShow,
        DataView::Direction direction)
{
    DataView *dataView = new DataView;
    QDockWidget *dockWidget = new QDockWidget(title);
    dockWidget->setWidget(dataView);
    dockWidget->setObjectName(objectName);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    dataView->setMainWindow(this);
    dataView->setDirection(direction);

    connect(actionShow, SIGNAL(toggled(bool)),
            dockWidget, SLOT(setVisible(bool)));
    connect(dockWidget, SIGNAL(visibilityChanged(bool)),
            actionShow, SLOT(setChecked(bool)));

    connect(this, SIGNAL(dataChanged()),
            dataView, SLOT(updateView()));
    connect(this, SIGNAL(cursorChanged()),
            dataView, SLOT(updateCursor()));
    connect(this, SIGNAL(rotationChanged(double)),
            dataView, SLOT(updateView()));
}

void MainWindow::initMapView()
{
    MapView *mapView = new MapView;
    QDockWidget *dockWidget = new QDockWidget(tr("Map View"));
    dockWidget->setWidget(mapView);
    dockWidget->setObjectName("mapView");
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    mapView->setMainWindow(this);

    connect(m_ui->actionShowMapView, SIGNAL(toggled(bool)),
            dockWidget, SLOT(setVisible(bool)));
    connect(dockWidget, SIGNAL(visibilityChanged(bool)),
            m_ui->actionShowMapView, SLOT(setChecked(bool)));

    connect(this, SIGNAL(dataLoaded()),
            mapView, SLOT(initView()));
    connect(this, SIGNAL(dataChanged()),
            mapView, SLOT(updateView()));
    connect(this, SIGNAL(cursorChanged()),
            mapView, SLOT(updateView()));
}

void MainWindow::initWindView()
{
    WindPlot *windPlot = new WindPlot;
    QDockWidget *dockWidget = new QDockWidget(tr("Wind View"));
    dockWidget->setWidget(windPlot);
    dockWidget->setObjectName("windView");
    dockWidget->setVisible(false);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    windPlot->setMainWindow(this);

    connect(m_ui->actionShowWindView, SIGNAL(toggled(bool)),
            dockWidget, SLOT(setVisible(bool)));
    connect(dockWidget, SIGNAL(visibilityChanged(bool)),
            m_ui->actionShowWindView, SLOT(setChecked(bool)));

    connect(this, SIGNAL(dataChanged()),
            windPlot, SLOT(updatePlot()));
    connect(this, SIGNAL(cursorChanged()),
            windPlot, SLOT(updatePlot()));
}

void MainWindow::initScoringView()
{
    mScoringView = new ScoringView;
    QDockWidget *dockWidget = new QDockWidget(tr("Scoring"));
    dockWidget->setWidget(mScoringView);
    dockWidget->setObjectName("scoringView");
    dockWidget->setVisible(false);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    mScoringView->setMainWindow(this);

    connect(m_ui->actionShowScoringView, SIGNAL(toggled(bool)),
            dockWidget, SLOT(setVisible(bool)));
    connect(m_ui->actionShowScoringView, SIGNAL(toggled(bool)),
            this, SLOT(setScoringVisible(bool)));
    connect(dockWidget, SIGNAL(visibilityChanged(bool)),
            m_ui->actionShowScoringView, SLOT(setChecked(bool)));

    connect(this, SIGNAL(dataChanged()),
            mScoringView, SLOT(updateView()));
}

void MainWindow::initLiftDragView()
{
    LiftDragPlot *liftDragPlot = new LiftDragPlot;
    QDockWidget *dockWidget = new QDockWidget(tr("Drag Polar"));
    dockWidget->setWidget(liftDragPlot);
    dockWidget->setObjectName("liftDragView");
    dockWidget->setVisible(false);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    liftDragPlot->setMainWindow(this);

    connect(m_ui->actionShowLiftDragView, SIGNAL(toggled(bool)),
            dockWidget, SLOT(setVisible(bool)));
    connect(dockWidget, SIGNAL(visibilityChanged(bool)),
            m_ui->actionShowLiftDragView, SLOT(setChecked(bool)));

    connect(this, SIGNAL(dataChanged()),
            liftDragPlot, SLOT(updatePlot()));
    connect(this, SIGNAL(cursorChanged()),
            liftDragPlot, SLOT(updatePlot()));
    connect(this, SIGNAL(aeroChanged()),
            liftDragPlot, SLOT(updatePlot()));
}

void MainWindow::initOrthoView()
{
    OrthoView *orthoView = new OrthoView;
    QDockWidget *dockWidget = new QDockWidget(tr("Ortho View"));
    dockWidget->setWidget(orthoView);
    dockWidget->setObjectName("orthoView");
    dockWidget->setVisible(false);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    orthoView->setMainWindow(this);

    connect(m_ui->actionShowOrthoView, SIGNAL(toggled(bool)),
            dockWidget, SLOT(setVisible(bool)));
    connect(dockWidget, SIGNAL(visibilityChanged(bool)),
            m_ui->actionShowOrthoView, SLOT(setChecked(bool)));

    connect(this, SIGNAL(dataChanged()),
            orthoView, SLOT(updateView()));
    connect(this, SIGNAL(cursorChanged()),
            orthoView, SLOT(updateView()));
}

void MainWindow::initPlaybackView()
{
    PlaybackView *playbackView = new PlaybackView;
    QDockWidget *dockWidget = new QDockWidget(tr("Playback View"));
    dockWidget->setWidget(playbackView);
    dockWidget->setObjectName("playbackView");
    dockWidget->setVisible(false);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    playbackView->setMainWindow(this);

    connect(m_ui->actionShowPlaybackView, SIGNAL(toggled(bool)),
            dockWidget, SLOT(setVisible(bool)));
    connect(dockWidget, SIGNAL(visibilityChanged(bool)),
            m_ui->actionShowPlaybackView, SLOT(setChecked(bool)));

    connect(this, SIGNAL(dataChanged()),
            playbackView, SLOT(updateView()));
    connect(this, SIGNAL(cursorChanged()),
            playbackView, SLOT(updateView()));
}

void MainWindow::initLogbookView()
{
    LogbookView *logbookView = new LogbookView;
    QDockWidget *dockWidget = new QDockWidget(tr("Logbook View"));
    dockWidget->setWidget(logbookView);
    dockWidget->setObjectName("logbookView");
    dockWidget->setVisible(false);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    logbookView->setMainWindow(this);
    logbookView->updateView();

    connect(m_ui->actionShowLogbookView, SIGNAL(toggled(bool)),
            dockWidget, SLOT(setVisible(bool)));
    connect(dockWidget, SIGNAL(visibilityChanged(bool)),
            m_ui->actionShowLogbookView, SLOT(setChecked(bool)));

    connect(this, SIGNAL(databaseChanged()),
            logbookView, SLOT(updateView()));
}

void MainWindow::closeEvent(
        QCloseEvent *event)
{
    // Save window state
    writeSettings();

    // Write scoring method settings
    for (int i = PPC; i < smLast; ++i)
    {
        mScoringMethods[i]->writeSettings();
    }

    // Okay to close
    event->accept();
}

DataPoint MainWindow::interpolateDataT(
        double t)
{
    const int i1 = findIndexBelowT(t);
    const int i2 = findIndexAboveT(t);

    if (i1 < 0)
    {
        return m_data.first();
    }
    else if (i2 >= m_data.size())
    {
        return m_data.last();
    }
    else
    {
        const DataPoint &dp1 = m_data[i1];
        const DataPoint &dp2 = m_data[i2];
        return DataPoint::interpolate(dp1, dp2, (t - dp1.t) / (dp2.t - dp1.t));
    }
}

int MainWindow::findIndexBelowT(
        double t)
{
    int below = -1;
    int above = m_data.size();

    while (below + 1 != above)
    {
        int mid = (below + above) / 2;
        const DataPoint &dp = m_data[mid];

        if (dp.t < t) below = mid;
        else          above = mid;
    }

    return below;
}

int MainWindow::findIndexAboveT(
        double t)
{
    int below = -1;
    int above = m_data.size();

    while (below + 1 != above)
    {
        int mid = (below + above) / 2;
        const DataPoint &dp = m_data[mid];

        if (dp.t > t) above = mid;
        else          below = mid;
    }

    return above;
}

int MainWindow::findIndexForLanding()
{
    int i = findIndexBelowT(0.0);

    while (++i < m_data.size()-1) {
        const DataPoint &p = m_data[i];
        if (p.velE*p.velE + p.velN*p.velN + p.velD < 1.0)
            break;
    }

    return i;
}

void MainWindow::on_actionImport_triggered()
{
    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get file to import
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                                                          tr("Import Tracks"),
                                                          settings.value("folder").toString(),
                                                          tr("CSV Files (*.csv)"));

    // Sort files from oldest to newest
    qSort(fileNames);

    // Import each file
    foreach (QString fileName, fileNames)
    {
        importFile(fileName);
    }
}

void MainWindow::on_actionImportFolder_triggered()
{
    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get folder to import
    QString folderName = QFileDialog::getExistingDirectory(this,
                                                           tr("Import Folder"),
                                                           settings.value("folder").toString(),
                                                           QFileDialog::ShowDirsOnly);

    // Import each file
    importFolder(folderName);
}

void MainWindow::importFolder(
        QString folderName)
{
    QDir dir(folderName);

    // Import each file in this folder
    foreach (QString fileName, dir.entryList(QStringList() << "*.csv",
                                             QDir::Files,
                                             QDir::Name))
    {
        importFile(dir.absoluteFilePath(fileName));
    }

    // Follow subfolders
    foreach (QString child, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot,
                                          QDir::Name))
    {
        importFolder(dir.absoluteFilePath(child));
    }
}

void MainWindow::importFile(
        QString fileName)
{
    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Remember last file read
    settings.setValue("folder", QFileInfo(fileName).absoluteFilePath());

    // Create temporary file
    QTemporaryFile temporaryFile;
    if (!temporaryFile.open())
    {
        QMessageBox::critical(0, tr("Import failed"), tr("Couldn't create temporary file"));
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(0, tr("Import failed"), tr("Couldn't read file"));
        return;
    }

    // Copy to temporary file
    temporaryFile.write(file.readAll());
    file.close();

    // Get hash
    QCryptographicHash hash(QCryptographicHash::Md5);

    temporaryFile.seek(0);
    if (!hash.addData(&temporaryFile))
    {
        QMessageBox::critical(0, tr("Import failed"), tr("Couldn't generate hash"));
        return;
    }

    // Get name of file in database
    QString uniqueName = QString(hash.result().toHex());
    QString newName = QString("FlySight/Tracks/%1.csv").arg(uniqueName);
    QString newPath = QDir(mDatabasePath).filePath(newName);

    QSqlQuery query(mDatabase);

    // Check if the file is in the database
    if (!query.exec(QString("select * from files where file_name='%1'")
                    .arg(uniqueName)))
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(0, tr("Query failed"), err.text());
        return;
    }

    bool isPresent = query.next();

    // Add an empty record if the file is not in the database
    if (!isPresent)
    {
        if (!query.exec(QString("insert into files (file_name) values ('%1')")
                      .arg(uniqueName)))
        {
            QSqlError err = query.lastError();
            QMessageBox::critical(0, tr("Query failed"), err.text());
        }
    }

    // Read file data
    temporaryFile.seek(0);
    import(&temporaryFile, m_data, uniqueName, true);

    // Clear optimum
    m_optimal.clear();

    // Initialize plot ranges
    initRange(uniqueName);

    emit dataLoaded();

    // If the file is not already in the database
    if (!isPresent)
    {
        QDir(mDatabasePath).mkpath("FlySight/Tracks");

        if (temporaryFile.copy(newPath))
        {
            QDateTime startTime = m_data.front().dateTime;
            qint64 duration = startTime.msecsTo(m_data.back().dateTime);

            int minLat = 900000000,  maxLat = -900000000;
            int minLon = 1800000000, maxLon = -1800000000;

            QVector< double > dt;
            for (int i = 0; i < m_data.size(); ++i)
            {
                if (i > 0)
                {
                    dt.push_back(m_data[i - 1].dateTime.msecsTo(m_data[i].dateTime));
                }

                int lat = m_data[i].lat * 10000000;
                int lon = m_data[i].lon * 10000000;

                if (lat < minLat) minLat = lat;
                if (lat > maxLat) maxLat = lat;
                if (lon < minLon) minLon = lon;
                if (lon > maxLon) maxLon = lon;
            }
            qSort(dt);
            qint64 samplePeriod = dt[dt.size() / 2];

            QDateTime importTime = QDateTime::currentDateTime();

            if (!query.exec(QString("update files set "
                                    "description='', "
                                    "start_time='%1', "
                                    "duration=%2, "
                                    "sample_period=%3, "
                                    "min_lat=%4, "
                                    "max_lat=%5, "
                                    "min_lon=%6, "
                                    "max_lon=%7, "
                                    "import_time='%8' "
                                    "where file_name='%9'")
                            .arg(dateTimeToUTC(startTime))
                            .arg(duration)
                            .arg(samplePeriod)
                            .arg(minLat)
                            .arg(maxLat)
                            .arg(minLon)
                            .arg(maxLon)
                            .arg(dateTimeToUTC(importTime))
                            .arg(uniqueName)))
            {
                QSqlError err = query.lastError();
                QMessageBox::critical(0, tr("Query failed"), err.text());
            }
        }
        else
        {
            QMessageBox::critical(0, tr("Import failed"), tr("Couldn't copy temporary file"));
        }
    }

    // Delete temporary file
    temporaryFile.close();
    temporaryFile.remove();

    // Remember current track
    setTrackName(uniqueName);
}

void MainWindow::importFromDatabase(
        const QString &uniqueName)
{
    // Get name of file in database
    QString newName = QString("FlySight/Tracks/%1.csv").arg(uniqueName);
    QString newPath = QDir(mDatabasePath).filePath(newName);

    QFile file(newPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(0, tr("Import failed"), tr("Couldn't read file"));
        return;
    }

    // Read file data
    import(&file, m_data, uniqueName, false);

    // Clear optimum
    m_optimal.clear();

    // Initialize plot ranges
    initRange(uniqueName);

    emit dataLoaded();

    // Remember current track
    setTrackName(uniqueName);
}

void MainWindow::setTrackName(
        const QString &trackName)
{
    mTrackName = trackName;
    emit databaseChanged();
}

void MainWindow::setSelectedTracks(
        QVector< QString > tracks)
{
    mSelectedTracks = tracks;
    m_ui->actionDeleteTrack->setEnabled(!mSelectedTracks.empty());
}

void MainWindow::setTrackDescription(
        const QString &trackName,
        const QString &description)
{
    QSqlQuery query(mDatabase);

    // Check the old description
    if (!query.exec(QString("select * from files where (file_name='%1' and description='%2')")
                    .arg(trackName).arg(description)))
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(0, tr("Query failed"), err.text());
        return;
    }

    // Return now if description is not changed
    if (query.next()) return;

    // Change the decription
    if (!query.exec(QString("update files set description='%1' where file_name='%2'")
                    .arg(description).arg(trackName)))
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(0, tr("Query failed"), err.text());
        return;
    }

    emit databaseChanged();
}

void MainWindow::setTrackChecked(
        const QString &trackName,
        bool checked)
{
    if (checked)
    {
        DataPoints data;

        if (trackName == mTrackName)
        {
            data = m_data;
        }
        else
        {
            // Get name of file in database
            QString newName = QString("FlySight/Tracks/%1.csv").arg(trackName);
            QString newPath = QDir(mDatabasePath).filePath(newName);

            QFile file(newPath);
            if (!file.open(QIODevice::ReadOnly))
            {
                QMessageBox::critical(0, tr("Import failed"), tr("Couldn't read file"));
                return;
            }

            // Read file data
            import(&file, data, trackName, false);
        }

        mCheckedTracks.insert(trackName, data);
    }
    else
    {
        mCheckedTracks.remove(trackName);
    }

    emit dataChanged();
}

bool MainWindow::trackChecked(
        const QString &trackName) const
{
    return mCheckedTracks.contains(trackName);
}

void MainWindow::importFromCheckedTrack(
        const QString &uniqueName)
{
    // Copy track data
    m_data = mCheckedTracks[uniqueName];

    // Clear optimum
    m_optimal.clear();

    // Initialize plot ranges
    initRange(uniqueName);

    emit dataLoaded();

    // Remember current track
    setTrackName(uniqueName);
}

void MainWindow::import(
        QIODevice *device,
        DataPoints &data,
        QString trackName,
        bool initDatabase)
{
    QTextStream in(device);

    // Column enumeration
    typedef enum {
        Time = 0,
        Lat,
        Lon,
        HMSL,
        VelN,
        VelE,
        VelD,
        HAcc,
        VAcc,
        SAcc,
        Heading,
        CAcc,
        NumSV
    } Columns;

    // Read column labels
    QMap< int, int > colMap;
    if (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList cols = line.split(",");

        for (int i = 0; i < cols.size(); ++i)
        {
            const QString &s = cols[i];

            if (s == "time")    colMap[Time]    = i;
            if (s == "lat")     colMap[Lat]     = i;
            if (s == "lon")     colMap[Lon]     = i;
            if (s == "hMSL")    colMap[HMSL]    = i;
            if (s == "velN")    colMap[VelN]    = i;
            if (s == "velE")    colMap[VelE]    = i;
            if (s == "velD")    colMap[VelD]    = i;
            if (s == "hAcc")    colMap[HAcc]    = i;
            if (s == "vAcc")    colMap[VAcc]    = i;
            if (s == "sAcc")    colMap[SAcc]    = i;
            if (s == "numSV")   colMap[NumSV]   = i;
        }
    }

    // Skip next row
    if (!in.atEnd()) in.readLine();

    data.clear();

    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList cols = line.split(",");

        DataPoint pt;

        pt.dateTime = QDateTime::fromString(cols[colMap[Time]], Qt::ISODate);

        pt.hasGeodetic = true;

        pt.lat   = cols[colMap[Lat]].toDouble();
        pt.lon   = cols[colMap[Lon]].toDouble();
        pt.hMSL  = cols[colMap[HMSL]].toDouble();

        pt.velN  = cols[colMap[VelN]].toDouble();
        pt.velE  = cols[colMap[VelE]].toDouble();
        pt.velD  = cols[colMap[VelD]].toDouble();

        pt.hAcc  = cols[colMap[HAcc]].toDouble();
        pt.vAcc  = cols[colMap[VAcc]].toDouble();
        pt.sAcc  = cols[colMap[SAcc]].toDouble();

        pt.numSV = cols[colMap[NumSV]].toDouble();

        data.append(pt);
    }

    // Initialize time
    initTime(data, trackName, initDatabase);

    // Altitude above ground
    initAltitude(data, trackName, initDatabase);

    // Wind adjustments
    updateVelocity(data, trackName, initDatabase);
}

void MainWindow::initTime(
        DataPoints &data,
        QString trackName,
        bool initDatabase)
{
    QString value;
    qint64 start;
    if (getDatabaseValue(trackName, "exit", value))
    {
        start = QDateTime::fromString(value, Qt::ISODate)
                .toMSecsSinceEpoch();
    }
    else
    {
        const DataPoint &dp0 = data[data.size() - 1];
        start = dp0.dateTime.toMSecsSinceEpoch();
    }

    if (initDatabase)
    {
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(start, Qt::UTC);
        setDatabaseValue(trackName, "exit", dateTimeToUTC(dt));
    }

    for (int i = 0; i < data.size(); ++i)
    {
        DataPoint &dp = data[i];
        qint64 end = dp.dateTime.toMSecsSinceEpoch();
        dp.t = (double) (end - start) / 1000;
    }
}

void MainWindow::initAltitude(
        DataPoints &data,
        QString trackName,
        bool initDatabase)
{
    QString value;
    double ground;
    if (getDatabaseValue(trackName, "ground", value))
    {
        ground = value.toDouble();
    }
    else if (mGroundReference == Automatic)
    {
        const DataPoint &dp0 = data[data.size() - 1];
        ground = dp0.hMSL;
    }
    else
    {
        ground = mFixedReference;
    }

    if (initDatabase)
    {
        setDatabaseValue(trackName, "ground", QString::number(ground, 'f', 3));
    }

    for (int i = 0; i < data.size(); ++i)
    {
        DataPoint &dp = data[i];
        dp.z = dp.hMSL - ground;
    }
}

void MainWindow::updateVelocity(
        DataPoints &data,
        QString trackName,
        bool initDatabase)
{
    double windE, windN;
    getWind(trackName, &windE, &windN);

    if (initDatabase)
    {
        setDatabaseValue(trackName, "wind_e", QString::number(windE, 'f', 2));
        setDatabaseValue(trackName, "wind_n", QString::number(windN, 'f', 2));
    }

    if (mWindAdjustment)
    {
        // Wind-adjusted position
        for (int i = 0; i < data.size(); ++i)
        {
            const DataPoint &dp0 = interpolateDataT(0);
            DataPoint &dp = data[i];

            double distance = getDistance(dp0, dp);
            double bearing = getBearing(dp0, dp);

            dp.x = distance * sin(bearing) - windE * dp.t;
            dp.y = distance * cos(bearing) - windN * dp.t;
        }

        // Wind-adjusted velocity
        for (int i = 0; i < data.size(); ++i)
        {
            DataPoint &dp = data[i];

            dp.vx = dp.velE - windE;
            dp.vy = dp.velN - windN;
        }
    }
    else
    {
        // Unadjusted position
        for (int i = 0; i < data.size(); ++i)
        {
            const DataPoint &dp0 = interpolateDataT(0);
            DataPoint &dp = data[i];

            double distance = getDistance(dp0, dp);
            double bearing = getBearing(dp0, dp);

            dp.x = distance * sin(bearing);
            dp.y = distance * cos(bearing);
        }

        // Unadjusted velocity
        for (int i = 0; i < data.size(); ++i)
        {
            DataPoint &dp = data[i];

            dp.vx = dp.velE;
            dp.vy = dp.velN;
        }
    }

    // Distance measurements
    double dist2D = 0, dist3D = 0;

    for (int i = 0; i < data.size(); ++i)
    {
        DataPoint &dp = data[i];

        if (i > 0)
        {
            const DataPoint &dpPrev = data[i - 1];

            double dx = dp.x - dpPrev.x;
            double dy = dp.y - dpPrev.y;
            double dh = sqrt(dx * dx + dy * dy);
            double dz = dp.hMSL - dpPrev.hMSL;

            dist2D += dh;
            dist3D += sqrt(dh * dh + dz * dz);
        }

        dp.dist2D = dist2D;
        dp.dist3D = dist3D;
    }

    QString value;
    double theta0;
    if (getDatabaseValue(trackName, "course", value))
    {
        theta0 = value.toDouble();
    }
    else
    {
        theta0 = 0;
    }

    if (initDatabase)
    {
        setDatabaseValue(trackName, "course", QString::number(theta0, 'f', 5));
    }

    // Cumulative heading
    double prevHeading;
    bool firstHeading = true;

    for (int i = 0; i < data.size(); ++i)
    {
        DataPoint &dp = data[i];

        // Calculate heading
        dp.heading = atan2(dp.vx, dp.vy) / PI * 180;

        // Calculate heading accuracy
        const double s = DataPoint::totalSpeed(dp);
        if (s != 0) dp.cAcc = dp.sAcc / s;
        else        dp.cAcc = 0;

        // Adjust heading
        if (!firstHeading)
        {
            while (dp.heading <  prevHeading - 180) dp.heading += 360;
            while (dp.heading >= prevHeading + 180) dp.heading -= 360;
        }

        // Relative heading
        dp.theta = dp.heading - theta0;

        firstHeading = false;
        prevHeading = dp.heading;
    }

    // Parameters depending on velocity
    for (int i = 0; i < data.size(); ++i)
    {
        DataPoint &dp = data[i];

        dp.curv = getSlope(i, DataPoint::diveAngle);
        dp.accel = getSlope(i, DataPoint::totalSpeed);
        dp.omega = getSlope(i, DataPoint::course);
    }

    // Initialize aerodynamics
    initAerodynamics(data);
}

void MainWindow::initAerodynamics(
        DataPoints &data)
{
    for (int i = 0; i < data.size(); ++i)
    {
        DataPoint &dp = data[i];

        // Acceleration
        double accelN = getSlope(i, DataPoint::northSpeed);
        double accelE = getSlope(i, DataPoint::eastSpeed);
        double accelD = getSlope(i, DataPoint::verticalSpeed);

        // Subtract acceleration due to gravity
        accelD -= A_GRAVITY;

        // Calculate acceleration due to drag
        const double vel = DataPoint::totalSpeed(dp);
        const double proj = (accelN * dp.vy + accelE * dp.vx + accelD * dp.velD) / vel;

        const double dragN = proj * dp.vy / vel;
        const double dragE = proj * dp.vx / vel;
        const double dragD = proj * dp.velD / vel;

        const double accelDrag = sqrt(dragN * dragN + dragE * dragE + dragD * dragD);

        // Calculate acceleration due to lift
        const double liftN = accelN - dragN;
        const double liftE = accelE - dragE;
        const double liftD = accelD - dragD;

        const double accelLift = sqrt(liftN * liftN + liftE * liftE + liftD * liftD);

        // From https://en.wikipedia.org/wiki/Atmospheric_pressure#Altitude_variation
        const double airPressure = SL_PRESSURE * pow(1 - LAPSE_RATE * dp.hMSL / SL_TEMP, A_GRAVITY * MM_AIR / GAS_CONST / LAPSE_RATE);

        // From https://en.wikipedia.org/wiki/Lapse_rate
        const double temperature = SL_TEMP - LAPSE_RATE * dp.hMSL;

        // From https://en.wikipedia.org/wiki/Density_of_air
        const double airDensity = airPressure / (GAS_CONST / MM_AIR) / temperature;

        // From https://en.wikipedia.org/wiki/Dynamic_pressure
        const double dynamicPressure = airDensity * vel * vel / 2;

        // Calculate lift and drag coefficients
        dp.lift = m_mass * accelLift / dynamicPressure / m_planformArea;
        dp.drag = m_mass * accelDrag / dynamicPressure / m_planformArea;
    }
}

double MainWindow::getSlope(
        const int center,
        double (*value)(const DataPoint &)) const
{
    int iMin = qMax (0, center - 2);
    int iMax = qMin (m_data.size () - 1, center + 2);

    double sumx = 0, sumy = 0, sumxx = 0, sumxy = 0;

    for (int i = iMin; i <= iMax; ++i)
    {
        const DataPoint &dp = m_data[i];
        double y = value(dp);

        sumx += dp.t;
        sumy += y;
        sumxx += dp.t * dp.t;
        sumxy += dp.t * y;
    }

    int n = iMax - iMin + 1;
    return (sumxy - sumx * sumy / n) / (sumxx - sumx * sumx / n);
}

double MainWindow::getDistance(
        const DataPoint &dp1,
        const DataPoint &dp2)
{
    if (dp1.hasGeodetic && dp2.hasGeodetic)
    {
        const Geodesic &geod = Geodesic::WGS84();
        double s12;

        geod.Inverse(dp1.lat, dp1.lon, dp2.lat, dp2.lon, s12);

        return s12;
    }
    else
    {
        const double dx = dp2.x - dp1.x;
        const double dy = dp2.y - dp1.y;

        return sqrt(dx * dx + dy * dy);
    }
}

double MainWindow::getBearing(
        const DataPoint &dp1,
        const DataPoint &dp2)
{
    const Geodesic &geod = Geodesic::WGS84();
    double azi1, azi2;

    geod.Inverse(dp1.lat, dp1.lon, dp2.lat, dp2.lon, azi1, azi2);

    return azi1 / 180 * PI;
}

void MainWindow::setMark(
        double start,
        double end)
{
    if (m_data.isEmpty()) return;

    if (start >= m_data.front().t &&
            start <= m_data.back().t &&
            end >= m_data.front().t &&
            end <= m_data.back().t)
    {
        mMarkStart = start;
        mMarkEnd = end;
        mMarkActive = true;
    }
    else
    {
        mMarkActive = false;
    }

    emit cursorChanged();
}

void MainWindow::setMark(
        double mark)
{
    if (m_data.isEmpty()) return;

    if (mark >= m_data.front().t &&
            mark <= m_data.back().t)
    {
        mMarkStart = mMarkEnd = mark;
        mMarkActive = true;
    }
    else
    {
        mMarkActive = false;
    }

    emit cursorChanged();
}

void MainWindow::clearMark()
{
    mMarkActive = false;

    emit cursorChanged();
}

void MainWindow::initRange(
        QString trackName)
{
    // Clear zoom stack
    mZoomLevelUndo.clear();
    mZoomLevelRedo.clear();

    QString strMin, strMax;
    if (getDatabaseValue(trackName, "t_min", strMin)
            && getDatabaseValue(trackName, "t_max", strMax))
    {
        const DataPoint &dp0 = m_data[0];
        mZoomLevel.rangeLower = dp0.t + dp0.dateTime.msecsTo(
                    QDateTime::fromString(strMin, Qt::ISODate)) / 1000.;
        mZoomLevel.rangeUpper = dp0.t + dp0.dateTime.msecsTo(
                    QDateTime::fromString(strMax, Qt::ISODate)) / 1000.;
    }
    else if (!m_data.isEmpty())
    {
        double lower, upper;
        for (int i = 0; i < m_data.size(); ++i)
        {
            const DataPoint &dp = m_data[i];

            if (i == 0)
            {
                lower = upper = dp.t;
            }
            else
            {
                if (dp.t < lower) lower = dp.t;
                if (dp.t > upper) upper = dp.t;
            }
        }

        mZoomLevel.rangeLower = qMin(lower, upper);
        mZoomLevel.rangeUpper = qMax(lower, upper);
    }
    else
    {
        mZoomLevel.rangeLower = mZoomLevel.rangeUpper = 0;
    }

    emit dataChanged();

    // Disable undo/redo zoom controls
    m_ui->actionUndoZoom->setEnabled(false);
    m_ui->actionRedoZoom->setEnabled(false);

    // Enable zoom to extent
    m_ui->actionZoomToExtent->setEnabled(!m_data.isEmpty());
}

void MainWindow::on_actionElevation_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::Elevation);
}

void MainWindow::on_actionVerticalSpeed_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::VerticalSpeed);
}

void MainWindow::on_actionHorizontalSpeed_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::HorizontalSpeed);
}

void MainWindow::on_actionTime_triggered()
{
    m_ui->plotArea->setXAxisType(DataPlot::Time);
    updateBottomActions();
}

void MainWindow::on_actionDistance2D_triggered()
{
    m_ui->plotArea->setXAxisType(DataPlot::Distance2D);
    updateBottomActions();
}

void MainWindow::on_actionDistance3D_triggered()
{
    m_ui->plotArea->setXAxisType(DataPlot::Distance3D);
    updateBottomActions();
}

void MainWindow::updateBottomActions()
{
    m_ui->actionTime->setChecked(m_ui->plotArea->xAxisType() == DataPlot::Time);
    m_ui->actionDistance2D->setChecked(m_ui->plotArea->xAxisType() == DataPlot::Distance2D);
    m_ui->actionDistance3D->setChecked(m_ui->plotArea->xAxisType() == DataPlot::Distance3D);
}

void MainWindow::updateLeftActions()
{
    m_ui->actionElevation->setChecked(m_ui->plotArea->plotVisible(DataPlot::Elevation));
    m_ui->actionVerticalSpeed->setChecked(m_ui->plotArea->plotVisible(DataPlot::VerticalSpeed));
    m_ui->actionHorizontalSpeed->setChecked(m_ui->plotArea->plotVisible(DataPlot::HorizontalSpeed));
    m_ui->actionTotalSpeed->setChecked(m_ui->plotArea->plotVisible(DataPlot::TotalSpeed));
    m_ui->actionDiveAngle->setChecked(m_ui->plotArea->plotVisible(DataPlot::DiveAngle));
    m_ui->actionCurvature->setChecked(m_ui->plotArea->plotVisible(DataPlot::Curvature));
    m_ui->actionGlideRatio->setChecked(m_ui->plotArea->plotVisible(DataPlot::GlideRatio));
    m_ui->actionHorizontalAccuracy->setChecked(m_ui->plotArea->plotVisible(DataPlot::HorizontalAccuracy));
    m_ui->actionVerticalAccuracy->setChecked(m_ui->plotArea->plotVisible(DataPlot::VerticalAccuracy));
    m_ui->actionSpeedAccuracy->setChecked(m_ui->plotArea->plotVisible(DataPlot::SpeedAccuracy));
    m_ui->actionNumberOfSatellites->setChecked(m_ui->plotArea->plotVisible(DataPlot::NumberOfSatellites));
    m_ui->actionAcceleration->setChecked(m_ui->plotArea->plotVisible(DataPlot::Acceleration));
    m_ui->actionTotalEnergy->setChecked(m_ui->plotArea->plotVisible(DataPlot::TotalEnergy));
    m_ui->actionEnergyRate->setChecked(m_ui->plotArea->plotVisible(DataPlot::EnergyRate));
    m_ui->actionLift->setChecked(m_ui->plotArea->plotVisible(DataPlot::Lift));
    m_ui->actionDrag->setChecked(m_ui->plotArea->plotVisible(DataPlot::Drag));
    m_ui->actionCourse->setChecked(m_ui->plotArea->plotVisible(DataPlot::Course));
    m_ui->actionCourseRate->setChecked(m_ui->plotArea->plotVisible(DataPlot::CourseRate));
    m_ui->actionCourseAccuracy->setChecked(m_ui->plotArea->plotVisible(DataPlot::CourseAccuracy));
}

void MainWindow::on_actionTotalSpeed_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::TotalSpeed);
}

void MainWindow::on_actionDiveAngle_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::DiveAngle);
}

void MainWindow::on_actionCurvature_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::Curvature);
}

void MainWindow::on_actionGlideRatio_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::GlideRatio);
}

void MainWindow::on_actionHorizontalAccuracy_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::HorizontalAccuracy);
}

void MainWindow::on_actionVerticalAccuracy_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::VerticalAccuracy);
}

void MainWindow::on_actionSpeedAccuracy_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::SpeedAccuracy);
}

void MainWindow::on_actionNumberOfSatellites_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::NumberOfSatellites);
}

void MainWindow::on_actionAcceleration_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::Acceleration);
}

void MainWindow::on_actionTotalEnergy_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::TotalEnergy);
}

void MainWindow::on_actionEnergyRate_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::EnergyRate);
}

void MainWindow::on_actionLift_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::Lift);
}

void MainWindow::on_actionDrag_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::Drag);
}

void MainWindow::on_actionCourse_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::Course);
}

void MainWindow::on_actionCourseRate_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::CourseRate);
}

void MainWindow::on_actionCourseAccuracy_triggered()
{
    m_ui->plotArea->togglePlot(DataPlot::CourseAccuracy);
}

void MainWindow::on_actionPan_triggered()
{
    setTool(Pan);
}

void MainWindow::on_actionZoom_triggered()
{
    setTool(Zoom);
}

void MainWindow::on_actionMeasure_triggered()
{
    setTool(Measure);
}

void MainWindow::on_actionZero_triggered()
{
    setTool(Zero);
}

void MainWindow::on_actionGround_triggered()
{
    setTool(Ground);
}

void MainWindow::on_actionSetCourse_triggered()
{
    setTool(Course);
}

void MainWindow::on_actionWind_triggered()
{
    mWindAdjustment = !mWindAdjustment;
    m_ui->actionWind->setChecked(mWindAdjustment);

    // Update plot data
    updateVelocity(m_data, mTrackName, false);

    // Update checked tracks
    QMap< QString, DataPoints >::iterator p;
    for (p = mCheckedTracks.begin();
         p != mCheckedTracks.end();
         ++p)
    {
        updateVelocity(p.value(), p.key(), false);
    }

    emit dataChanged();
}

void MainWindow::on_actionImportGates_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Import Gates"), "", tr("CSV Files (*.csv)"));

    for (int i = 0; i < fileNames.size(); ++i)
    {
        QFile file(fileNames[i]);
        if (!file.open(QIODevice::ReadOnly))
        {
            // TODO: Error message
            continue;
        }

        QTextStream in(&file);

        // skip first 2 rows
        if (!in.atEnd()) in.readLine();
        if (!in.atEnd()) in.readLine();

        QVector< double > lat, lon, hMSL;

        while (!in.atEnd())
        {
            QString line = in.readLine();
            QStringList cols = line.split(",");

            lat.append(cols[1].toDouble());
            lon.append(cols[2].toDouble());
            hMSL.append(cols[3].toDouble());
        }

        if (lat.size() > 0)
        {
            qSort(lat.begin(), lat.end());
            qSort(lon.begin(), lon.end());
            qSort(hMSL.begin(), hMSL.end());

            int mid = lat.size() / 2;

            DataPoint dp ;

            dp.lat  = lat[mid];
            dp.lon  = lon[mid];
            dp.hMSL = hMSL[mid];

            m_waypoints.append(dp);
        }
    }

    emit dataChanged();
}

void MainWindow::on_actionPreferences_triggered()
{
    ConfigDialog dlg(this);

    dlg.setUnits(m_units);
    dlg.setMass(m_mass);
    dlg.setPlanformArea(m_planformArea);
    dlg.setMinDrag(m_minDrag);
    dlg.setMinLift(m_minLift);
    dlg.setMaxLift(m_maxLift);
    dlg.setMaxLD(m_maxLD);
    dlg.setSimulationTime(m_simulationTime);
    dlg.setLineThickness(mLineThickness);

    const double factor = (m_units == PlotValue::Metric) ? MPS_TO_KMH : MPS_TO_MPH;
    const QString unitText = (m_units == PlotValue::Metric) ? "km/h" : "mph";

    double windSpeed = sqrt(mWindE * mWindE + mWindN * mWindN);
    double windDirection = atan2(-mWindE, -mWindN) / PI * 180;
    if (windDirection < 0) windDirection += 360;
    windSpeed *= factor;

    dlg.setWindSpeed(windSpeed);
    dlg.setWindUnits(unitText);
    dlg.setWindDirection(windDirection);

    dlg.setGroundReference(mGroundReference);
    dlg.setFixedReference(mFixedReference);

    dlg.setDatabasePath(mDatabasePath);

    if (dlg.exec() == QDialog::Accepted)
    {
        if (m_units != dlg.units())
        {
            m_units = dlg.units();

            emit dataChanged();
        }

        if (m_mass != dlg.mass() ||
            m_planformArea != dlg.planformArea())
        {
            m_mass = dlg.mass();
            m_planformArea = dlg.planformArea();

            // Update plot data
            initAerodynamics(m_data);

            // Update checked tracks
            foreach (DataPoints data, mCheckedTracks)
            {
                initAerodynamics(data);
            }

            emit dataChanged();
        }

        if (m_minDrag != dlg.minDrag() ||
            m_minLift != dlg.minLift() ||
            m_maxLift != dlg.maxLift() ||
            m_maxLD != dlg.maxLD())
        {
            m_minDrag = dlg.minDrag();
            m_minLift = dlg.minLift();
            m_maxLift = dlg.maxLift();
            m_maxLD = dlg.maxLD();

            emit dataChanged();
        }

        m_simulationTime = dlg.simulationTime();

        bool plotChanged = false;
        for (int i = 0; i < plotArea()->yaLast; ++i)
        {
            PlotValue *yValue = plotArea()->yValue(i);

            if (yValue->color() != dlg.plotColor(i))
            {
                yValue->setColor(dlg.plotColor(i));
                plotChanged = true;
            }

            if (yValue->minimum() != dlg.plotMinimum(i))
            {
                yValue->setMinimum(dlg.plotMinimum(i) / yValue->factor(units()));
                plotChanged = true;
            }

            if (yValue->maximum() != dlg.plotMaximum(i))
            {
                yValue->setMaximum(dlg.plotMaximum(i) / yValue->factor(units()));
                plotChanged = true;
            }

            if (yValue->useMinimum() != dlg.plotUseMinimum(i))
            {
                yValue->setUseMinimum(dlg.plotUseMinimum(i));
                plotChanged = true;
            }

            if (yValue->useMaximum() != dlg.plotUseMaximum(i))
            {
                yValue->setUseMaximum(dlg.plotUseMaximum(i));
                plotChanged = true;
            }
        }

        if (mLineThickness != dlg.lineThickness())
        {
            mLineThickness = dlg.lineThickness();
            plotChanged = true;
        }

        if (plotChanged)
        {
            emit dataChanged();
        }

        if (mWindE != -dlg.windSpeed() * sin(dlg.windDirection() / 180 * PI) / factor ||
            mWindN != -dlg.windSpeed() * cos(dlg.windDirection() / 180 * PI) / factor)
        {
            mWindE = -dlg.windSpeed() * sin(dlg.windDirection() / 180 * PI) / factor;
            mWindN = -dlg.windSpeed() * cos(dlg.windDirection() / 180 * PI) / factor;

            emit dataChanged();
        }

        if (mGroundReference != dlg.groundReference() ||
            mFixedReference != dlg.fixedReference())
        {
            mGroundReference = dlg.groundReference();
            mFixedReference = dlg.fixedReference();
        }

        if (mDatabasePath != dlg.databasePath())
        {
            // Change the database path
            mDatabasePath = dlg.databasePath();

            // Open/create database
            initDatabase();

            // Update views
            emit databaseChanged();
        }
    }
}

void MainWindow::on_actionImportVideo_triggered()
{
    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get last file read
    QString rootFolder = settings.value("videoFolder").toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Video"), rootFolder);

    if (!fileName.isEmpty())
    {
        // Remember last file read
        settings.setValue("videoFolder", QFileInfo(fileName).absoluteFilePath());

        // Create video view
        VideoView *videoView = new VideoView;
        QDockWidget *dockWidget = new QDockWidget(tr("Video View"));
        dockWidget->setWidget(videoView);
        dockWidget->setObjectName("videoView");
        addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

        // Default to floating and delete when closed
        dockWidget->setFloating(true);
        dockWidget->setAttribute(Qt::WA_DeleteOnClose);

        // Associate the view with the main window
        videoView->setMainWindow(this);

        // Set up notifications for video view
        connect(this, SIGNAL(dataChanged()),
                videoView, SLOT(updateView()));
        connect(this, SIGNAL(cursorChanged()),
                videoView, SLOT(updateView()));

        // Associate view with this file
        videoView->setMedia(fileName);
    }
}

void MainWindow::on_actionExportKML_triggered()
{
    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get last file read
    QString rootFolder = settings.value("kmlFolder").toString();

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export KML"),
                                                    rootFolder,
                                                    tr("KML Files (*.kml)"));

    if (!fileName.isEmpty())
    {
        // Remember last file read
        settings.setValue("kmlFolder", QFileInfo(fileName).absoluteFilePath());

        // Open output file
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            // TODO: Error message
            return;
        }

        QTextStream stream(&file);

        // Write headers
        stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
        stream << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << endl;
        stream << "  <Placemark>" << endl;
        stream << "    <name>" << QFileInfo(fileName).baseName() << "</name>" << endl;
        stream << "    <LineString>" << endl;
        stream << "      <altitudeMode>absolute</altitudeMode>" << endl;
        stream << "      <coordinates>" << endl;

        double lower = rangeLower();
        double upper = rangeUpper();

        bool first = true;
        for (int i = 0; i < dataSize(); ++i)
        {
            const DataPoint &dp = dataPoint(i);

            if (lower <= dp.t && dp.t <= upper)
            {
                if (first)
                {
                    stream << "        ";
                    first = false;
                }
                else
                {
                    stream << " ";
                }

                stream << QString("%1,%2,%3").arg(dp.lon, 0, 'f', 7).arg(dp.lat, 0, 'f', 7).arg(dp.hMSL, 0, 'f', 3);
            }
        }

        if (!first)
        {
            stream << endl;
        }


        // Write footers
        stream << "      </coordinates>" << endl;
        stream << "    </LineString>" << endl;
        stream << "  </Placemark>" << endl;
        stream << "</kml>" << endl;
    }
}

void MainWindow::on_actionExportPlot_triggered()
{
    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get last file read
    QString rootFolder = settings.value("plotFolder").toString();

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export Plot"),
                                                    rootFolder,
                                                    tr("CSV Files (*.csv)"));

    if (!fileName.isEmpty())
    {
        // Remember last file read
        settings.setValue("plotFolder", QFileInfo(fileName).absoluteFilePath());

        // Open output file
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            // TODO: Error message
            return;
        }

        QTextStream stream(&file);

        // Write header
        stream << m_ui->plotArea->xValue()->title(m_units);
        for (int j = 0; j < DataPlot::yaLast; ++j)
        {
            if (!m_ui->plotArea->yValue(j)->visible()) continue;
            stream << "," << m_ui->plotArea->yValue(j)->title(m_units);
        }
        stream << endl;

        double lower = rangeLower();
        double upper = rangeUpper();

        for (int i = 0; i < dataSize(); ++i)
        {
            const DataPoint &dp = dataPoint(i);

            if (lower <= dp.t && dp.t <= upper)
            {
                stream << m_ui->plotArea->xValue()->value(dp, m_units);
                for (int j = 0; j < DataPlot::yaLast; ++j)
                {
                    if (!m_ui->plotArea->yValue(j)->visible()) continue;
                    stream << QString(",%1").arg(m_ui->plotArea->yValue(j)->value(dp, m_units), 0, 'f');
                }
                stream << endl;
            }
        }
    }
}

void MainWindow::on_actionExportTrack_triggered()
{
    // Initialize settings object
    QSettings settings("FlySight", "Viewer");

    // Get last file read
    QString rootFolder = settings.value("trackFolder").toString();

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export Track"),
                                                    rootFolder,
                                                    tr("CSV Files (*.csv)"));

    if (!fileName.isEmpty())
    {
        // Remember last file read
        settings.setValue("trackFolder", QFileInfo(fileName).absoluteFilePath());

        // Open output file
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            // TODO: Error message
            return;
        }

        QTextStream stream(&file);

        // Write header
        stream << "time,lat,lon,hMSL,velN,velE,velD,hAcc,vAcc,sAcc,heading,cAcc,gpsFix,numSV" << endl;
        stream << ",(deg),(deg),(m),(m/s),(m/s),(m/s),(m),(m),(m/s),(deg),(deg),," << endl;

        double lower = rangeLower();
        double upper = rangeUpper();

        for (int i = 0; i < dataSize(); ++i)
        {
            const DataPoint &dp = dataPoint(i);

            if (lower <= dp.t && dp.t <= upper)
            {
                stream << dateTimeToUTC(dp.dateTime) << ",";

                stream << QString::number(dp.lat, 'f', 7) << ",";
                stream << QString::number(dp.lon, 'f', 7) << ",";
                stream << QString::number(dp.hMSL, 'f', 3) << ",";

                stream << QString::number(dp.velN, 'f', 2) << ",";
                stream << QString::number(dp.velE, 'f', 2) << ",";
                stream << QString::number(dp.velD, 'f', 2) << ",";

                stream << QString::number(dp.hAcc, 'f', 3) << ",";
                stream << QString::number(dp.vAcc, 'f', 3) << ",";
                stream << QString::number(dp.sAcc, 'f', 2) << ",";

                // Get adjusted heading
                double heading = dp.heading;
                while (heading <  0)   heading += 360;
                while (heading >= 360) heading -= 360;

                stream << QString::number(heading, 'f', 5) << ",";
                stream << QString::number(dp.cAcc, 'f', 5) << ",";

                stream << ",";  // gpsFix

                stream << QString::number(dp.numSV) << endl;
            }
        }
    }
}

QString MainWindow::dateTimeToUTC(
        const QDateTime &dt)
{
    QString ret;
    ret += dt.toUTC().date().toString(Qt::ISODate) + "T";
    ret += dt.toUTC().time().toString(Qt::ISODate) + ".";
    ret += QString("%1").arg(dt.toUTC().time().msec(), 3, 10, QChar('0')) + "Z";
    return ret;
}

void MainWindow::setRange(
        double lower,
        double upper,
        bool immediate)
{
    if (!zoomTimer->isActive())
    {
        mZoomLevelPrev = mZoomLevel;
    }

    if (immediate)
    {
        zoomTimer->start(0);
    }
    else
    {
        zoomTimer->start(1000);
    }

    mZoomLevel.rangeLower = qMin(lower, upper);
    mZoomLevel.rangeUpper = qMax(lower, upper);

    emit dataChanged();
}

void MainWindow::saveZoom()
{
    // Save zoom level to undo
    mZoomLevelUndo.push(mZoomLevelPrev);
    mZoomLevelRedo.clear();

    // Enable controls
    m_ui->actionUndoZoom->setEnabled(!mZoomLevelUndo.empty());
    m_ui->actionRedoZoom->setEnabled(!mZoomLevelRedo.empty());

    // Save zoom level to database
    DataPoint dp = interpolateDataT(mZoomLevel.rangeLower);
    setDatabaseValue(mTrackName, "t_min", dateTimeToUTC(dp.dateTime));
    dp = interpolateDataT(mZoomLevel.rangeUpper);
    setDatabaseValue(mTrackName, "t_max", dateTimeToUTC(dp.dateTime));

    emit databaseChanged();
}

void MainWindow::setRotation(
        double rotation)
{
    m_viewDataRotation = rotation;
    emit rotationChanged(m_viewDataRotation);
}

void MainWindow::setZero(
        double t)
{
    if (m_data.isEmpty()) return;

    DataPoint dp0 = interpolateDataT(t);
    setDatabaseValue(mTrackName, "exit", dateTimeToUTC(dp0.dateTime));

    for (int i = 0; i < m_data.size(); ++i)
    {
        DataPoint &dp = m_data[i];

        dp.t -= dp0.t;
        dp.x -= dp0.x;
        dp.y -= dp0.y;

        dp.dist2D -= dp0.dist2D;
        dp.dist3D -= dp0.dist3D;
    }

    mMarkStart -= dp0.t;
    mMarkEnd -= dp0.t;

    QVector< ZoomLevel >::iterator p;
    for (p = mZoomLevelUndo.begin(); p != mZoomLevelUndo.end(); ++p)
    {
        p->rangeLower -= dp0.t;
        p->rangeUpper -= dp0.t;
    }
    for (p = mZoomLevelRedo.begin(); p != mZoomLevelRedo.end(); ++p)
    {
        p->rangeLower -= dp0.t;
        p->rangeUpper -= dp0.t;
    }

    mZoomLevel.rangeLower -= dp0.t;
    mZoomLevel.rangeUpper -= dp0.t;

    emit dataChanged();

    setTool(mPrevTool);
}

void MainWindow::setGround(
        double t)
{
    if (m_data.isEmpty()) return;

    DataPoint dp0 = interpolateDataT(t);
    setTrackGround(mTrackName, dp0.hMSL);

    setTool(mPrevTool);
}

void MainWindow::setTrackGround(
        QString trackName,
        double ground)
{
    setDatabaseValue(trackName, "ground", QString::number(ground, 'f', 3));

    // Update current track
    if (trackName == mTrackName)
    {
        updateGround(m_data, ground);
        emit dataChanged();
    }

    // Update checked tracks
    QMap< QString, DataPoints >::iterator p;
    for (p = mCheckedTracks.begin();
         p != mCheckedTracks.end();
         ++p)
    {
        if (trackName == p.key())
        {
            updateGround(p.value(), ground);
        }
    }
}

void MainWindow::setTrackWindSpeed(
        QString trackName,
        double windSpeed)
{
    double windSpeedOld, windDir;
    getWindSpeedDirection(trackName, &windSpeedOld, &windDir);

    double windE, windN;
    windE = -windSpeed * sin(windDir / 180 * PI);
    windN = -windSpeed * cos(windDir / 180 * PI);

    setDatabaseValue(trackName, "wind_e", QString::number(windE, 'f', 2));
    setDatabaseValue(trackName, "wind_n", QString::number(windN, 'f', 2));

    // Update current track
    if (trackName == mTrackName)
    {
        updateVelocity(m_data, mTrackName, false);
        emit dataChanged();
    }

    // Update checked tracks
    QMap< QString, DataPoints >::iterator p;
    for (p = mCheckedTracks.begin();
         p != mCheckedTracks.end();
         ++p)
    {
        if (trackName == p.key())
        {
            updateVelocity(p.value(), p.key(), false);
        }
    }
}

void MainWindow::setTrackWindDir(
        QString trackName,
        double windDir)
{
    double windSpeed, windDirOld;
    getWindSpeedDirection(trackName, &windSpeed, &windDirOld);

    double windE, windN;
    windE = -windSpeed * sin(windDir / 180 * PI);
    windN = -windSpeed * cos(windDir / 180 * PI);

    setDatabaseValue(trackName, "wind_e", QString::number(windE, 'f', 2));
    setDatabaseValue(trackName, "wind_n", QString::number(windN, 'f', 2));

    // Update current track
    if (trackName == mTrackName)
    {
        updateVelocity(m_data, mTrackName, false);
        emit dataChanged();
    }

    // Update checked tracks
    QMap< QString, DataPoints >::iterator p;
    for (p = mCheckedTracks.begin();
         p != mCheckedTracks.end();
         ++p)
    {
        if (trackName == p.key())
        {
            updateVelocity(p.value(), p.key(), false);
        }
    }
}

void MainWindow::updateGround(
        DataPoints &data,
        double ground)
{
    for (int i = 0; i < data.size(); ++i)
    {
        DataPoint &dp = data[i];
        dp.z = dp.hMSL - ground;
    }
}

void MainWindow::setCourse(
        double t)
{
    if (m_data.isEmpty()) return;

    DataPoint dp0 = interpolateDataT(t);
    setDatabaseValue(mTrackName, "course", QString::number(dp0.heading, 'f', 5));

    for (int i = 0; i < m_data.size(); ++i)
    {
        DataPoint &dp = m_data[i];
        dp.theta -= dp0.theta;
    }

    emit dataChanged();

    setTool(mPrevTool);
}

bool MainWindow::setDatabaseValue(
        QString trackName,
        QString column,
        QString value)
{
    QSqlQuery query(mDatabase);

    // Check the old value
    if (!query.exec(QString("select * from files where (file_name='%1' and %2='%3')")
                    .arg(trackName).arg(column).arg(value)))
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(0, tr("Query failed"), err.text());
        return false;
    }

    // Return now if value is not changed
    if (query.next()) return true;

    // Change the value
    if (!query.exec(QString("update files set %1='%2' where file_name='%3'")
                    .arg(column).arg(value).arg(trackName)))
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(0, tr("Query failed"), err.text());
        return false;
    }

    emit databaseChanged();
    return true;
}

bool MainWindow::getDatabaseValue(
        QString trackName,
        QString column,
        QString &value)
{
    QSqlQuery query(mDatabase);

    // Read value from database
    if (!query.exec(QString("select %1 from files where file_name='%2'")
                    .arg(column).arg(trackName)))
    {
        QSqlError err = query.lastError();
        QMessageBox::critical(0, tr("Query failed"), err.text());
        return false;
    }

    // Check if there is a result
    if (!query.next()) return false;

    // Handle empty results
    if (query.value(0).toString().isEmpty()) return false;

    // Return the result
    value = query.value(0).toString();
    return true;
}

void MainWindow::setScoringVisible(
        bool visible)
{
    emit dataChanged();
}

void MainWindow::setMinDrag(
        double minDrag)
{
    m_minDrag = minDrag;

    emit aeroChanged();
}

void MainWindow::setMaxLift(
        double maxLift)
{
    m_maxLift = maxLift;

    emit aeroChanged();
}

void MainWindow::setMaxLD(
        double maxLD)
{
    m_maxLD = maxLD;

    emit aeroChanged();
}

void MainWindow::setWindowMode(
        WindowMode mode)
{
    mWindowMode = mode;

    emit dataChanged();
}

void MainWindow::setTool(
        Tool tool)
{
    if (tool != Course && tool != Ground && tool != Zero)
    {
        mPrevTool = tool;
    }
    mTool = tool;

    m_ui->actionPan->setChecked(tool == Pan);
    m_ui->actionZoom->setChecked(tool == Zoom);
    m_ui->actionMeasure->setChecked(tool == Measure);
    m_ui->actionZero->setChecked(tool == Zero);
    m_ui->actionGround->setChecked(tool == Ground);
    m_ui->actionSetCourse->setChecked(tool == Course);
}

DataPlot *MainWindow::plotArea() const
{
    return m_ui->plotArea;
}

void MainWindow::setLineThickness(
        double width)
{
    mLineThickness = width;
    emit dataChanged();
}

void MainWindow::setWind(
        double windE,
        double windN)
{
    setDatabaseValue(mTrackName, "wind_e", QString::number(windE, 'f', 2));
    setDatabaseValue(mTrackName, "wind_n", QString::number(windN, 'f', 2));

    // Update plot data
    updateVelocity(m_data, mTrackName, false);

    // Update checked tracks
    QMap< QString, DataPoints >::iterator p;
    for (p = mCheckedTracks.begin();
         p != mCheckedTracks.end();
         ++p)
    {
        updateVelocity(p.value(), p.key(), false);
    }

    emit dataChanged();
}

void MainWindow::getWind(
        QString trackName,
        double *windE,
        double *windN)
{
    QString strE, strN;
    if (getDatabaseValue(trackName, "wind_e", strE)
            && getDatabaseValue(trackName, "wind_n", strN))
    {
        *windE = strE.toDouble();
        *windN = strN.toDouble();
    }
    else
    {
        *windE = mWindE;
        *windN = mWindN;
    }
}

void MainWindow::getWindSpeedDirection(
        QString trackName,
        double *windSpeed,
        double *windDirection)
{
    double windE, windN;
    getWind(trackName, &windE, &windN);

    *windSpeed = sqrt(windE * windE + windN * windN);
    *windDirection = atan2(-windE, -windN) / PI * 180;
    if (*windDirection < 0) *windDirection += 360;
}

void MainWindow::on_actionUndoZoom_triggered()
{
    mZoomLevelRedo.push(mZoomLevel);
    mZoomLevel = mZoomLevelUndo.pop();

    emit dataChanged();

    // Enable controls
    m_ui->actionUndoZoom->setEnabled(!mZoomLevelUndo.empty());
    m_ui->actionRedoZoom->setEnabled(!mZoomLevelRedo.empty());
}

void MainWindow::on_actionRedoZoom_triggered()
{
    mZoomLevelUndo.push(mZoomLevel);
    mZoomLevel = mZoomLevelRedo.pop();

    emit dataChanged();

    // Enable controls
    m_ui->actionUndoZoom->setEnabled(!mZoomLevelUndo.empty());
    m_ui->actionRedoZoom->setEnabled(!mZoomLevelRedo.empty());
}

void MainWindow::on_actionZoomToExtent_triggered()
{
    double lower, upper;
    for (int i = 0; i < m_data.size(); ++i)
    {
        const DataPoint &dp = m_data[i];

        if (i == 0)
        {
            lower = upper = dp.t;
        }
        else
        {
            if (dp.t < lower) lower = dp.t;
            if (dp.t > upper) upper = dp.t;
        }
    }

    setRange(lower, upper, true);
}

void MainWindow::on_actionDeleteTrack_triggered()
{
    QString message = tr("Are you sure you want to delete the selected tracks?\n");
    if (QMessageBox::question(0, tr("FlySight"), message) != QMessageBox::Yes) return;

    foreach (QString uniqueName, mSelectedTracks)
    {
        if (uniqueName == mTrackName)
        {
            // Clear track data
            m_data.clear();
            emit dataChanged();

            // Clear current track name;
            mTrackName = QString();
        }

        // Get name of file in database
        QString newName = QString("FlySight/Tracks/%1.csv").arg(uniqueName);
        QString newPath = QDir(mDatabasePath).filePath(newName);

        // Delete the track
        if (!QFile::remove(newPath))
        {
            QMessageBox::critical(0, tr("Operation failed"), tr("Couldn't delete track"));
        }

        // Remove track from database
        QSqlQuery query(mDatabase);
        if (!query.exec(QString("delete from files where file_name='%1'").arg(uniqueName)))
        {
            QSqlError err = query.lastError();
            QMessageBox::critical(0, tr("Query failed"), err.text());
        }
    }

    emit databaseChanged();
}

void MainWindow::setScoringMode(
        ScoringMode mode)
{
    mScoringMode = mode;
    emit dataChanged();
}

void MainWindow::prepareDataPlot(
        DataPlot *plot)
{
    if (mScoringView->isVisible())
    {
        mScoringMethods[mScoringMode]->prepareDataPlot(plot);
    }
}

void MainWindow::prepareMapView(
        MapView *view)
{
    if (mScoringView->isVisible())
    {
        mScoringMethods[mScoringMode]->prepareMapView(view);
    }
}

bool MainWindow::updateReference(
        double lat,
        double lon)
{
    if (mScoringView->isVisible())
    {
        return mScoringMethods[mScoringMode]->updateReference(lat, lon);
    }
    else
    {
        return false;
    }
}

void MainWindow::closeReference()
{
    if (mScoringView->isVisible())
    {
        mScoringMethods[mScoringMode]->closeReference();
    }
}

void MainWindow::setOptimal(
        const DataPoints &result)
{
    m_optimal = result;
    emit dataChanged();
}
