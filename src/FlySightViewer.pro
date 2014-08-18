#-------------------------------------------------
#
# Project created by QtCreator 2013-02-02T15:04:16
#
#-------------------------------------------------

QT       += core gui printsupport

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
    configdialog.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    datapoint.h \
    dataplot.h \
    dataview.h \
    waypoint.h \
    plotvalue.h \
    configdialog.h

FORMS    += mainwindow.ui \
    configdialog.ui

RC_ICONS = FlySightViewer.ico
