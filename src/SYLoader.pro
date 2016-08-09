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
    core/extractors/facebook_extractor.cpp \
    core/extractor.cpp \
    core/extractors/dailymotion_extractor.cpp \
    update_form.cpp

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
    core/extractors/facebook_extractor.h \
    core/extractors/dailymotion_extractor.h \
    update_info.h \
    update_form.h

FORMS    += \
    about_form.ui \
    main_form.ui \
    main_window.ui \
    settings_form.ui \
    update_form.ui

RESOURCES += \
    resources.qrc

RC_ICONS = logo.ico

# Comment it if you want update check disabled
# Useful if you need to have a package manager aware build.
QMAKE_CXXFLAGS += -DWITH_UPDATE_CHECK

# Comment it if you don't distribute OpenSSL binaries with the software.
QMAKE_CXXFLAGS += -DWITH_OPENSSL_NOTICE

# Comment it if you want to use the globally installed ffmpeg.
# Useful if you need to have a package manager aware build.
# If you leave it uncommented, the software will try to call ffmpeg and
# ffmpeg64 bundled executables depending on the architecture currently running.
#QMAKE_CXXFLAGS += -DUSE_BUNDLED_FFMPEG
