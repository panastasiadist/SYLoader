#-------------------------------------------------
#
# Project created by QtCreator 2015-07-25T20:00:54
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SYLoader
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    parser.cpp \
    progressitemdelegate.cpp \
    processor.cpp \
    utility.cpp \
    updater.cpp \
    mainform.cpp \
    aboutform.cpp \
    settingsform.cpp \
    messenger.cpp

HEADERS  += mainwindow.h \
    download.h \
    parser.h \
    progressitemdelegate.h \
    global.h \
    processor.h \
    utility.h \
    processorstats.h \
    updater.h \
    mainform.h \
    aboutform.h \
    settingsform.h \
    messenger.h

FORMS    += mainwindow.ui \
    mainform.ui \
    aboutform.ui \
    settingsform.ui

RESOURCES += \
    resources.qrc
