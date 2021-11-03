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

#include "audiobookconverter.h"

#include <QProcess>
#include <QDebug>
#include <QSettings>
#include <QDateTime>
#include <QDir>
#include <QApplication>

#include "define.h"

extern QString FFMPEG_PATH;
extern QString HOERBERT_TEMP_PATH;
extern QStringList PROCESS_ERROR;

AudioBookConverter::AudioBookConverter(const QString &absoluteFilePath)
{
    m_filePath = absoluteFilePath;
    m_isAborted = false;

    m_maxMetadataLength = METADATA_MAX_LENGTH;

    QDir dir(HOERBERT_TEMP_PATH);
    if (!dir.exists())
        dir.mkpath(HOERBERT_TEMP_PATH);
}

QFileInfoList AudioBookConverter::convert(const QString &absoluteFilePath)
{
    QFileInfoList info_list;
    m_filePath = absoluteFilePath.isEmpty() ? m_filePath : absoluteFilePath;
    if (m_filePath.isEmpty() || !QFile::exists(m_filePath))
    {
        qDebug() << "Cannot find the audio book file!";
        emit failed("Cannot find the audio book file!");
        return info_list;
    }

    QStringList arguments;
    arguments.append("-i");
    arguments.append(m_filePath);
    arguments.append("-hide_banner");

    QString output = m_processExecutor.executeCommand(FFMPEG_PATH, arguments).second;

    auto chapters = parseForChapters(output);

    if (m_isAborted)
        return info_list;

    arguments.clear();

    arguments.append("-i");
    arguments.append(m_filePath);
    arguments.append("-ss");
    arguments.append("0"); // index = 3
    arguments.append("-to");
    arguments.append("<END>"); // index = 5
    if( qApp->property("hoerbertModel")==2011 ){
        arguments.append("-acodec");
        arguments.append("pcm_s16le");
        arguments.append("-ar");
        arguments.append("32k");
        arguments.append("-ac");
        arguments.append("1");
    } else {
        arguments.append("-ar");        // sample rate
        arguments.append("44100");
        arguments.append("-ac");        // channels
        arguments.append("2");
        arguments.append("-b:a");        // bit rate
        arguments.append("192k");
    }
    arguments.append("-metadata");
    arguments.append("<METADATA>"); // index = 13
    arguments.append("-y");
    arguments.append("-hide_banner");

    double adjustByDb = getVolumeDifference(m_filePath);
    if( qAbs(adjustByDb)>0.1 && qAbs(adjustByDb)<20.0 )
    {
        arguments.append("-af");
        arguments.append(QString("volume=%1dB").arg( adjustByDb, 0, 'f', 1 ) );
    }

    arguments.append("<OUTPUT_PATH>"); // index = 18 or 16

    int counter = 0;
    int lastArgumentIndex = arguments.count()-1;

    for (const auto &chapter : chapters)
    {
        if (m_isAborted)
            return info_list;

        auto start = chapter.section("<!@#^&>", 0, 0);
        auto end = chapter.section("<!@#^&>", 1, 1);
        auto metadata = chapter.section("<!@#^&>", 2);

        if (start.toDouble() != 0.000000)
            arguments.replace(3, start);
        arguments.replace(5, end);

        metadata.resize(m_maxMetadataLength, ' ');
        arguments.replace(13, QString("title=%1").arg(metadata.trimmed()));

        QString output_path;
        if( qApp->property("hoerbertModel")==2011 ){
            output_path = HOERBERT_TEMP_PATH + QDateTime::currentDateTime().toString("yyyyMMddHHmmss") + QString("-%1").arg(counter) + DESTINATION_FORMAT_WAV;
        } else {
            output_path = HOERBERT_TEMP_PATH + QDateTime::currentDateTime().toString("yyyyMMddHHmmss") + QString("-%1").arg(counter) + DESTINATION_FORMAT_MP3.toLower();
        }
        arguments.replace(lastArgumentIndex, output_path);

        info_list.append(output_path);

        QString output = m_processExecutor.executeCommand(FFMPEG_PATH, arguments).second;

        auto tail = output.right(300);

        if (tail.contains("Error", Qt::CaseSensitive) || tail.contains("Invalid", Qt::CaseSensitive))
        {
            auto error_string = QString("Conversion failed! (%1)\n[Source File]\n%2\n\n%3").arg(counter).arg(m_filePath).arg(output);

            QFile file(output_path);
            if (!file.exists())
            {
                qDebug() << error_string;
                emit failed(error_string);
            }
            else
            {
                if (QFileInfo(output_path).size() == 0)
                {
                    qDebug() << error_string;
                    emit failed(error_string);
                }
            }
        }

        counter++;

        emit processUpdated(100 * counter / chapters.count());
    }

    return info_list;
}

void AudioBookConverter::abort()
{
    m_isAborted = true;
}

QStringList AudioBookConverter::parseForChapters(const QString &output)
{
    QStringList result;
    QStringList chapters = output.section("Chapter #", 1).section("Stream #", 0, 0).split("Chapter #");

    QString start, end, metadata;

    int index = 1;

    for (const auto &chapter : chapters)
    {
        start = "";
        end = "";
        metadata = "";

        bool metadata_detected = false;

        auto lines = chapter.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
        for (auto line : lines)
        {
            line = line.trimmed();

            if (line.contains("start") && line.contains("end"))
            {
                start = line.section("start ", 1).section(", end", 0, 0).trimmed();
                end = line.section(", end ", 1).trimmed();
            }
            else if (line.startsWith("Metadata:"))
            {
                metadata_detected = true;
            }
            else if (line.startsWith("title") && metadata_detected)
            {
                metadata = line.section("title", 1).section(":", 1).trimmed();
            }
        }

        bool start_ok = false;
        bool end_ok = false;

        auto s = start.toDouble(&start_ok);
        auto e = start.toDouble(&end_ok);

        if (!start_ok || !end_ok)
        {
            qDebug() << "Start/End values are not double values";
            emit failed(QString("Start/End values are not double values: %1, %2\n%3\n\n%4").arg(start).arg(end).arg(m_filePath).arg(output));
            return result;
        }

        Q_UNUSED(s)
        Q_UNUSED(e)

        if (!start.isEmpty() && !end.isEmpty() && start_ok && end_ok)
        {
            if (metadata.isEmpty())
            {
                metadata = QFileInfo(m_filePath).fileName() + " Chapter " + QString::number(index);
            }
            result.append(QString("%1<!@#^&>%2<!@#^&>%3").arg(start).arg(end).arg(metadata));
        }
        else
        {
            qDebug() << "Something went wrong while parsing chapter information!";
            emit failed(QString("Something went wrong while parsing chapter information!\n%1").arg(output));
            return result;
        }

        index++;
    }

    return result;
}

// @TODO: the following method is duplicate code. It is found in hoerbertprocessor.cpp, too. Should be de-duplicated.
double AudioBookConverter::getVolumeDifference(const QString &sourceFilePath)
{
    QSettings settings;
    settings.beginGroup("Global");
    QString volumeString = settings.value("volume").toString();
    settings.endGroup();

    bool ok;
    double destinationMaxLevel = volumeString.toDouble(&ok);
    if( !ok )
    {
        destinationMaxLevel = -1.5;   // -1.5 is our fallback. Not to ever ever change.
    }

    // detect maximum volume
    QStringList arguments;
    arguments.append("-i");
    arguments.append(sourceFilePath);
    arguments.append("-af");
    arguments.append("volumedetect");
    arguments.append("-vn");
    arguments.append("-sn");
    arguments.append("-dn");
    arguments.append("-hide_banner");
    arguments.append("-f");
    arguments.append("null");

#ifdef _WIN32
    arguments.append("NUL");
#else
    arguments.append("/dev/null");
#endif

    qDebug() << QString("ffmpeg %1").arg(arguments.join(" "));

    QString output = m_processExecutor.executeCommand(FFMPEG_PATH, arguments).second;

    QStringList lines = output.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);

    // extract the maximum volume from ffmpeg output
    // [Parsed_volumedetect_0 @ 0x7fd4c3604080] n_samples: 89795187
    // [Parsed_volumedetect_0 @ 0x7fd4c3604080] mean_volume: -16.1 dB
    // [Parsed_volumedetect_0 @ 0x7fd4c3604080] max_volume: -0.3 dB
    for (QString line : lines)
    {
        line = line.trimmed();
        if (line.startsWith("[Parsed_volumedetect"))
        {
            volumeString = line.section("max_volume:", 1).trimmed();
            if( !volumeString.isEmpty() )
            {
                volumeString = volumeString.replace("dB","").trimmed();
                break;
            }
        }
    }

    double fileVolume = volumeString.toDouble(&ok);
    double difference = 0.0;    // fallback: don't report any difference.

    if( ok )
    {
        difference = destinationMaxLevel - fileVolume;
    }

    settings.beginGroup("Global");
    bool increaseVolume = settings.value("increaseVolume").toBool();
    settings.endGroup();
    if( !increaseVolume && difference>0.0 )
    {
        difference = 0.0;
    }

    return difference;
}
