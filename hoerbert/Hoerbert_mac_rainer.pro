#-------------------------------------------------
#
# Project created by QtCreator 2019-09-26T11:28:25
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Hoerbert
TEMPLATE = app
DESTDIR = ../Build

#include(../libQDeviceWatcher/libQDeviceWatcher.pri)

win32:!wince*:LIBS += -lUser32

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += *.cpp

HEADERS += *.h

win32 {

	LIBS += -lhid -lsetupapi
    FFMPEG_FILES.path = ../Build/bin
    FFMPEG_FILES.files = \
            $$PWD/tools/windows/ffmpeg

    COPIES += FFMPEG_FILES
    ICON = hoerbert.ico
    RC_FILE = app.rc
}

macx {
    OTHER_FILES += \
        $$PWD/tools/mac/ffmpeg

    FFMPEG_FILES.files = $$OTHER_FILES
    FFMPEG_FILES.path = Contents/Resources
    QMAKE_BUNDLE_DATA += FFMPEG_FILES
    ICON = hoerbert.icns
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
  resources.qrc

TRANSLATIONS = languages/hoerbert_en.ts \
               languages/hoerbert_de.ts \
               languages/hoerbert_fr.ts

DISTFILES += \
    languages/hoerbert_de.qm \
    languages/hoerbert_en.qm \
    languages/hoerbert_fr.qm \
    hoerbert.icns \
    hoerbert.ico
