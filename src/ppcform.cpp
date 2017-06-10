#include "ppcform.h"
#include "ui_ppcform.h"

#include <iostream>

#include "common.h"
#include "dataplot.h"
#include "datapoint.h"
#include "mainwindow.h"
#include "plotvalue.h"
#include "ppcscoring.h"

#include "ui_getuserdialog.h"

PPCForm::PPCForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PPCForm),
    mMainWindow(0)
{
    ui->setupUi(this);

    connect(ui->faiButton, SIGNAL(clicked()), this, SLOT(onFAIButtonClicked()));
    connect(ui->upButton, SIGNAL(clicked()), this, SLOT(onUpButtonClicked()));
    connect(ui->downButton, SIGNAL(clicked()), this, SLOT(onDownButtonClicked()));

    connect(ui->topEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));
    connect(ui->bottomEdit, SIGNAL(editingFinished()), this, SLOT(onApplyButtonClicked()));

    connect(ui->timeButton, SIGNAL(clicked()), this, SLOT(onModeChanged()));
    connect(ui->distanceButton, SIGNAL(clicked()), this, SLOT(onModeChanged()));
    connect(ui->hSpeedButton, SIGNAL(clicked()), this, SLOT(onModeChanged()));

    // Connect optimization buttons
    connect(ui->actualButton, SIGNAL(clicked()), this, SLOT(onActualButtonClicked()));
    connect(ui->optimalButton, SIGNAL(clicked()), this, SLOT(onOptimalButtonClicked()));
    connect(ui->optimizeButton, SIGNAL(clicked()), this, SLOT(onOptimizeButtonClicked()));

    // Connect PPC button
    connect(ui->ppcButton, SIGNAL(clicked()), this, SLOT(onPpcButtonClicked()));
}

PPCForm::~PPCForm()
{
    delete ui;
}

QSize PPCForm::sizeHint() const
{
    // Keeps windows from being intialized as very short
    return QSize(175, 175);
}

void PPCForm::setMainWindow(
        MainWindow *mainWindow)
{
    mMainWindow = mainWindow;
}

void PPCForm::updateView()
{
    // Update mode selection
    ui->actualButton->setChecked(mMainWindow->windowMode() == MainWindow::Actual);
    ui->optimalButton->setChecked(mMainWindow->windowMode() == MainWindow::Optimal);

    PPCScoring *method = (PPCScoring *) mMainWindow->scoringMethod(MainWindow::PPC);

    const double bottom = method->windowBottom();
    const double top = method->windowTop();

    // Update window bounds
    ui->bottomEdit->setText(QString("%1").arg(bottom));
    ui->topEdit->setText(QString("%1").arg(top));

    DataPoint dpBottom, dpTop;
    bool success;

    switch (mMainWindow->windowMode())
    {
    case MainWindow::Actual:
        success = method->getWindowBounds(mMainWindow->data(), dpBottom, dpTop);
        break;
    case MainWindow::Optimal:
        success = method->getWindowBounds(mMainWindow->optimal(), dpBottom, dpTop);
        break;
    }

    if (success)
    {
        // Calculate results
        const double time = dpBottom.t - dpTop.t;
        const double distance = MainWindow::getDistance(dpTop, dpBottom);
        const double horizontalSpeed = distance / time;

        // Update display
        if (mMainWindow->units() == PlotValue::Metric)
        {
            ui->timeEdit->setText(QString("%1").arg(time));
            ui->distanceEdit->setText(QString("%1").arg(distance / 1000));
            ui->distanceUnits->setText(tr("km"));
            ui->horizontalSpeedEdit->setText(QString("%1").arg(horizontalSpeed * MPS_TO_KMH));
            ui->horizontalSpeedUnits->setText(tr("km/h"));
        }
        else
        {
            ui->timeEdit->setText(QString("%1").arg(time));
            ui->distanceEdit->setText(QString("%1").arg(distance * METERS_TO_FEET / 5280));
            ui->distanceUnits->setText(tr("mi"));
            ui->horizontalSpeedEdit->setText(QString("%1").arg(horizontalSpeed * MPS_TO_MPH));
            ui->horizontalSpeedUnits->setText(tr("mph"));
        }
    }
    else
    {
        // Update display
        if (mMainWindow->units() == PlotValue::Metric)
        {
            ui->distanceUnits->setText(tr("km"));
            ui->horizontalSpeedUnits->setText(tr("km/h"));
        }
        else
        {
            ui->distanceUnits->setText(tr("mi"));
            ui->horizontalSpeedUnits->setText(tr("mph"));
        }

        ui->timeEdit->setText(tr("n/a"));
        ui->distanceEdit->setText(tr("n/a"));
        ui->horizontalSpeedEdit->setText(tr("n/a"));
    }
}

void PPCForm::onFAIButtonClicked()
{
    PPCScoring *method = (PPCScoring *) mMainWindow->scoringMethod(MainWindow::PPC);
    method->setWindow(2000, 3000);
}

void PPCForm::onApplyButtonClicked()
{
    double bottom = ui->bottomEdit->text().toDouble();
    double top = ui->topEdit->text().toDouble();

    PPCScoring *method = (PPCScoring *) mMainWindow->scoringMethod(MainWindow::PPC);
    method->setWindow(bottom, top);

    mMainWindow->setFocus();
}

void PPCForm::onUpButtonClicked()
{
    PPCScoring *method = (PPCScoring *) mMainWindow->scoringMethod(MainWindow::PPC);
    method->setWindow(method->windowBottom() + 10, method->windowTop() + 10);
}

void PPCForm::onDownButtonClicked()
{
    PPCScoring *method = (PPCScoring *) mMainWindow->scoringMethod(MainWindow::PPC);
    method->setWindow(method->windowBottom() - 10, method->windowTop() - 10);
}

void PPCForm::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        // Reset window bounds
        updateView();

        // Release focus
        mMainWindow->setFocus();
    }

    QWidget::keyPressEvent(event);
}

void PPCForm::onModeChanged()
{
    PPCScoring *method = (PPCScoring *) mMainWindow->scoringMethod(MainWindow::PPC);

    if (ui->timeButton->isChecked())          method->setMode(PPCScoring::Time);
    else if (ui->distanceButton->isChecked()) method->setMode(PPCScoring::Distance);
    else                                      method->setMode(PPCScoring::Speed);
}

void PPCForm::onActualButtonClicked()
{
    mMainWindow->setWindowMode(MainWindow::Actual);
}

void PPCForm::onOptimalButtonClicked()
{
    mMainWindow->setWindowMode(MainWindow::Optimal);
}

void PPCForm::onOptimizeButtonClicked()
{
    PPCScoring *method = (PPCScoring *) mMainWindow->scoringMethod(MainWindow::PPC);

    // Perform optimization
    method->optimize();

    // Switch to optimal view
    mMainWindow->setWindowMode(MainWindow::Optimal);
}

void PPCForm::onPpcButtonClicked() {

// HTTPS requires the OpenSSL binaries.
// On windows thess are libeay32.dll, libssl32.dll and ssleay32.dll available from https://slproweb.com/products/Win32OpenSSL.html

    // Return if plot empty
    if (mMainWindow->dataSize() == 0) return;

    PPCScoring *method = (PPCScoring *) mMainWindow->scoringMethod(MainWindow::PPC);
    DataPoint dpBottom, dpTop;

    ui->ppcButton->setEnabled(false);

    if (method->getWindowBounds(mMainWindow->data(), dpBottom, dpTop)) {

        int startOffset = mMainWindow->findIndexBelowT(-10.0);
        int endOffset = mMainWindow->findIndexForLanding();

        qDebug() << "QNE: " << mMainWindow->getQNE();
        qDebug() << "Start: " << startOffset << " - t: " << mMainWindow->dataPoint(startOffset).t << ", h: " << mMainWindow->dataPoint(startOffset).hMSL;
        qDebug() << "End: " << endOffset << " - t: " << mMainWindow->dataPoint(endOffset).t << ", h: " << mMainWindow->dataPoint(endOffset).hMSL;

        QJsonArray times, latitudes, longitudes, altitudes, vSpeeds, hSpeeds, distances;
        double t0 =  mMainWindow->dataPoint(startOffset).t;

        for (int i = startOffset; i < endOffset; i++) {
            DataPoint p = mMainWindow->dataPoint(i);

            if (p.t - t0 < 1.0)
                continue;

            times.append(p.t);
            latitudes.append(p.lat);
            longitudes.append(p.lon);
            altitudes.append(p.z);
            vSpeeds.append(p.velD);
            hSpeeds.append(sqrt(p.velE * p.velE + p.velN * p.velN));
            distances.append(p.dist2D);

            t0 = p.t;
        }

        QJsonObject profile;
        profile["distances"] = distances;
        profile["hSpeeds"] = hSpeeds;
        profile["vSpeeds"] = vSpeeds;
        profile["altitudes"] = altitudes;
        profile["longitudes"] = longitudes;
        profile["latitudes"] = latitudes;
        profile["times"] = times;

        qDebug() << QJsonDocument(profile).toJson(QJsonDocument::Compact);

        const double time = dpBottom.t - dpTop.t;
        const double distance = MainWindow::getDistance(dpTop, dpBottom);
        const double horizontalSpeed = distance / time;
        const double verticalSpeed = (method->windowTop() - method->windowBottom()) / time;
        double windSpeed, windDirection;
        mMainWindow->getWindSpeedDirection(&windSpeed, &windDirection);

        QString name, countrycode, wingsuit, place;
        if (!getUserDetails(&name, &countrycode, &wingsuit, &place)) {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText("Upload aborted");
            msgBox.exec();
            ui->ppcButton->setEnabled(true);
            return;
        }

        QString url = "https://ppc.paralog.net/uploadtrack-db.php";
        QString post = "Source=FSV&";
            post += "Name="+QUrl::toPercentEncoding(name)+"&";
            post += "CountryCode="+QUrl::toPercentEncoding(countrycode)+"&";
            post += "Place="+QUrl::toPercentEncoding(place)+"&";
            post += "QNE="+QString::number(mMainWindow->getQNE())+"&";
            post += "Equipment="+QUrl::toPercentEncoding(wingsuit)+"&";
            post += "Timestamp="+mMainWindow->dataPoint(mMainWindow->findIndexBelowT(0.0)).dateTime.toString(Qt::ISODate)+"&";
            post += "WindowBegin="+QString::number(method->windowTop())+"&";
            post += "WindowEnd="+QString::number(method->windowBottom())+"&";
            post += "WindDir="+QString::number(windDirection)+"&";
            post += "WindSpeed="+QString::number(windSpeed)+"&";
            post += "Time="+QString::number(time)+"&";
            post += "Distance="+QString::number(distance)+"&";
            post += "VerticalSpeed="+QString::number(verticalSpeed)+"&";
            post += "HorizontalSpeed="+QString::number(horizontalSpeed)+"&";
            post += "HorizontalSpeed="+QString::number(horizontalSpeed)+"&";
            post += "Profile="+QJsonDocument(profile).toJson(QJsonDocument::Compact);

        QNetworkRequest request= QNetworkRequest(QUrl(url));

        request.setRawHeader("User-Agent", "FlySight Viewer");
        request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
        request.setRawHeader("Charset", "utf8");

        QNetworkAccessManager *naManager = new QNetworkAccessManager(this);
        connect(naManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(finished(QNetworkReply*)));
        naManager->post(request, post.toAscii());

        qDebug() << url;
        qDebug() << post;
    }
}

void PPCForm::finished(QNetworkReply *reply) {
    QMessageBox msgBox;
    if(reply->error() == QNetworkReply::NoError) {
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Success!");
        QDesktopServices::openUrl(QUrl("https://ppc.paralog.net/showTrack.php&?track="+QObject::tr(reply->readAll())));
    } else {
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(reply->errorString());
    }
    msgBox.exec();
    ui->ppcButton->setEnabled(true);
}


bool PPCForm::getUserDetails(QString *name, QString *countrycode, QString *wingsuit, QString *place) {
    QDialog userDetailsDialog;
    Ui::Dialog ui;
    ui.setupUi(&userDetailsDialog);
    userDetailsDialog.adjustSize();

//    ui.nameEdit->setText("Klaus Rheinwald");
//    ui.countrycodeEdit->setText("GER");
//    ui.wingsuitEdit->setText("Stealth");
//    ui.placeEdit->setText("Kiliti");

    if (userDetailsDialog.exec() == QDialog::Accepted) {
        name->append(ui.nameEdit->text());
        countrycode->append(ui.countrycodeEdit->text());
        wingsuit->append(ui.wingsuitEdit->text());
        place->append(ui.placeEdit->text());
        return true;
    }
    return false;
}
