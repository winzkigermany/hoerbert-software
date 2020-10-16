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

#ifndef XMLMETADATAREADER_H
#define XMLMETADATAREADER_H

#include <QObject>
#include <QXmlStreamReader>

#include "define.h"

class QString;

/**
 * @brief XmlMetadataReader reads hoerbert.xml file and returns list of entries
 *         whose metadata need to be written
 */
class XmlMetadataReader : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief XmlMetadataReader Contructor
     * @param path absolute directory path to the file to be parsed
     */
    XmlMetadataReader(const QString &path);

    /**
     * @brief parse xml file and return list of entries
     * @return list of entries
     */
    AudioList getEntryList();

signals:

    void failed(int statusCode, const QString &errorString);

private:

    /**
     * @brief read the file
     */
    void read();

    /**
     * @brief parse <folder> tag
     */
    void processFolders();

    /**
     * @brief parse <item> tag
     */
    void processFolder(const QString &folderID);

    /**
     * @brief processItems
     */
    void processItems(int folderID);

    /**
     * @brief processItem
     * @param itemID
     */
    void processItem(int folderID);

    /**
     * @brief readNextText
     * @return text of current element
     */
    QString readNextText();

    AudioList m_entryList;
    QString m_path; // absolute path
    QXmlStreamReader m_xml;
    int m_entryID;
};

#endif // XMLMETADATAREADER_H
