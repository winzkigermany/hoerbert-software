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
#include <QEventLoop>

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

    QSettings settings;
    settings.beginGroup("Global");
    m_audioVolume = settings.value("volume").toString();
    settings.endGroup();

    QStringList arguments;
    arguments.append("-i");
    arguments.append(m_filePath);
    arguments.append("-hide_banner");

    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);

    connect(&process, &QProcess::errorOccurred, this, [this, &process] (QProcess::ProcessError error) {
        qDebug() << "- AudioBookConverter: Error!";
        emit failed(QString("QProcess error: %1\n%2").arg(PROCESS_ERROR.at(error)).arg(QString(process.readAllStandardOutput())));
    });


    bool returnValue = false;
    QEventLoop loop;
    connect(&process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [&returnValue, &loop](int result, QProcess::ExitStatus x){
        Q_UNUSED(x)
        returnValue = (result==0);      // keep in mind that this call will return a non-0 value, because we're just getting info. More parameters are missing!
        loop.quit();
    });
    process.start(FFMPEG_PATH, arguments);
    if (!process.waitForStarted())
    {
        qDebug() << "Failed executing ffmpeg!";
        emit failed( QString("Failed executing ffmpeg: [%1]").arg(arguments.join(",")) );
        process.disconnect();
        return info_list;
    }
    loop.exec();
    process.disconnect();

    QString output = process.readAllStandardOutput();
    process.close();

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
    arguments.append("-acodec");
    arguments.append("pcm_s16le");
    arguments.append("-ar");
    arguments.append("32k");
    arguments.append("-ac");
    arguments.append("1");
    arguments.append("-metadata");
    arguments.append("<METADATA>"); // index = 13

    double adjustByDb = getVolumeDifference(absoluteFilePath);
    if( qAbs(adjustByDb)>0.1 && qAbs(adjustByDb)<30.0 )
    {
        arguments.append("-af");
        arguments.append(QString("volume=%1dB").arg( adjustByDb, 0, 'f', 1 ) );
    }

    arguments.append("<OUTPUT_PATH>"); // index = 16
    arguments.append("-y");
    arguments.append("-hide_banner");

    int counter = 0;

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

        auto output_path = HOERBERT_TEMP_PATH + QDateTime::currentDateTime().toString("yyyyMMddHHmmss") + QString("-%1").arg(counter) + DEFAULT_DESTINATION_FORMAT;
        arguments.replace(16, output_path);

        info_list.append(output_path);

        bool returnValue = false;
        QEventLoop loop;
        connect(&process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [&returnValue, &loop](int result){
            returnValue = (result==0);
            loop.quit();
        });
        process.start(FFMPEG_PATH, arguments);
        if (!process.waitForStarted())
        {
            qDebug() << "Failed executing ffmpeg!";
            emit failed("Failed executing ffmpeg!");
            process.disconnect();
            return info_list;
        }
        loop.exec();
        process.disconnect();

        output = process.readAllStandardOutput();
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
    auto chapters = output.section("Chapter #", 1).section("Stream #", 0, 0).split("Chapter #");

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
    arguments.append("-f");
    arguments.append("null");

#ifdef Q_OS_WIN
    arguments.append("NUL");
#else
    arguments.append("/dev/null");
#endif

    qDebug() << QString("ffmpeg %1").arg(arguments.join(" "));
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    bool returnValue = false;
    {
        QEventLoop loop;
        connect(&process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [&returnValue, &loop](int result){
            returnValue = (result==0);
            loop.quit();
        });
        process.start(FFMPEG_PATH, arguments);
        loop.exec();
        process.disconnect();
    }

    if (!returnValue)
    {
        qDebug() << "Failed to execute ffmpeg command";
        emit failed("Failed to execute ffmpeg command");
        return false;
    }

    QString result_output = process.readAllStandardOutput();
    process.close();
    process.disconnect();

    QStringList lines = result_output.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);

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

    return difference;
}
