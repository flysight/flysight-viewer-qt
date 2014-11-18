#-------------------------------------------------
#
# Project created by QtCreator 2013-02-02T15:04:16
#
#-------------------------------------------------

QT       += core gui printsupport webkitwidgets multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FlySightViewer
TEMPLATE = app


SOURCES += main.cpp\
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
    scrubdial.cpp \
    positionslider.cpp

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
    scrubdial.h \
    positionslider.h

FORMS    += mainwindow.ui \
    configdialog.ui \
    videoview.ui

RC_ICONS = FlySightViewer.ico
ICON = FlySightViewer.icns

RESOURCES += \
    resource.qrc
