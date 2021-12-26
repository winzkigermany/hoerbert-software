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
 *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#include "xmlwriter.h"

#include <QDebug>
#include <QRandomGenerator>
#include <QFile>
#include <QDir>
#include <QUuid>

#include "version.h"

XmlWriter::XmlWriter(const QString &path, const AudioList &audioInfoList, bool isDiagMode)
{
    m_path = path;
    m_audioInfoList = audioInfoList;
    m_isDiagnosticsMode = isDiagMode;

    // delete old file if exists
    QFile file(m_path);
    if (file.exists())
        file.remove();

    if (m_audioInfoList.count() <= 0) {
        qDebug() << "XmlWriter: Empty file list!";
        return;
    }
}

bool XmlWriter::create()
{
    QString header = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                     "<hoerbert>\n"
                     "\t<hoerbert_playlists>\n"
                     "\t\t<version>1.0</version>\n"
                     "\t\t<generator>%1</generator>\n"
                     "\t\t<folders>\n";
    QString folder_start = "\t\t\t<folder id=\"%1\">\n"
                           "\t\t\t\t<items>\n";
    QString folder_end = "\t\t\t\t</items>\n"
                         "\t\t\t</folder>\n";
    QString item_start = "\t\t\t\t\t<item guid=\"%1\">\n";
    QString item_end = "\t\t\t\t\t</item>\n";
    QString item_content = "\t\t\t\t\t\t<sequence>%1</sequence>\n"
                      "\t\t\t\t\t\t<source>%2</source>\n"
                      "\t\t\t\t\t\t<userLabel>%3</userLabel>\n"
                      "\t\t\t\t\t\t<byteSize>%4</byteSize>\n";
    QString tail = "\t\t</folders>\n"
                   "\t<debug>%1</debug>\n"
                   "\t</hoerbert_playlists>\n"
                   "</hoerbert>\n";
    header = header.arg(VER_PRODUCTVERSION_STR);
    tail = tail.arg(m_isDiagnosticsMode ? "true" : "false");

    QString content = "";
    auto current_dir = -1;
    auto folderItemCount = 0;

    for (auto entry : m_audioInfoList)
    {
        auto dir_num = getDirectoryNumber(entry.path);

        if (dir_num != current_dir) {
            current_dir = dir_num;
            if (current_dir != 0) {
                content += folder_end;
            }
            content += folder_start.arg(current_dir);
            folderItemCount = 0;
        }

        QString guid = QUuid::createUuid().toString( QUuid::WithoutBraces ).toUpper();

        content += item_start.arg(guid);
        content += item_content.arg(folderItemCount).arg(entry.metadata.comment).arg(entry.metadata.title).arg(QFile(entry.path).size());
        content += item_end;

        folderItemCount++;
    }
    content += folder_end;

    QFile file(m_path);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << header << content << tail << Qt::endl;

        // pad the file to  a size of 100kb. This is enough empty space for any xml data,
        // AND it reserves the space on the card, so it's always possible to write a new hoerbert.xml file.
        int byteSize = static_cast<int>(file.size());   // force to smaller type is acceptable here, we assume no fatal problems.
        int paddingByteCount = 100*1024 - byteSize;

        if( paddingByteCount>0 ){
            QString padding = QString( paddingByteCount, ' ' );
            stream << padding << Qt::endl;
        }

        file.close();
    }
    else {
        qDebug() << "XmlWriter: Cannot create xml file!";
        return false;
    }

    return true;
}

int XmlWriter::getDirectoryNumber(const QString &path)
{
    /// TODO: Must confirm separator for Mac and Linux
    QString separator = "/";
#ifdef Q_OS_MAC
    separator = "/";
#elif defined(Q_OS_WIN)
    separator = "/";
#else
    separator = "/";
#endif
    return path.section(separator, -2, -2).toInt();
}
