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

#ifndef DEFINE_H
#define DEFINE_H

#include <QString>
#include <QColor>
#include <QMap>

// fonts, styles and colors
#if defined (Q_OS_MACOS)
const QString HOERBERT_FONTFAMILY                     = "Lucida Grande";
#elif defined (Q_OS_WIN)
const QString HOERBERT_FONTFAMILY                     = "Segoe UI";
#elif defined (Q_OS_LINUX)
const QString HOERBERT_FONTFAMILY                     = "Ubuntu Regular";
#endif

const QString PIE_STYLE_TEMPLATE =
        "#PieButton {color: %5;background-color: %2;"
        " border-radius: %1px; border-width: 1px; border-style: solid; border-color: rgba(55, 55, 55, 155);}"
        "#PieButton:hover {color: %5;background-color: %3;"
        " border-radius: %1px; border-width: 1px; border-style: solid; border-color: rgba(55, 55, 55, 155);}"
        "#PieButton:pressed {color: %5;background-color: %4;"
        " border-radius: %1px; border-width: 1px; border-style: solid; border-color: rgba(55, 55, 55, 155);}";

const QString FLAT_STYLE_TEMPLATE = "#StyleButton {color: white;background: qlineargradient(spread:pad, x1:0 y1:0, x2:1 y2:1, stop:0 rgba(15, 157, 88, 255), stop:1 rgba(55, 157, 88, 225)); border-radius: 3px; padding: 5px 25px 5px 25px}";

const QString BAR_STYLE_TEMPLATE = "#CardSizeBar {background: qlineargradient(spread:pad, x1:0 y1:0, x2:1 y2:1, stop:0 rgba(15, 157, 88, 255), stop:1 rgba(55, 157, 88, 225)); border-radius: 3px; padding: 1px;}";

//qradialgradient(cx: 0.5, cy: 0.5, radius: 2, fx: 0.5, fy: 1, stop: 0 rgba(255,30,30,255), stop: 0.2 rgba(255,30,30,144), stop: 0.4 rgba(255,30,30,32)

const QString SUPPORTED_FILES = "*.wav *.mp3 *.ogg *.flac *.m4a *.m4b *.wma *.aif *.aiff *cda *.m3u *.m3u8 *.wpl *.xml *.xspf";

const QColor CL_DIR0 = QColor(60, 40, 43);
const QColor CL_DIR1 = QColor(169, 21, 11);
const QColor CL_DIR2 = QColor(43, 44, 124);
const QColor CL_DIR3 = QColor(80, 159, 76);
const QColor CL_DIR4 = QColor(225, 175, 42);
const QColor CL_DIR5 = QColor(80, 104, 132);
const QColor CL_DIR6 = QColor(39, 113, 194);
const QColor CL_DIR7 = QColor(195, 60, 13);
const QColor CL_DIR8 = QColor(19, 96, 83);

const QStringList COLOR_LIST = QStringList() << "#3c283f" << "#a9150b" << "#2b2c7c" << "#509f4c" << "#e1af2a" << "#506884" << "#2771c2" << "#c33c0d" << "#136053";

struct MetaData {
    QString title;
    QString album;
    QString comment;

    bool operator!=(const MetaData& a) const {
        if (a.title == title && a.album == album && a.comment == comment)
            return false;
        else
            return true;
    }

    bool operator==(const MetaData& a) const {
        if (a.title == title && a.album == album && a.comment == comment)
            return true;
        else
            return false;
    }
};

struct AudioEntry {
    int id; // identifier
    int order; // row number in list
    int state; // checked or unchecked or 3
    QString path; // actual path on disk
    MetaData metadata; // user modified text or original file name
    int duration; // in seconds
    int flag = -1; // -1: default, 0: added, 1: metadata, 2: split on silence, 3: split per 3, 4: renamed, 5: silence,
};

typedef QMap<int, AudioEntry> AudioList;
typedef QMapIterator<int, AudioEntry> AudioListIterator;

enum ENTRY_LIST_TYPE {
    REMOVED_ENTRIES = 0,
    ADDED_ENTRIES,
    MOVED_ENTRIES,
    SPLIT_ENTRIES,
    METADATA_CHANGED_ENTRIES
};


/*!
 * \brief Path and files
 */
const int VOLUME_SIZE_LIMIT                  = 32; // in Gigabytes

const QString DEFAULT_DESTINATION_FORMAT     = ".WAV";

const QString FFMPEG_PATH_WIN                = "/ffmpeg/";
const QString FFMPEG_PATH_MAC                = "/../Resources/ffmpeg/";
const QString FFMPEG_PATH_LINUX              = "/ffmpeg/";

const QString FREAC_PATH_WIN                 = "/freac/";
const QString FREAC_PATH_MAC                 = "/../Resources/freac/MacOS/";
const QString FREAC_PATH_LINUX               = "/freac/";

const QString DIAG_FILES_PATH_WIN            = "/diagnostics/";
const QString DIAG_FILES_PATH_MAC            = "/../Resources/diagnostics/";
const QString DIAG_FILES_PATH_LINUX          = "/diagnostics/";

const QString HOERBERT_XML                   = "hoerbert.xml";
const QString HOERBERT_XML_BACKUP            = "hoerbert.bak";
const QString BACKUP_INFO_FILE               = "backup.xml";
const QString CARD_INFO_FILE                 = "info.xml";
const QString CONTENT_HTML                   = "contents.html";
const QString DIAGMODE_FILE                  = "diag.txt";
const QString DIAGMODE_ORIGINALS_DIR         = "originals";

const QString COLLECT_FILE_NAME              = "winzki_service";


const int METADATA_MAX_LENGTH                = 80; // in bytes
const int WAV_HEADER_SIZE_IN_BYTES           = 44;
const int MEMORY_SPARE_SPACE_IN_BYTES        = 1024;

#endif // DEFINE_H
