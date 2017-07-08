#include "ppcupload.h"
#include "ui_getuserdialog.h"

PPCUpload::PPCUpload(MainWindow *mainWindow, QObject *parent) :
    QObject(parent)
{
    mMainWindow = mainWindow;
}

// HTTPS requires the OpenSSL binaries.
// On windows thess are libeay32.dll, libssl32.dll and ssleay32.dll available from https://slproweb.com/products/Win32OpenSSL.html

void PPCUpload::upload(const QString type, const double windowTop, const double windowBottom, const double time, const double distance) {

    const double horizontalSpeed = distance / time;
    const double verticalSpeed = (windowTop - windowBottom) / time;

    const int startOffset = mMainWindow->findIndexBelowT(-10.0);
    const int endOffset = mMainWindow->findIndexForLanding();

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

    double windSpeed, windDirection;
    mMainWindow->getWindSpeedDirection(&windSpeed, &windDirection);

    QString name, countrycode, equipment, place;
    if (!getUserDetails(&name, &countrycode, !type.compare("WS"), &equipment, &place)) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Upload aborted");
        msgBox.exec();
        return;
    }

    QNetworkRequest request= QNetworkRequest(QUrl("https://ppc.paralog.net/uploadtrack-db.php"));
    request.setRawHeader("User-Agent", "FlySight Viewer");
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("Charset", "utf8");

    QString post = "Source=FSV&";
        post += "Name="+QUrl::toPercentEncoding(name)+"&";
        post += "CountryCode="+QUrl::toPercentEncoding(countrycode)+"&";
        post += "Place="+QUrl::toPercentEncoding(place)+"&";
        post += "QNE="+QString::number(mMainWindow->getQNE())+"&";
        post += "Equipment="+QUrl::toPercentEncoding(equipment)+"&";
        post += "Type="+type+"&";
        post += "Timestamp="+mMainWindow->dataPoint(mMainWindow->findIndexBelowT(0.0)).dateTime.toString(Qt::ISODate)+"&";
        post += "WindowBegin="+QString::number(windowTop)+"&";
        post += "WindowEnd="+QString::number(windowBottom)+"&";
        post += "WindDir="+QString::number(windDirection)+"&";
        post += "WindSpeed="+QString::number(windSpeed)+"&";
        post += "Time="+QString::number(time)+"&";
        post += "Distance="+QString::number(distance)+"&";
        post += "VerticalSpeed="+QString::number(verticalSpeed)+"&";
        post += "HorizontalSpeed="+QString::number(horizontalSpeed)+"&";
        post += "HorizontalSpeed="+QString::number(horizontalSpeed)+"&";
        post += "Profile="+QJsonDocument(profile).toJson(QJsonDocument::Compact);

    QNetworkAccessManager *naManager = new QNetworkAccessManager(this);
    connect(naManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(finished(QNetworkReply*)));
    naManager->post(request, post.toAscii());
}

void PPCUpload::finished(QNetworkReply *reply) {
    if(reply->error() == QNetworkReply::NoError) {
        QMessageBox(QMessageBox::Information, "Succes", tr("Track submitted successfully.")).exec();
        QDesktopServices::openUrl(QUrl("https://ppc.paralog.net/showTrack.php&?track="+reply->readAll()));
    } else
        QMessageBox(QMessageBox::Critical, "Error", reply->errorString()).exec();
}

Ui::Dialog getUserDetailsUI;

bool PPCUpload::getUserDetails(QString *name, QString *countrycode,bool getEqipment, QString *equipment, QString *place) {
    QDialog userDetailsDialog;
    getUserDetailsUI.setupUi(&userDetailsDialog);
    userDetailsDialog.adjustSize();

    QNetworkRequest request = QNetworkRequest(QUrl("https://ppc.paralog.net/getnames.php"));
    request.setRawHeader("User-Agent", "FlySight Viewer");
    request.setRawHeader("Charset", "utf8");
    QNetworkAccessManager *naManager = new QNetworkAccessManager(this);
    connect(naManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(usersFinished(QNetworkReply*)));
    naManager->get(request);

    if (getEqipment) {
        request = QNetworkRequest(QUrl("https://ppc.paralog.net/getsuits.php"));
        request.setRawHeader("User-Agent", "FlySight Viewer");
        request.setRawHeader("Charset", "utf8");
        naManager = new QNetworkAccessManager(this);
        connect(naManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(suitsFinished(QNetworkReply*)));
        naManager->get(request);
    } else {
        getUserDetailsUI.wingsuitEdit->addItem("n/a");
        getUserDetailsUI.wingsuitEdit->setEnabled(false);
    }

    DataPoint p = mMainWindow->dataPoint(mMainWindow->findIndexForLanding());
    request = QNetworkRequest(QUrl("http://api.geonames.org/findNearbyPlaceNameJSON?lat="+QString::number(p.lat)+"&lng="+QString::number(p.lon)+"&username=flysight"));
    request.setRawHeader("User-Agent", "FlySight Viewer");
    request.setRawHeader("Charset", "utf8");
    naManager = new QNetworkAccessManager(this);
    connect(naManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(placeFinished(QNetworkReply*)));
    naManager->get(request);

    QRegularExpression re("^.+, [A-Z]{3}$");
    bool rc;

    while (rc = (userDetailsDialog.exec() == QDialog::Accepted)) {
        if ( getUserDetailsUI.wingsuitEdit->currentText().isEmpty()) {
            QMessageBox(QMessageBox::Critical, "Error", "Wingsuit must not be empty!").exec();
            continue;
        }
        if ( getUserDetailsUI.placeEdit->text().isEmpty()) {
            QMessageBox(QMessageBox::Critical, "Error", "Place must not be empty!").exec();
            continue;
        }
        if ( !re.match(getUserDetailsUI.nameEdit->currentText()).hasMatch()) {
            QMessageBox(QMessageBox::Critical, "Error", "Invalid Name/CountryCode!").exec();
            continue;
        }

        name->append(getUserDetailsUI.nameEdit->currentText().split(',').at(0));
        countrycode->append(getUserDetailsUI.nameEdit->currentText().split(',').at(1).trimmed());
        equipment->append(getUserDetailsUI.wingsuitEdit->currentText());
        place->append(getUserDetailsUI.placeEdit->text());

        break;
    }

    return rc;
}

void PPCUpload::usersFinished(QNetworkReply *reply) {

    if(reply->error() == QNetworkReply::NoError) {

        getUserDetailsUI.nameEdit->addItem("");

        QString result = reply->readAll();
        QStringList users = result.split('\n');
        for (int i = 0; i < users.length(); i++)
            getUserDetailsUI.nameEdit->addItem(users.at(i));
    } else
        QMessageBox(QMessageBox::Critical, "Error", reply->errorString()).exec();
}

void PPCUpload::suitsFinished(QNetworkReply *reply) {

    if(reply->error() == QNetworkReply::NoError) {

        getUserDetailsUI.wingsuitEdit->addItem("");

        QString result = reply->readAll();
        QStringList suits = result.split('\n');
        for (int i = 0; i < suits.length(); i++)
            getUserDetailsUI.wingsuitEdit->addItem(suits.at(i));
    } else
        QMessageBox(QMessageBox::Critical, "Error", reply->errorString()).exec();
}

void PPCUpload::placeFinished(QNetworkReply *reply) {

    if(reply->error() == QNetworkReply::NoError) {
        QString result = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(result.toUtf8());
        QJsonValue value = jsonResponse.object().value("geonames");
        QString place = value.toArray().at(0).toObject().value("name").toString();
        getUserDetailsUI.placeEdit->setText(place);
    } else
        QMessageBox(QMessageBox::Critical, "Error", reply->errorString()).exec();
}
