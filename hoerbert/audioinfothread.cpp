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

#include "audioinfothread.h"

#include <QDir>
#include <QFileInfoList>
#include <QApplication>
#include <QTime>
#include <QProcess>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include "qprocesspriority.h"

extern QString FFPROBE_PATH;

AudioInfoThread::AudioInfoThread()
    : QThread()
{
    m_flag = -1;
    m_IDBeginsFrom = 0;
}
AudioInfoThread::AudioInfoThread(const QFileInfoList &fileList, int idBeginsFrom, int flag)
    : QThread()
{
    m_fileList = fileList;
    m_flag = flag;
    m_IDBeginsFrom = idBeginsFrom;
}

AudioInfoThread::~AudioInfoThread()
{

}
void AudioInfoThread::setFileList(const QFileInfoList &fileList)
{
    m_fileList = fileList;
}
void AudioInfoThread::setFileListWithMetadata(const QFileInfoList &fileList, const QStringList &metadataList)
{
    m_fileList = fileList;
    m_metadataList = metadataList;
    if (m_fileList.count() != m_metadataList.count())
    {
        qDebug() << "Number of files and number of metadata do not match.";
    }
}
void AudioInfoThread::setBeginID(int beginID)
{
    m_IDBeginsFrom = beginID;
}
void AudioInfoThread::setDeafultFlag(int flag)
{
    m_flag = flag;
}
void AudioInfoThread::run()
{
    if (m_fileList.count() == 0)
        return;

    AudioList entry_list;

    int counter = 0;

    // create a new process to run the command
    QProcessPriority * process = new QProcessPriority(this->parent());
    process->setPriority(QProcessPriority::NormalPriority);

    QStringList arguments;
    arguments.append("-i");
    arguments.append("<SOURCE_PATH>");
    arguments.append("-hide_banner");
    arguments.append("-show_format");
    arguments.append("-print_format");
    arguments.append("json");
    arguments.append("-v");
    arguments.append("quiet");

    AudioEntry entry;

    bool readFromCard = (m_IDBeginsFrom == 0);

    emit processUpdated(counter);

    for (int i = 0; i < m_fileList.size(); i++)
    {
        QFileInfo info = m_fileList.at(i);

        arguments.replace(1, info.absoluteFilePath());

        process->start(FFPROBE_PATH, arguments);
        process->waitForFinished();

        // parse the output to get desired data
        QString output = process->readAllStandardOutput();

        process->close();

        QJsonDocument jsonDocument = QJsonDocument::fromJson(output.toUtf8());
        QJsonObject jsonObject = jsonDocument.object();
        QJsonObject formatObject = jsonObject["format"].toObject();
        QJsonObject tagsObject = formatObject["tags"].toObject();

        // this is in format->tags->title
        QString metadata_title = tagsObject["title"].toString();
        QString metadata_album = tagsObject["album"].toString();
        QString metadata_comment = tagsObject["comment"].toString();

        auto duration = formatObject["duration"].toString().toDouble();

        // skip over temp files
        if (duration <= 0.0)
            continue;

        if (m_metadataList.count() == 0)
        {
            if( metadata_title.isNull() || metadata_title.isEmpty() ){
                metadata_title = "";
            }

            if (metadata_title.isEmpty())
                metadata_title = info.absoluteFilePath();
        }
        else {
            metadata_title = m_metadataList.at(i);
        }

        metadata_title = metadata_title.trimmed();

        // build audio entry from the information
        entry.id = m_IDBeginsFrom++;
        if (readFromCard)
            entry.order = entry.id;
        entry.state = 0;
        entry.path = info.absoluteFilePath();
        entry.metadata.title = metadata_title;
        entry.metadata.album = metadata_album;
        entry.metadata.comment = metadata_comment;
        entry.duration = static_cast<int>(duration);
        entry.flag = m_flag;
        entry_list[entry.id] = entry;
        counter++;

        emit processUpdated(counter * 100 / m_fileList.count());
    }
    emit taskCompleted(entry_list);
    process->deleteLater();
}


