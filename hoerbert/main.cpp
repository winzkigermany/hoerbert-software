/***************************************************************************
 * hörbert Software
 * Copyright (C) 2019 WINZKI GmbH & Co. KG
 *
 * Authors of the original version: Igor Yalovenko, Rainer Brang
 * Dec. 2019
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#include "mainwindow.h"

#include <csignal>

#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QTextCodec>

#include "define.h"
#include "functions.h"
#include "dpiscale.h"
#include "runguard.h"

QString FFMPEG_PATH;
QString FFPROBE_PATH;
QString FFPLAY_PATH;
QString FREAC_PATH;
QString SYNC_PATH;

#if defined (Q_OS_WIN)
QString ZIP_PATH;
#endif

QString HOERBERT_TEMP_PATH;
QStringList PROCESS_ERROR;

[[ noreturn ]] void signalHandler(int signum)
{
    fprintf(stderr, "Interrupt signal(%d) received.\n", signum);

    if (!deleteAllFilesInDirecotry(HOERBERT_TEMP_PATH))
    {
        perror("Failed to delete old files in temp folder");
    }

    exit(signum);
}

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    signal(SIGINT, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGSEGV, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGFPE, signalHandler);
    signal(SIGILL, signalHandler);

    qRegisterMetaType<QMap<ENTRY_LIST_TYPE,AudioList> >("QMap<ENTRY_LIST_TYPE, AudioList>");

    QCoreApplication::setAttribute( Qt::AA_EnableHighDpiScaling );
    QApplication a(argc, argv);

    QString localeName = QLocale::system().name();

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + localeName.mid(0,2), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator qtBaseTranslator;
    qtTranslator.load("qtbase_" + localeName.mid(0,2), ":/languages");
    a.installTranslator(&qtBaseTranslator);

    QTranslator myappTranslator;
    myappTranslator.load("hoerbert_" + localeName.mid(0,2), ":/languages");
    a.installTranslator(&myappTranslator);

    PROCESS_ERROR = QStringList() << "FailedToStart" << "Crashed" << "Timedout" << "WriteError" << "ReadError" << "UnknownError";

    QString platform = QSysInfo::productType();
    qDebug() << platform.toUpper() + " " + QSysInfo::kernelType() + " " + QSysInfo::kernelVersion();

    if (platform.compare("windows") == 0)
    {
#ifdef _WIN64
        FFMPEG_PATH  = QCoreApplication::applicationDirPath() + FFMPEG_PATH_WIN + "ffmpeg64.exe";

        FFPROBE_PATH = QCoreApplication::applicationDirPath() + FFMPEG_PATH_WIN + "ffprobe64.exe";

        FFPLAY_PATH  = QCoreApplication::applicationDirPath() + FFMPEG_PATH_WIN + "ffplay64.exe";

        FREAC_PATH   = QCoreApplication::applicationDirPath() + FREAC_PATH_WIN + "freaccmd.exe";

        SYNC_PATH    = QCoreApplication::applicationDirPath() + "/Sync/sync64.exe";
#else
        FFMPEG_PATH = QCoreApplication::applicationDirPath() + FFMPEG_PATH_WIN + "ffmpeg32.exe";

        FFPROBE_PATH = QCoreApplication::applicationDirPath() + FFMPEG_PATH_WIN + "ffprobe32.exe";

        FFPLAY_PATH = QCoreApplication::applicationDirPath() + FFMPEG_PATH_WIN + "ffplay32.exe";

        FREAC_PATH = QCoreApplication::applicationDirPath() + FREAC_PATH_WIN + "freaccmd.exe";

        SYNC_PATH    = QCoreApplication::applicationDirPath() + "/Sync/sync.exe";
#endif

        HOERBERT_TEMP_PATH = tailPath(QStandardPaths::writableLocation(QStandardPaths::TempLocation));

#if defined (Q_OS_WIN)
        ZIP_PATH = QCoreApplication::applicationDirPath() + "/7z/7za.exe";
#endif

    }
    else if (platform.compare("osx") == 0)
    {
        FFMPEG_PATH  = QCoreApplication::applicationDirPath() + FFMPEG_PATH_MAC + "ffmpeg";

        FFPROBE_PATH = QCoreApplication::applicationDirPath() + FFMPEG_PATH_MAC + "ffprobe";

        FFPLAY_PATH  = QCoreApplication::applicationDirPath() + FFMPEG_PATH_MAC + "ffplay";

        FREAC_PATH   = QCoreApplication::applicationDirPath() + FREAC_PATH_MAC + "freaccmd";

        SYNC_PATH    = "/bin/sync";

        HOERBERT_TEMP_PATH = "/tmp/.hoerbert/";
    }
    else if ( QSysInfo::kernelType() == "linux" )
    {
        FFMPEG_PATH  = QCoreApplication::applicationDirPath() + FFMPEG_PATH_LINUX + "ffmpeg";

        FFPROBE_PATH = QCoreApplication::applicationDirPath() + FFMPEG_PATH_LINUX + "ffprobe";

        FFPLAY_PATH  = QCoreApplication::applicationDirPath() + FFMPEG_PATH_LINUX + "ffplay";

        FREAC_PATH   = QCoreApplication::applicationDirPath() + FREAC_PATH_LINUX + "freaccmd";

        SYNC_PATH    = "/bin/sync";

        HOERBERT_TEMP_PATH = "/tmp/.hoerbert/";
    }

    QDir dir(HOERBERT_TEMP_PATH);
    if (!dir.exists())
    {
        if (!dir.mkpath(HOERBERT_TEMP_PATH))
            qDebug() << "Failed creating temp directory for ripping.";
    }

    QFile ffmpeg_file(FFMPEG_PATH);
    if (!ffmpeg_file.exists())
    {
        qDebug() << FFMPEG_PATH;
        QMessageBox::critical(nullptr, "hörbert", QObject::tr("Cannot find ffmpeg binary. Please reinstall this app."));
        return -1;
    }

    QFile ffprobe_file(FFPROBE_PATH);
    if (!ffprobe_file.exists())
    {
        qDebug() << FFPROBE_PATH;
        QMessageBox::critical(nullptr, "hörbert", QObject::tr("Cannot find ffprobe binary. Please reinstall this app."));
        return -1;
    }

    QFile ffplay_file(FFPLAY_PATH);
    if (!ffplay_file.exists())
    {
        qDebug() << FFPLAY_PATH;
        QMessageBox::critical(nullptr, "hörbert", QObject::tr("Cannot find ffplay binary. Please reinstall this app."));
        return -1;
    }

    QFile freac_file(FREAC_PATH);
    if (!freac_file.exists())
    {
        qDebug() << FREAC_PATH;
        QMessageBox::critical(nullptr, "hörbert", QObject::tr("Cannot find freac binary. Please reinstall this app."));
        return -1;
    }

    QFile sync_file(SYNC_PATH);
    if (!sync_file.exists())
    {
        qDebug() << SYNC_PATH;
        QMessageBox::critical(nullptr, "hörbert", QObject::tr("Cannot find sync binary. Please reinstall this app."));
        return -1;
    }

#if defined (Q_OS_WIN)
    QFile zip_file(ZIP_PATH);
    if (!zip_file.exists())
    {
        qDebug() << SYNC_PATH;
        QMessageBox::critical(nullptr, "hörbert", QObject::tr("Cannot find 7zip binary. Please reinstall this app."));
        return -1;
    }
#endif

    QCoreApplication::setOrganizationName("WINZKI GmbH & Co. KG");
    QCoreApplication::setOrganizationDomain("hoerbert.com");
    QCoreApplication::setApplicationName("hörbert");

    // default settings of the app
    QSettings settings;
    settings.beginGroup("Global");

    QString audio_volume = settings.value("volume").toString();
    if (audio_volume.isEmpty())
        settings.setValue("volume", "-1.5");

    QString show_reminder = settings.value("showBackupReminder").toString();
    if (show_reminder.isEmpty())
        settings.setValue("showBackupReminder", false);

    QString generateXml = settings.value("regenerateHoerbertXml").toString();
    if (generateXml.isEmpty())
        settings.setValue("regenerateHoerbertXml", true);

    QString darkMode = settings.value("darkMode").toString();
    if (darkMode.isEmpty())
        settings.setValue("darkMode", false);

    settings.endGroup();

    QApplication::setStyle("Fusion");

    if( darkMode == "true" ){
        QPalette palette;
        // Now use a palette to switch to dark colors.
        // I found this nice color scheme at https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle/blob/master/DarkStyle.cpp
        palette.setColor(QPalette::Window,QColor(53,53,53));
        palette.setColor(QPalette::WindowText,Qt::white);
        palette.setColor(QPalette::Disabled,QPalette::WindowText,QColor(127,127,127));
        palette.setColor(QPalette::Base,QColor(42,42,42));
        palette.setColor(QPalette::AlternateBase,QColor(66,66,66));
        palette.setColor(QPalette::ToolTipBase,QColor(35,35,35));
        palette.setColor(QPalette::ToolTipText,Qt::white);
        palette.setColor(QPalette::Text,Qt::white);
        palette.setColor(QPalette::Disabled,QPalette::Text,QColor(127,127,127));
        palette.setColor(QPalette::Dark,QColor(35,35,35));
        palette.setColor(QPalette::Shadow,QColor(20,20,20));
        palette.setColor(QPalette::Button,QColor(60,60,60));
        palette.setColor(QPalette::ButtonText,Qt::white);
        palette.setColor(QPalette::Disabled,QPalette::ButtonText,QColor(127,127,127));
        palette.setColor(QPalette::BrightText,Qt::red);
        palette.setColor(QPalette::Link,QColor(42,130,218));
        palette.setColor(QPalette::Highlight,QColor(42,130,218));
        palette.setColor(QPalette::Disabled,QPalette::Highlight,QColor(80,80,80));
        palette.setColor(QPalette::HighlightedText,Qt::white);
        palette.setColor(QPalette::Disabled,QPalette::HighlightedText,QColor(127,127,127));

        QApplication::setPalette(palette);

        a.setStyle( "#ColorblindHint { background-color:#353535; }" );
        a.setStyle( "#PlaylistTable {background-color:#353535; color:white; alternate-background-color: rgba(48, 48, 48, 0);}" );
        a.setStyle( "QLineEdit {background-color: #404040; color: white;}" );
        a.setStyle( "#BigButton {background-color:#353535; color:white; border: 1px solid #d3d3d3; border-radius: 5px;}" );
    } else {
        a.setStyle( "#ColorblindHint { background-color:#ffffff; }" );
        a.setStyle( "#PlaylistTable {background-color: white; color: black; alternate-background-color: rgba(245, 245, 245, 0);}" );
        a.setStyle( "QLineEdit {background-color: white; color: black;}" );
        a.setStyle( "#BigButton {background-color:#dddddd; color: black; border: 1px solid #d3d3d3; border-radius: 5px;}" );
    }

    qRegisterMetaType <AudioList> ("AudioList");


    RunGuard guard( "winzki_hoerbert_app_running" );
    if ( !guard.tryToRun() )
    {
        // this app is already running.
        QMessageBox::information(NULL, QObject::tr("Already running"), QObject::tr("This app is already running. It can not be started twice."));
        exit(0);
    }

    MainWindow w;
    w.show();

    return a.exec();
}
