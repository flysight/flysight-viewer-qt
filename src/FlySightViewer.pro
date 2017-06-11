#-------------------------------------------------
#
# Project created by QtCreator 2013-02-02T15:04:16
#
#-------------------------------------------------

QT       += core gui printsupport webkitwidgets sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000

TARGET = FlySightViewer
TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp \
    dataplot.cpp \
    dataview.cpp \
    waypoint.cpp \
    datapoint.cpp \
    configdialog.cpp \
    mapview.cpp \
    common.cpp \
    videoview.cpp \
    windplot.cpp \
    liftdragplot.cpp \
    scoringview.cpp \
    genome.cpp \
    orthoview.cpp \
    playbackview.cpp \
    ppcform.cpp \
    speedform.cpp \
    scoringmethod.cpp \
    ppcscoring.cpp \
    speedscoring.cpp \
    wideopenspeedform.cpp \
    wideopendistanceform.cpp \
    wideopendistancescoring.cpp \
    wideopenspeedscoring.cpp \
    geographicutil.cpp \
    importworker.cpp \
    logbookview.cpp \
    performancescoring.cpp \
    performanceform.cpp \
    flareform.cpp \
    flarescoring.cpp \
    ppcupload.cpp \
    GeographicLib/Accumulator.cpp \
    GeographicLib/AlbersEqualArea.cpp \
    GeographicLib/AzimuthalEquidistant.cpp \
    GeographicLib/CassiniSoldner.cpp \
    GeographicLib/CircularEngine.cpp \
    GeographicLib/DMS.cpp \
    GeographicLib/Ellipsoid.cpp \
    GeographicLib/EllipticFunction.cpp \
    GeographicLib/GARS.cpp \
    GeographicLib/Geocentric.cpp \
    GeographicLib/GeoCoords.cpp \
    GeographicLib/Geodesic.cpp \
    GeographicLib/GeodesicExact.cpp \
    GeographicLib/GeodesicExactC4.cpp \
    GeographicLib/GeodesicLine.cpp \
    GeographicLib/GeodesicLineExact.cpp \
    GeographicLib/Geohash.cpp \
    GeographicLib/Geoid.cpp \
    GeographicLib/Georef.cpp \
    GeographicLib/Gnomonic.cpp \
    GeographicLib/GravityCircle.cpp \
    GeographicLib/GravityModel.cpp \
    GeographicLib/LambertConformalConic.cpp \
    GeographicLib/LocalCartesian.cpp \
    GeographicLib/MagneticCircle.cpp \
    GeographicLib/MagneticModel.cpp \
    GeographicLib/Math.cpp \
    GeographicLib/MGRS.cpp \
    GeographicLib/NormalGravity.cpp \
    GeographicLib/OSGB.cpp \
    GeographicLib/PolarStereographic.cpp \
    GeographicLib/PolygonArea.cpp \
    GeographicLib/Rhumb.cpp \
    GeographicLib/SphericalEngine.cpp \
    GeographicLib/TransverseMercator.cpp \
    GeographicLib/TransverseMercatorExact.cpp \
    GeographicLib/Utility.cpp \
    GeographicLib/UTMUPS.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    datapoint.h \
    dataplot.h \
    dataview.h \
    waypoint.h \
    plotvalue.h \
    configdialog.h \
    mapview.h \
    common.h \
    videoview.h \
    windplot.h \
    liftdragplot.h \
    scoringview.h \
    genome.h \
    orthoview.h \
    playbackview.h \
    ppcform.h \
    speedform.h \
    scoringmethod.h \
    ppcscoring.h \
    speedscoring.h \
    performancescoring.h \
    performanceform.h \
    wideopenspeedform.h \
    wideopendistanceform.h \
    wideopendistancescoring.h \
    wideopenspeedscoring.h \
    geographicutil.h \
    importworker.h \
    logbookview.h \
    flareform.h \
    flarescoring.h \
    ppcupload.h

FORMS    += mainwindow.ui \
    configdialog.ui \
    videoview.ui \
    scoringview.ui \
    playbackview.ui \
    ppcform.ui \
    getuserdialog.ui \
    speedform.ui \
    performanceform.ui \
    wideopenspeedform.ui \
    wideopendistanceform.ui \
    logbookview.ui \
    flareform.ui

win32 {
    RC_ICONS = FlySightViewer.ico
}
else {
    ICON = FlySightViewer.icns
}

RESOURCES += \
    resource.qrc

INCLUDEPATH += ../include
INCLUDEPATH += ../include/wwWidgets
INCLUDEPATH += ../include/GeographicLib

win32 {
    LIBS += -L../lib
    LIBS += -lVLCQtCore -l VLCQtQml -lVLCQtWidgets
    LIBS += -lwwwidgets4
}
else:macx {
    QMAKE_LFLAGS += -F../frameworks
    LIBS         += -framework VLCQtCore
    LIBS         += -framework VLCQtQml
    LIBS         += -framework VLCQtWidgets
    LIBS         += -framework wwwidgets4
}
else {
    LIBS += -L/usr/local/lib
    LIBS += -lvlc-qt -lvlc-qt-widgets
    LIBS += -lwwwidgets4
}

