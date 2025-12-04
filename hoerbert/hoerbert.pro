#-------------------------------------------------
#
# hörbert Software
# Copyright (C) 2019 WINZKI GmbH & Co. KG
#
# Authors of the original version: Igor Yalovenko, Rainer Brang
# Dec. 2019
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>
#
#-------------------------------------------------

QT       += core gui xml network concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hoerbert
TEMPLATE = app
CONFIG += QMAKE_LFLAGS_WINDOWS
DESTDIR = ../../Build

win32:!wince*:LIBS += -lUser32

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += CALCULATE_SCALING_FACTOR

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

SOURCES += \
        aboutdialog.cpp \
        advancedfeaturesdialog.cpp \
        audiobookconverter.cpp \
        audioinfothread.cpp \
        backupmanager.cpp \
        backuprestoredialog.cpp \
        capacitybar.cpp \
        cardpage.cpp \
        cdripper.cpp \
        convertchaptertask.cpp \
        debugdialog.cpp \
        devicemanager.cpp \
        dpiscale.cpp \
        functions.cpp \
        generalexception.cpp \
        hoerbertprocessor.cpp \
        main.cpp \
        mainwindow.cpp \
        piebutton.cpp \
        playlistpage.cpp \
        playlistparser.cpp \
        playlistview.cpp \
        playsymbolbutton.cpp \
        pleasewaitdialog.cpp \
        processexecutor.cpp \
        runguard.cpp \
        triplecheckbox.cpp \
        triplecheckboxwidget.cpp \
        waitingspinnerwidget.cpp \
        windowsdrivelistener.cpp \
        xmlmetadatareader.cpp \
        xmlwriter.cpp

HEADERS += \
        aboutdialog.h \
        advancedfeaturesdialog.h \
        audiobookconverter.h \
        audioinfothread.h \
        backupmanager.h \
        backuprestoredialog.h \
        capacitybar.h \
        cardpage.h \
        cdripper.h \
        convertchaptertask.h \
        debugdialog.h \
        devicemanager.h \
        dpiscale.h \
        functions.h \
        generalexception.h \
        define.h \
        hoerbertprocessor.h \
        mainwindow.h \
        piebutton.h \
        playlistpage.h \
        playlistparser.h \
        playlistview.h \
        playsymbolbutton.h \
        pleasewaitdialog.h \
        processexecutor.h \
        runguard.h \
        triplecheckbox.h \
        triplecheckboxwidget.h \
        version.h \
        waitingspinnerwidget.h \
        windowsdrivelistener.h \
        xmlmetadatareader.h \
        xmlwriter.h

win32 {
    LIBS += -lhid -lsetupapi -lgdi32
    INCLUDEPATH += $$(QTDIR)/include/

    ICON = hoerbert.ico
    RC_FILE = app.rc
    CONFIG += embed_manifest_dll
    CONFIG += embed_manifest_exe

    DESTDIR = ../Build/bin/
    TOOLS.path = ../Build/bin/

    contains(QMAKE_TARGET.arch, x86_64) {
        message("x86_64 build")
        ## Windows x64 (64bit) specific build here

        TOOLS.files = \
                $$PWD/tools/windows/64/ffmpeg \
                $$PWD/tools/windows/64/freac \
                $$PWD/tools/windows/64/Sync \
                $$PWD/tools/windows/64/7z \
                $$PWD/tools/windows/64/EjectMedia \
                $$PWD/tools/diagnostics

    } else {
        message("x86 build")
        ## Windows x86 (32bit) specific build here

        TOOLS.files = \
                $$PWD/tools/windows/32/ffmpeg \
                $$PWD/tools/windows/32/freac \
                $$PWD/tools/windows/32/Sync \
                $$PWD/tools/windows/32/7z \
                $$PWD/tools/windows/32/EjectMedia \
                $$PWD/tools/diagnostics

    }

    COPIES += TOOLS

    DISTFILES += \
        hoerbert.exe.manifest\
        hoerbert.icns \
        hoerbert.ico
}


macx {
    message("macx build")
    OTHER_FILES += \
        $$PWD/tools/mac/ffmpeg \
        $$PWD/tools/mac/freac \
        $$PWD/tools/diagnostics

    TOOLS.files = $$OTHER_FILES
    TOOLS.path = Contents/Resources
    QMAKE_BUNDLE_DATA += TOOLS
    ICON = hoerbert.icns
    QMAKE_TARGET_BUNDLE_PREFIX = de.winzki
}

linux {
    message("linux build")
    OTHER_FILES += \
        $$PWD/tools/linux/ffmpeg \
        $$PWD/tools/linux/freac \
        $$PWD/tools/diagnostics


    TOOLS.files = $$OTHER_FILES
    TOOLS.path = ../../Build/
    QMAKE_BUNDLE_DATA += TOOLS
    COPIES += TOOLS

    DISTFILES += \
        hoerbert.png

}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

TRANSLATIONS = \
    languages/hoerbert_en.ts \
    languages/hoerbert_de.ts \
    languages/hoerbert_fr.ts
