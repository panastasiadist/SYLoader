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
    core/utility.cpp \
    updater.cpp \
    messenger.cpp \
    core/downloader.cpp \
    core/network_gateway.cpp \
    core/url_processor.cpp \
    core/task_processor.cpp \
    core/extractors/youtube_extractor.cpp \
    main_form.cpp \
    main_window.cpp \
    settings_form.cpp \
    progress_item_delegate.cpp \
    about_form.cpp \
    core/queue_processor.cpp \
    core/extractors/vimeo_extractor.cpp \
    core/extractors/facebook_extractor.cpp

HEADERS  += \
    core/download.h \
    global.h \
    updater.h \
    messenger.h \
    core/utility.h \
    core/extractor.h \
    core/downloader.h \
    core/network_gateway.h \
    core/downloader_stats.h \
    core/url_processor.h \
    core/task_processor.h \
    core/extractors/youtube_extractor.h \
    progress_item_delegate.h \
    settings_form.h \
    main_window.h \
    main_form.h \
    about_form.h \
    core/queue_processor.h \
    core/downloader_progress.h \
    core/output_format.h \
    core/extractors/vimeo_extractor.h \
    core/extractors/facebook_extractor.h

FORMS    += \
    about_form.ui \
    main_form.ui \
    main_window.ui \
    settings_form.ui

RESOURCES += \
    resources.qrc

RC_ICONS = logo.ico

# Comment it if you want update check disabled
# Usefull if you need to have a package manager specific build
QMAKE_CXXFLAGS += -DWITH_UPDATE_CHECK

# Comment it if you don't distribute OpenSSL binaries with the software
QMAKE_CXXFLAGS += -DWITH_OPENSSL_NOTICE
