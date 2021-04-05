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

#include "xmlmetadatareader.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QUrl>

#include "functions.h"

XmlMetadataReader::XmlMetadataReader(const QString &path)
{
    m_path = tailPath(path);

    m_entryID = 0;
}
AudioList XmlMetadataReader::getEntryList()
{
    read();
    return m_entryList;
}
void XmlMetadataReader::read()
{
    QString file_name = m_path + HOERBERT_XML;

    QFile file(file_name);
    if (!file.exists()) {
        qDebug() << "Failed reading xml file. Does not exist:" << file_name;
        emit failed(0, "Failed reading xml file. Does not exist: " + file_name);
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed opening xml file:" << file_name;
        emit failed(1, "Failed opening xml file: " + file_name);
        return;
    }

    m_xml.setDevice(&file);
    QString name;
    while (m_xml.readNextStartElement()) {
        name = m_xml.name().toString();
        //qDebug() << "#1" << name;
        if (m_xml.name().compare(QString("hoerbert_playlists")) == 0) {
            while (m_xml.readNextStartElement()) {
                if (m_xml.name().compare(QString("folders")) == 0) {
                    processFolders();
                }
                else
                    m_xml.skipCurrentElement();
            }
        }
        /*if (m_xml.name().compare(QString("version")) == 0)
            processFolders();
        if (m_xml.name().compare(QString("hoerbert_playlists")) == 0) {
            while (m_xml.readNextStartElement()) {
                qDebug() << "#2" << m_xml.name();

            }
        }*/
    }

    if (m_xml.tokenType() == QXmlStreamReader::Invalid)
        m_xml.readNext();

    if (m_xml.hasError())
    {
        //m_xml.raiseError();
        qDebug() << "Failed parsing xml file:" << m_xml.errorString();
        emit failed(2, "Failed parsing xml file: " + m_xml.errorString());
        return;
    }
}
void XmlMetadataReader::processFolders()
{
    qDebug() << "Process Folders";

    while (m_xml.readNextStartElement()) {
        //qDebug() << "#3" << m_xml.name();
        if (m_xml.name().compare(QString("folder")) == 0)
            processFolder(m_xml.attributes().value("id").toString());
    }
}
void XmlMetadataReader::processFolder(const QString &folderID)
{
    //qDebug() << "process folder";
    if (folderID.isEmpty())
    {
        qDebug() << "Folder ID is not specified!";
        return;
    }

    if (!m_xml.isStartElement() || m_xml.name().compare(QString("folder")) != 0)
        return;

    auto folder_id = folderID.toInt();

    while (m_xml.readNextStartElement()) {
        if (m_xml.name().compare(QString("items")) == 0)
            processItems(folder_id);
        else
            m_xml.skipCurrentElement();
    }
}
void XmlMetadataReader::processItems(int folderID)
{
    //qDebug() << "process items";
    if (!m_xml.isStartElement() || m_xml.name().compare(QString("items")) != 0)
        return;

    while (m_xml.readNextStartElement()) {
        if (m_xml.name().compare(QString("item")) == 0)
            processItem(folderID);
        else
            m_xml.skipCurrentElement();
    }
}

void XmlMetadataReader::processItem(int folderID)
{
    if (!m_xml.isStartElement() || m_xml.name().compare(QString("item")) != 0)
        return;

    auto guid = m_xml.attributes().value("guid").toString();
    QString sequence;
    QString source;
    QString userLabel;
    QString byteSize;
    while (m_xml.readNextStartElement()) {
        auto name = m_xml.name();
        //qDebug() << "#4" << name;
        if (name.compare(QString("sequence")) == 0)
            sequence = m_xml.readElementText();
        else if (name.compare(QString("source")) == 0)
            source = m_xml.readElementText();
        else if (name.compare(QString("userLabel")) == 0)
            userLabel = m_xml.readElementText();
        else if (name.compare(QString("byteSize")) == 0) {
            //qDebug() << "ByteSize";
            byteSize = m_xml.readElementText();
        }
        else
            m_xml.skipCurrentElement();
    }
    qDebug() << guid << sequence << source << userLabel << byteSize;

    AudioEntry entry;
    entry.id = m_entryID;
    entry.order = sequence.toInt();
    entry.path = m_path + QString::number(folderID) + "/" + sequence + DEFAULT_DESTINATION_FORMAT;
    entry.metadata.title = userLabel;
    entry.metadata.comment = QUrl::fromPercentEncoding(source.toUtf8());
    entry.flag = 5; // metadata changed

    m_entryList.insert(entry.id, entry);

    m_entryID++;
}
QString XmlMetadataReader::readNextText()
{
    m_xml.readNext();
    return m_xml.text().toString();
}
