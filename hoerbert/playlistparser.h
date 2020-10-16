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

#ifndef PLAYLISTPARSER_H
#define PLAYLISTPARSER_H

#include <QObject>

/**
 * @brief The PlaylistParser class parses playlist file and gets a list of files
 */
class PlaylistParser : public QObject
{
    Q_OBJECT
public:

    enum parse_type {
        TEXT_PARSE = 0,
        XML_PARSE
    } PARSE_TYPE;
    /**
     * @brief PlaylistParser
     * @param parent
     */
    explicit PlaylistParser(QObject *parent = nullptr);

    /**
     * @brief setTextParseDetails get details about how to parse the playlist file
     * @param startsWithOrNot true if paths start with certain string(or character),
     *        false path lines do not include prefix while other lines do
     * @param prefix prefix attached to the lines excluding trailing indices
     * @param delimiter delimiting character or string
     * @param betweenTags true if the path is between xml/html tags or false otherwise
     * @param surroundedBy indicates which character or string the path is surrounded by
     */
    void setTextParseDetails(bool startsWithOrNot = false, const QString &prefix = "#",
                         const QString &delimiter = "=", bool betweenTags = false, const QString &surroundedBy = "");

    void setTagParseDetails(const QString &tag = "media", const QString &property = "src", const QString &prefix = "");

    /**
     * @brief get
     * @param filePath absolute path to the playlist file
     * @return returns a list of audio file paths inside the playlist file
     */
    QStringList get(const QString &filePath);
signals:

private:

    /**
     * @brief textParse parse given line in text mode
     * @param line single line of string to be parsed
     * @return path of audio file
     */
    QString textParse(const QString &line);

    /**
     * @brief xmlParse parse given line in xml mode
     * @param line single line of string to be parsed
     * @return path of audio file
     */
    QString xmlParse(const QString &line);

    int m_parseType;

    bool m_startsWithOrNot;
    QString m_prefix;
    QString m_delimiter;
    bool m_betweenTags;
    QString m_surroundedBy;

    QString m_tag;
    QString m_property;
};

#endif // PLAYLISTPARSER_H
