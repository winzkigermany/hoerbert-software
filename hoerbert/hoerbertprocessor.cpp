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

#include "hoerbertprocessor.h"

#include <QSettings>
#include <QCoreApplication>
#include <QProcess>
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <stdio.h>

#include "functions.h"
#include "qprocesspriority.h"

extern QString FFMPEG_PATH;
extern QString HOERBERT_TEMP_PATH;
extern QStringList PROCESS_ERROR;

HoerbertProcessor::HoerbertProcessor(const QString &dirPath, int dirNum)
{
    m_dirPath = dirPath;
    m_dirNum = dirNum;

    QSettings settings;
    settings.beginGroup("Global");
    m_audioVolume = settings.value("volume").toString();
    settings.endGroup();
    m_maxMetadataLength = METADATA_MAX_LENGTH;
    m_split3SegmentLength = QString::number(SPLIT_AUDIO_SEGMENT_LENGTH);
    m_silenceNoise = "-50dB";
    m_silenceDetectDuration = "1.5";
    m_splitOnSilenceSegmentLength = "180";

    m_totalEntryCount = 0;
    m_splitOffset = 0;
    m_abort = false;

    qRegisterMetaType <QProcess::ProcessState> ("QProcess::ProcessState");
    qRegisterMetaType <QProcess::ExitStatus> ("QProcess::ExitStatus");
}

HoerbertProcessor::~HoerbertProcessor()
{
    if (m_process) {
        m_process->close();
        m_process->deleteLater();
    }
}

void HoerbertProcessor::addEntryList(ENTRY_LIST_TYPE type, const AudioList &list)
{
    m_entries.insert(type, list);
    m_totalEntryCount += list.count();
}

void HoerbertProcessor::setEntryList(const QMap<ENTRY_LIST_TYPE, AudioList> &list)
{
    m_entries.clear();
    m_entries = list;
    m_totalEntryCount = 0;
    /*for (auto sublist : list)
    {
        m_totalEntryCount += sublist.count();
    }*/

    if (m_entries[REMOVED_ENTRIES].count() > 0)
        m_totalEntryCount += 1;

    if (m_entries[MOVED_ENTRIES].count() > 0)
        m_totalEntryCount += 1;

    m_totalEntryCount += m_entries[ADDED_ENTRIES].count();
    m_totalEntryCount += m_entries[METADATA_CHANGED_ENTRIES].count();
    m_totalEntryCount += m_entries[SPLIT_ENTRIES].count();
}

void HoerbertProcessor::run()
{
    if (m_totalEntryCount == 0) {
        qDebug() << "Nothing to process!";
        return;
    }
    emit processUpdated(0);

    m_process = new QProcessPriority();
    m_process->setPriority(QProcessPriority::NormalPriority);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::errorOccurred, this, &HoerbertProcessor::OnProcessErrorOccurred);
    connect(m_process, &QProcess::readyReadStandardError, this, &HoerbertProcessor::OnProcessReadyReadStandardError);
    //connect(m_process, &QProcess::readyReadStandardOutput, this, &HoerbertProcessor::OnProcessReadyReadStandardOutput);
    connect(m_process, &QProcess::started, this, &HoerbertProcessor::OnProcessStarted);
    connect(m_process, &QProcess::stateChanged, this, &HoerbertProcessor::OnProcessStateChanged);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [] (int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << " - FFmpeg process finished! Exit Code:" << exitCode << ", Exit Status:" << exitStatus;
    });

    m_counter = 0;

    for (const auto& entry : m_entries[REMOVED_ENTRIES])
    {
        if (!removeEntry(entry))
            m_failCounter++;
    }
    if (m_entries[REMOVED_ENTRIES].count() > 0) {
        m_counter++;
        emit processUpdated(m_counter * 100 / m_totalEntryCount);
    }

    /**
     * move moved files
     *
     * Rename all the files in the move list by attaching a certain suffix
     * and then rename again to desired files.
     */
    {
        AudioList moved_list;

        // attach suffixes
        for (auto entry : m_entries[MOVED_ENTRIES])
        {
            if (!QFile::exists(entry.path))
                continue;

            auto num = getFileNameWithoutExtension(entry.path);
            if (num == -1) {
                auto max = getLastNumberInDirectory(m_dirPath);
                if (moveFile(entry.path, tailPath(m_dirPath) + QString::number(max + 1) + DEFAULT_DESTINATION_FORMAT)) {
                    entry.path = tailPath(m_dirPath) + QString::number(max + 1) + DEFAULT_DESTINATION_FORMAT;
                }
            }
            auto tmp_file = attachSuffixToFileName(entry.path, "A");
            if (!tmp_file.isEmpty()) {
                entry.path = tmp_file;
                moved_list.insert(entry.id, entry);
            }
            else {
                qDebug() << "Failed to process an entry while moving files:" << entry.path;
                m_entries[MOVED_ENTRIES].remove(entry.id);
            }
        }

        // move to desired files
        for (const auto& entry : moved_list)
        {
            if (!moveEntry(entry))
                m_failCounter++;
            else {

                /* the path of the entry is updated due to move, thus update path of entry on other lists
                   no need to update added list since it only contains newly added files which were not on card */
                if (m_entries[METADATA_CHANGED_ENTRIES].contains(entry.id))
                {
                    m_entries[METADATA_CHANGED_ENTRIES][entry.id].path = tailPath(m_dirPath) + QString::number(entry.order) + DEFAULT_DESTINATION_FORMAT;
                }

                if (m_entries[SPLIT_ENTRIES].contains(entry.id))
                {
                    m_entries[SPLIT_ENTRIES][entry.id].path = tailPath(m_dirPath) + QString::number(entry.order) + DEFAULT_DESTINATION_FORMAT;
                }
            }

        }
        if (m_entries[MOVED_ENTRIES].count() > 0) {
            m_counter++;
            emit processUpdated(m_counter * 100 / m_totalEntryCount);
        }
    }

    for (const auto& entry : m_entries[ADDED_ENTRIES])
    {
        if (!addEntry(entry))
            m_failCounter++;
        m_counter++;
        emit processUpdated(m_counter * 100 / m_totalEntryCount);
        if (m_abort)
            return;
    }

    for (const auto& entry : m_entries[METADATA_CHANGED_ENTRIES])
    {
        if (!changeEntryMetadata(entry))
            m_failCounter++;
        m_counter++;
        emit processUpdated(m_counter * 100 / m_totalEntryCount);
        if (m_abort)
            return;
    }

    // deal with split entries for the last because we do not know how many sub files it creates
    for (const auto& entry : m_entries[SPLIT_ENTRIES])
    {
        if (!splitEntry(entry))
            m_failCounter++;
        m_counter++;
        emit processUpdated(m_counter * 100 / m_totalEntryCount);
        if (m_abort)
            return;
    }
    if (m_entries[SPLIT_ENTRIES].count() > 0)
        renameSplitFiles(m_dirPath);
    if (m_failCounter > 0)
        emit taskCompleted(m_failCounter, m_totalEntryCount);
}

void HoerbertProcessor::abort()
{
    m_abort = true;
}

int HoerbertProcessor::directoryNumber()
{
    return m_dirNum;
}

bool HoerbertProcessor::removeEntry(const AudioEntry &entry)
{
    QFile file(entry.path);
    if (file.exists())
    {
        return file.remove();
    }
    else
        return true;
}

bool HoerbertProcessor::addEntry(const AudioEntry &entry)
{
    auto sourcePath = entry.path;
    auto destPath = QString("%1/%2%3").arg(m_dirPath/*.replace("//", "/")*/).arg(entry.order).arg(DEFAULT_DESTINATION_FORMAT);

    if (entry.flag != 5)
        return convertToWav(sourcePath, destPath, entry.metadata);
    else
        return createSilenceWav(destPath, entry.duration, entry.metadata);
}

bool HoerbertProcessor::moveEntry(const AudioEntry &entry)
{
    return moveFile(entry.path, m_dirPath + "/" + QString::number(entry.order) + DEFAULT_DESTINATION_FORMAT);
}

bool HoerbertProcessor::splitEntry(const AudioEntry &entry)
{
    if (entry.state == 2) // split per 3 minutes
    {
        if (!splitPer3Mins(entry.path, m_dirPath, entry.order + m_splitOffset, entry.metadata))
            return false;
    }
    else if (entry.state == 1) // split on silence
    {
        if (!splitOnSilence(entry.path, m_dirPath, entry.order + m_splitOffset, entry.metadata))
            return false;
    }

    auto segment_count = countSubfiles(m_dirPath, entry.order + m_splitOffset);

    if (segment_count == 0)
    {
        if (entry.path.startsWith(m_dirPath.replace("//", "/")))
        {
            // the audio cannot be split, it remains as it was
            return true;
        }
        else {
            /* can't be split, no silence section detected.
             just convert & add the file to the card */
            auto sourcePath = entry.path;
            auto destPath = QString("%1/%2%3").arg(m_dirPath/*.replace("//", "/")*/).arg(entry.order + m_splitOffset).arg(DEFAULT_DESTINATION_FORMAT);
            return convertToWav(sourcePath, destPath, entry.metadata);
        }

    }

    if (entry.path.startsWith(m_dirPath.replace("//", "/")))
    {
        qDebug() << "Operation is done on the card. Deleting original file";

        if (remove(entry.path.toLocal8Bit().data()))
        {
            perror("Failed to remove original file");
            emit failed("Failed to remove original file");
            return false;
        }
    }

    if (entry.state == 2)
    {
        auto file_number = entry.order + m_splitOffset;
        QDir dir(m_dirPath);
        dir.setFilter(QDir::Files | QDir::NoSymLinks);
        dir.setNameFilters(QStringList() << "*" + DEFAULT_DESTINATION_FORMAT);
        dir.setSorting(QDir::Name);
        QFileInfoList list;
        for (auto fileInfo : dir.entryInfoList())
        {
            if (fileInfo.fileName().toUpper().section(DEFAULT_DESTINATION_FORMAT, 0, 0).section("-", 0, 0).toInt() == file_number)
            {
                auto suffix_number = fileInfo.fileName().toUpper().section(DEFAULT_DESTINATION_FORMAT, 0, 0).section("-", 1, 1).toInt();
                auto metadata_title = entry.metadata.title;
                auto index = QString(" %1").arg(suffix_number + 1);
                if (metadata_title.toUtf8().size() + index.toUtf8().size() > m_maxMetadataLength)
                {
                    metadata_title.resize(m_maxMetadataLength - index.toUtf8().size(), ' ');
                }
                if (!changeMetaData(fileInfo.absoluteFilePath(), entry.metadata, index))
                {
                    qDebug() << "Failed attaching index to metadata!" << fileInfo.absoluteFilePath();
                    emit failed("Failed attaching index to metadata!" + fileInfo.absoluteFilePath());
                    return false;
                }
            }
        }
    }

    return true;
}

bool HoerbertProcessor::changeEntryMetadata(const AudioEntry &entry)
{
    return changeMetaData(entry.path, entry.metadata);
}

bool HoerbertProcessor::convertToWav(const QString &sourceFilePath, const QString &destFilePath, const MetaData &metadata)
{
    QStringList arguments;
    arguments.append("-i");
    arguments.append(sourceFilePath);
    arguments.append("-acodec");
    arguments.append("pcm_s16le");
    arguments.append("-ar");
    arguments.append("32k");
    arguments.append("-ac");
    arguments.append("1");
    arguments.append("-metadata");

    auto arg_metadata_title = metadata.title;
    arg_metadata_title.resize(m_maxMetadataLength, ' ');
    arguments.append(QString("title=%1").arg(arg_metadata_title.trimmed()));

    arguments.append("-metadata");
    arguments.append(QString("album=%1").arg(metadata.album));

    arguments.append("-metadata");
    arguments.append(QString("comment=%1").arg(sourceFilePath));

    arguments.append("-filter:a");

    auto arg_volume = QString("volume=%1dB").arg(m_audioVolume);
    arguments.append(arg_volume);

    arguments.append("-y");
    arguments.append("-hide_banner");

    arguments.append(destFilePath);

    m_process->start(FFMPEG_PATH, arguments);

    return m_process->waitForFinished(PROCESS_LIFECYCLE);
}

bool HoerbertProcessor::splitPer3Mins(const QString &sourceFilePath, const QString &destDir, int outfileName, const MetaData &metadata)
{
    QStringList arguments;
    arguments.append("-i");
    arguments.append(sourceFilePath);
    arguments.append("-acodec");
    arguments.append("pcm_s16le");
    arguments.append("-ar");
    arguments.append("32k");
    arguments.append("-ac");
    arguments.append("1");

    arguments.append("-metadata");
    arguments.append(QString("title=%1").arg(metadata.title));

//    auto arg_metadata_title = metadata;
//    arg_metadata_title.resize(m_maxMetadataLength, ' ');
//    arguments.append(QString("title=%1").arg(arg_metadata_title.trimmed()));

    arguments.append("-metadata");
    arguments.append(QString("album=%1").arg(metadata.album));

    arguments.append("-metadata");
    arguments.append(QString("comment=%1").arg(metadata.comment));

    arguments.append("-f");
    arguments.append("segment");
    arguments.append("-segment_time");
    arguments.append(m_split3SegmentLength);

    arguments.append("-filter:a");

    auto arg_volume = QString("volume=%1dB").arg(m_audioVolume);
    arguments.append(arg_volume);

    arguments.append(tailPath(destDir) + QString::number(outfileName) + "-%01d" + DEFAULT_DESTINATION_FORMAT);
    arguments.append("-y");

    m_process->start(FFMPEG_PATH, arguments);

    return m_process->waitForFinished(PROCESS_LIFECYCLE);
}

bool HoerbertProcessor::splitOnSilence(const QString &sourceFilePath, const QString &destDir, int outfileName, const MetaData &metadata)
{
    QStringList arguments;

    // detect silence part
    arguments.append("-i");
    arguments.append(sourceFilePath);
    arguments.append("-af");

    auto arg_audio_volume = "silencedetect=n=" + m_silenceNoise + ":d=" + m_silenceDetectDuration;
    arguments.append(arg_audio_volume);
    arguments.append("-hide_banner");
    arguments.append("-f");
    arguments.append("null");
    arguments.append("-");

    m_process->start(FFMPEG_PATH, arguments);
    if (!m_process->waitForFinished(PROCESS_LIFECYCLE))
    {
        qDebug() << "Failed to execute ffmpeg command";
        emit failed("Failed to execute ffmpeg command");
        return false;
    }

    QString result_output = m_process->readAllStandardOutput();
    m_process->close();

    QStringList lines;
    QString duration_string;

    for (auto line : result_output.split(QRegExp("[\r\n]"), QString::SkipEmptyParts))
    {
        line = line.trimmed();
        if (line.startsWith("[silencedetect"))
            lines.append(line.section("]", 1).trimmed());
        if (line.startsWith("Duration:", Qt::CaseSensitive))
            duration_string = line.section("Duration:", 1).section(",", 0, 0).trimmed();
    }

    // now split on silence
    double segment_length = m_splitOnSilenceSegmentLength.toDouble();
    arguments.clear();

    arguments.append("-i");
    arguments.append(sourceFilePath);

    double total_length = duration_string.section(":", 0, 0).toDouble() * 3600 + duration_string.section(":", 1, 1).toDouble() * 60
            + duration_string.section(":", 2).toDouble();

    /**
     * [silencedetect @ 00000257cfefc880] silence_start: 262.275
     * [silencedetect @ 00000257cfefc880] silence_end: 267.782 | silence_duration: 5.50717
     */

    QMap<int, QStringList> segment_list;

    QStringList first_segment = QStringList() << "0";
    segment_list.insert(0, first_segment);

    double silence_start, silence_end;
    int chunk_count = 0;
    for (int i = 0; i < lines.length(); i = i + 2)
    {
        if (!lines[i].contains("silence_start:")) {
            qDebug() << "Failed to get silence start point";
            break;
        }
        silence_start = lines[i].section(":", 1, 1).trimmed().toDouble();
        //qDebug() << silence_start << segment_length << total_length << duration_string;
        if (silence_start > segment_length)
        {
            silence_end = lines[i+1].section("silence_end:", 1, 1).section("|", 0, 0).trimmed().toDouble();
            if (total_length - silence_end > 1)
            {
                if (chunk_count == 0)
                {
                    arguments.append("-ss");
                    arguments.append("0");
                    arguments.append("-to");
                    arguments.append(QString::number(silence_start));
                }

                chunk_count++;
                segment_length = silence_end + segment_length;
                segment_list[chunk_count - 1].pop_back();
                segment_list[chunk_count - 1].push_back(QString::number(silence_start));
                segment_list[chunk_count].push_back(QString::number(silence_end));

            }
            segment_list[chunk_count].push_back(duration_string);
        }
    }

    if (chunk_count == 0) {
        qDebug() << "No silence detected. No need to split!";
        emit noSilenceDetected();
        return true;
    }

    arguments.append("-acodec");
    arguments.append("pcm_s16le");
    arguments.append("-ar");
    arguments.append("32k");
    arguments.append("-ac");
    arguments.append("1");
    arguments.append("-metadata");

    QString metadata_ = metadata.title;
    const QString attached_suffix = " 1";
    int attached_length = attached_suffix.toUtf8().size();

    if (metadata.title.toUtf8().size() + attached_length > m_maxMetadataLength)
    {
        metadata_.resize(m_maxMetadataLength - attached_length, ' ');
    }

    arguments.append(QString("title=%1").arg(metadata_.trimmed() + attached_suffix));

    arguments.append("-metadata");
    arguments.append(QString("album=%1").arg(metadata.album));

    arguments.append("-metadata");
    arguments.append(QString("comment=%1").arg(metadata.comment));

    arguments.append("-filter:a");

    QString arg_volume = QString("volume=%1dB").arg(m_audioVolume);
    arguments.append(arg_volume);

    QString output_filename = destDir + "/" + QString::number(outfileName) + "-0" + DEFAULT_DESTINATION_FORMAT;
    arguments.append(output_filename);
    arguments.append("-y");

    m_process->start(FFMPEG_PATH, arguments);
    bool success = m_process->waitForFinished(PROCESS_LIFECYCLE);

    if (!success)
        return false;

    for (int i = 0; i < chunk_count; i++)
    {
        if (!segment_list.keys().contains(i))
            break;

        arguments[3] = segment_list[i + 1].at(0);
        arguments[5] = segment_list[i + 1].at(1);

        arguments[13] = QString("title=%1").arg(metadata_ + " " + QString::number(i + 2));

        output_filename = destDir + "/" + QString::number(outfileName) + "-" + QString::number(i + 1) + DEFAULT_DESTINATION_FORMAT;
        arguments[20] = output_filename;
        m_process->start(FFMPEG_PATH, arguments);
        success = m_process->waitForFinished(PROCESS_LIFECYCLE);
        m_process->close();
        if (!success)
            return false;
    }

    m_process->close();

    return true;
}

bool HoerbertProcessor::createSilenceWav(const QString &destFilePath, int duration, const MetaData &metadata)
{
    QStringList arguments;
    arguments.append("-f");
    arguments.append("lavfi");
    arguments.append("-i");
    arguments.append("anullsrc=r=32000:cl=mono");
    arguments.append("-t");
    arguments.append(QString::number(duration));
    arguments.append("-acodec");
    arguments.append("pcm_s16le");

    arguments.append("-metadata");
    arguments.append(QString("title=%1").arg(metadata.title));

//    QString arg_metadata_title = metadata;
//    arg_metadata_title.resize(m_maxMetadataLength, ' ');
//    arguments.append(QString("title=%1").arg(arg_metadata_title.trimmed()));

    arguments.append("-metadata");
    arguments.append(QString("album=%1").arg(metadata.album));

    arguments.append("-metadata");
    arguments.append(QString("comment=%1").arg(metadata.comment));

    arguments.append(destFilePath);
    arguments.append("-y");

    m_process->start(FFMPEG_PATH, arguments);
    qDebug() << arguments;
    return m_process->waitForFinished(PROCESS_LIFECYCLE);
}

bool HoerbertProcessor::changeMetaData(const QString &sourceFilePath, const MetaData &metadata, const QString &suffix)
{
    QStringList arguments;
    arguments.append("-i");
    arguments.append(sourceFilePath);
    arguments.append("-metadata");

//    QString arg_metadata_title = metadata.title;
//    arg_metadata_title.resize(m_maxMetadataLength, ' ');
//    arguments.append(QString("title=%1").arg(arg_metadata_title.trimmed()));
    arguments.append(QString("title=%1").arg(suffix.length() > 0 ? metadata.title + " " + suffix : metadata.title));

    arguments.append("-metadata");
    arguments.append(QString("album=%1").arg(metadata.album));

    arguments.append("-metadata");
    arguments.append(QString("comment=%1").arg(metadata.comment));

    arguments.append("-codec");
    arguments.append("copy");

    QString tmp_file = tailPath(HOERBERT_TEMP_PATH) + "temp" + DEFAULT_DESTINATION_FORMAT;

    arguments.append(tmp_file);
    arguments.append("-y");

    m_process->start(FFMPEG_PATH, arguments);
    bool success = m_process->waitForFinished(PROCESS_LIFECYCLE);

    qDebug() << sourceFilePath << tmp_file;

    if (success)
    {
        if (QFile::exists(sourceFilePath))
        {
            if (QFile::remove(sourceFilePath))
            {
                return QFile::rename(tmp_file, sourceFilePath);
            }
        }
    }
    return false;
}

bool HoerbertProcessor::renameSplitFiles(const QString &destDir)
{
    QDir dir(destDir);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setNameFilters(QStringList() << "*" + DEFAULT_DESTINATION_FORMAT);
    dir.setSorting(QDir::Name);

    QFileInfoList file_info_list = dir.entryInfoList();
    std::sort(file_info_list.begin(), file_info_list.end(), sortByNumber);
    for (int i = file_info_list.count() - 1; i >= 0 ; i--)
    {
        qDebug() << file_info_list[i].absoluteFilePath();
        if (file_info_list[i].fileName().toUpper().section(DEFAULT_DESTINATION_FORMAT.toUpper(), 0, 0).toInt() != i)
        {
            auto destPath = tailPath(file_info_list[i].absolutePath()) + QString("%1%2").arg(i).arg(DEFAULT_DESTINATION_FORMAT);

            if (!moveFile(file_info_list[i].absoluteFilePath(), destPath))
            {
                qDebug() << "Failed to move split file" << file_info_list[i].absoluteFilePath() << "to" << destPath;
                emit failed("Failed to move split file " + file_info_list[i].absoluteFilePath() + " to " + destPath);
                return false;
            }
        }
    }
    return true;
}

void HoerbertProcessor::OnProcessStarted()
{
    qDebug() << " + FFmpeg process started!";
}

void HoerbertProcessor::OnProcessErrorOccurred(QProcess::ProcessError error)
{
    qDebug() << " - Process Error!";
    qDebug() << error;
    emit failed("hoerbertThread:" + PROCESS_ERROR.at(error) + "\n" + m_process->readAllStandardError() + "\n" + m_process->readAllStandardOutput());
}

void HoerbertProcessor::OnProcessReadyReadStandardError()
{
    //emit failed(m_process->readAllStandardError());
}

void HoerbertProcessor::OnProcessReadyReadStandardOutput()
{
    //qDebug() << m_process->readAllStandardOutput();
}

void HoerbertProcessor::OnProcessStateChanged(QProcess::ProcessState newState)
{
    qDebug() << "Process state changed to" << newState;
}

