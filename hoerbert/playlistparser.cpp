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

#include "playlistparser.h"

#include <QFile>
#include <QDebug>

PlaylistParser::PlaylistParser(QObject *parent)
    : QObject(parent)
{

}

void PlaylistParser::setTextParseDetails(bool startsWithOrNot, const QString &prefix, const QString &delimiter, bool betweenTags, const QString &surroundedBy)
{
    m_parseType = TEXT_PARSE;
    m_startsWithOrNot = startsWithOrNot;
    m_prefix = prefix;
    m_delimiter = delimiter;
    m_betweenTags = betweenTags;
    m_surroundedBy = surroundedBy;
}

void PlaylistParser::setTagParseDetails(const QString &tag, const QString &property, const QString &prefix)
{
    m_parseType = XML_PARSE;
    m_tag = tag;
    m_property = property;
    m_prefix = prefix;

    if (m_prefix.isEmpty())
        m_prefix = "<" + m_tag;
}

QStringList PlaylistParser::get(const QString &filePath)
{
    QStringList list = QStringList();
    QFile file(filePath);
    if (!file.exists())
    {
        qDebug() << "Playlist file does not exist!" << filePath;
        return list;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Playlist file cannot be opened!" << filePath;
        return list;
    }

    QStringList content = QString::fromUtf8(file.readAll()).split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    QTextStream stream(&file);
    QString line = QString();
    QString path = QString();

    if (m_parseType == TEXT_PARSE)
    {
        for (int i = 0; i < content.count(); i++)
        {
            line = content.at(i).trimmed();
            if (line.length() == 0)
                continue;

            if (m_startsWithOrNot && !line.startsWith(m_prefix, Qt::CaseSensitive))
                continue;

            if (!m_startsWithOrNot && line.startsWith(m_prefix, Qt::CaseSensitive))
                continue;

            path = textParse(line);
            path = path.replace("%20", " ").replace("%27", "'").replace("%60", "`").replace("%23", "#")
                    .replace("%25", "%").replace("%5E", "^").replace("&amp;", "&").replace("%7B", "{").replace("%7D", "}");

            if (!path.isEmpty())
                list << path;
        }
    }
    else
    {
        for (int i = 0; i < content.count(); i++)
        {
            line = content.at(i).trimmed();
            if (line.length() == 0)
                continue;

            if (!line.startsWith(m_prefix))
                continue;

            path = xmlParse(line);
            path = path.replace("%20", " ").replace("%27", "'").replace("%60", "`").replace("%23", "#")
                    .replace("%25", "%").replace("%5E", "^").replace("&amp;", "&").replace("%7B", "{").replace("%7D", "}");

            if (!path.isEmpty())
                list << path;
        }
    }

    return list;
}
QString PlaylistParser::textParse(const QString &line)
{
    QString path = QString();

    if (m_startsWithOrNot)
        path = line.section(m_prefix, 1);
    else
        path = line;

    if (m_betweenTags)
    {
        if (m_prefix.isEmpty())
            return QString();

        // prefix contains all the tag bounding path
        QString closing_tag = m_surroundedBy.left(1) + "/" + m_surroundedBy.section(m_surroundedBy.left(1), 1);
        path = path.section(m_surroundedBy, 1).section(closing_tag, 0, 0);

        QString local_file_prefix;
#ifdef _WIN32
        local_file_prefix = "file:///";
#else
        local_file_prefix = "file://";
#endif
        if (path.startsWith(local_file_prefix))
            path = path.section(local_file_prefix, 1);
    }
    else
    {
        if (!m_delimiter.isEmpty())
        {
            if (m_startsWithOrNot)
                path = path.section(m_delimiter, 1);
        }
        else
        {
            if (!m_startsWithOrNot)
            {
                path = path.trimmed();
            }
        }

        if (!m_surroundedBy.isEmpty())
        {
            path = path.section(m_surroundedBy, 1).section(m_surroundedBy, 0, -1);
        }
    }

    return path;
}

QString PlaylistParser::xmlParse(const QString &line)
{
    return line.section(m_prefix, 1).section(QString(" " + m_property + "=\""), 1).section("\"", 0, 0);
}
