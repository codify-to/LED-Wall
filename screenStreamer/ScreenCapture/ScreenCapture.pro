#-------------------------------------------------
#
# Project created by QtCreator 2013-08-20T15:02:30
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ScreenCapture
TEMPLATE = app


SOURCES += main.cpp\
        selectionareawidget.cpp \
    previewwidget.cpp

HEADERS  += selectionareawidget.h \
    previewwidget.h

RESOURCES += \
    resources.qrc

QMAKE_INFO_PLIST = Info.plist
OTHER_FILES = Info.plist
