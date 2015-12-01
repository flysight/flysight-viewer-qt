#-------------------------------------------------
#
# Project created by QtCreator 2013-02-02T15:04:16
#
#-------------------------------------------------

QT       += core gui printsupport webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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
    genome.cpp

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
    genome.h

FORMS    += mainwindow.ui \
    configdialog.ui \
    videoview.ui \
    scoringview.ui

win32 {
    RC_ICONS = FlySightViewer.ico
}
else {
    ICON = FlySightViewer.icns
}

RESOURCES += \
    resource.qrc

LIBS     += -lvlc-qt -lvlc-qt-widgets

win32 {
    LIBS        += -L../lib
    INCLUDEPATH += ../include
}
else {
    LIBS        += -L/usr/local/lib
    INCLUDEPATH += /usr/local/include
}
