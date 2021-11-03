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

#include "backupmanager.h"

#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QApplication>

#include "define.h"
#include "functions.h"

extern QString FFMPEG_PATH;
extern QStringList PROCESS_ERROR;

BackupManager::BackupManager(const QString &sourcePath, const QString &destpath, bool isBackup, bool restoreOnly)
    : QObject ()
{
    m_sourcePath = sourcePath;
    m_destPath = tailPath(destpath);

    m_isBackup = isBackup;
    m_abort = false;
    m_restoreOnly = restoreOnly;

    m_totalEntryCount = 0;

    qRegisterMetaType <QProcess::ProcessState> ("QProcess::ProcessState");
    qRegisterMetaType <QProcess::ExitStatus> ("QProcess::ExitStatus");
}

BackupManager::~BackupManager()
{
    if (m_process)
        m_process->deleteLater();
}
void BackupManager::setSourcePath(const QString &path)
{
    m_sourcePath = path;
}

void BackupManager::setDestPath(const QString &path)
{
    m_destPath = path;
}

void BackupManager::setBackupOrRestore(bool isBackup)
{
    m_isBackup = isBackup;
}

void BackupManager::addStamp(const QString &key, const QString &value)
{
    m_stamp << QString("%1!@#$%^&*%2").arg(key).arg(value);
}

void BackupManager::process()
{
    if (m_sourcePath.isEmpty() || m_destPath.isEmpty() || m_destPath.compare("/") == 0)
    {
        qDebug() << "Must specify source & destination directories to get the job done!";
        emit failed("Source or destination directory is not specified");
        return;
    }

    /*
     * This is the case when user selected restore and then restore backup only which means
     * delete all files on card before restoring backup
     * */
    if (m_restoreOnly)
    {
        QDir dir(m_destPath);

        // this should never happen
        if (!dir.exists())
        {
            qDebug() << "Destination directory does not exist";
            emit failed("Destination directory does not exist! " + m_destPath);
            return;
        }

        dir.refresh();

        for (int i = 0; i < 9; i++)
        {
            QDir sub_dir(m_destPath + QString::number(i));

            if (sub_dir.exists())
            {
                if( qApp->property("hoerbertModel")==2011 ){
                    sub_dir.setNameFilters(QStringList() << "*" + DESTINATION_FORMAT_WAV);
                } else {
                    sub_dir.setNameFilters(QStringList() << "*" + DESTINATION_FORMAT_WAV << "*" + DESTINATION_FORMAT_MP3 << "*" + DESTINATION_FORMAT_URL);      // don't use * here. For safety.
                }
                sub_dir.setFilter(QDir::Files);
                for (const auto& file : sub_dir.entryList())
                {
                    sub_dir.refresh();

                    if (!sub_dir.remove(file))
                    {
                        qDebug() << "Failed deleting file on card";
                        emit failed("Failed deleting file on card");
                        return;
                    }
                    else
                    {
                        qDebug() << "Removed file" << file;
                    }
                }
            }
        }
    }

    // create process instance and handle signals
    m_process = new QProcess();
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, &QProcess::errorOccurred, this, &BackupManager::OnProcessErrorOccurred);
    connect(m_process, &QProcess::readyReadStandardError, this, &BackupManager::OnProcessReadyReadStandardError);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &BackupManager::OnProcessReadyReadStandardOutput);
    connect(m_process, &QProcess::started, this, &BackupManager::OnProcessStarted);
    connect(m_process, &QProcess::stateChanged, this, &BackupManager::OnProcessStateChanged);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [] (int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << " - FFmpeg process finished! Exit Code:" << exitCode << ", Exit Status:" << exitStatus;
    });

    int counter = 0;

    QMap<int, QFileInfoList> file_list_map;
    QDir source_dir(m_sourcePath);

    if (source_dir.exists())
    {

        QDir dest_dir(m_destPath);
        if (!dest_dir.exists())
        {
            qDebug() << "Destination directory does not exist!" << m_destPath;
            emit failed("Destination directory does not exist! " + m_destPath);
            return;
        }

        for (int i = 0; i < 9; i++)
        {
            QString sub_dir = QString();

            sub_dir = tailPath(tailPath(m_sourcePath) + QString::number(i));

            QDir dir(sub_dir);
            if (!dir.exists())
                continue;

            dir.setFilter(QDir::Files | QDir::NoSymLinks);
            if (m_isBackup){
                if( qApp->property("hoerbertModel")==2011 ){
                    dir.setNameFilters(QStringList() << "*" + DESTINATION_FORMAT_WAV);
                } else {
                    dir.setNameFilters(QStringList() << "*.*" );
                }
            } else {
                if( qApp->property("hoerbertModel")==2011 ){
                    dir.setNameFilters(QStringList() << "*" + DESTINATION_FORMAT_FLAC.toLower());
                } else {
                    dir.setNameFilters(QStringList() << "*.*" );
                }
            }

            QFileInfoList file_info_list = dir.entryInfoList();

            std::sort(file_info_list.begin(), file_info_list.end(), sortByNumber);

            file_list_map.insert(i, file_info_list);
            m_totalEntryCount += file_info_list.count();

            if (!dest_dir.exists(QString::number(i)) && !dest_dir.mkdir(QString::number(i)))
            {
                qDebug() << "Failed to make sub-directory in destination directory." << i;
                continue;
            }
        }
    }
    else
    {
        qDebug() << "Source directory does not exist!" << m_sourcePath;
        emit failed("Source directory does not exist! " + m_sourcePath);
        return;
    }

    QDir dest_dir(m_destPath);
    // get list of non-hoerbert files, directories
    if (m_isBackup)
    {
        emit processUpdated(0);

        for (int i = 0; i < 9; i++)
        {
            m_excludeList.append(tailPath(source_dir.absolutePath()) + QString::number(i));
        }

        QFileInfoList all_file_info_list;
        getFileInfoList(m_sourcePath, &all_file_info_list);

        m_totalEntryCount += all_file_info_list.count();

        for (const auto& info : all_file_info_list)
        {
            QString dest_absolute_file_path = tailPath(dest_dir.absolutePath()) + info.absoluteFilePath().remove(source_dir.absolutePath());

            QFileInfo fi(dest_absolute_file_path);
            dest_dir.mkpath(fi.absolutePath());

            qDebug() << "Copy:" << info.absoluteFilePath() << fi.absoluteFilePath();

            if (QFile::copy(info.absoluteFilePath(), fi.absoluteFilePath()))
            {
                qDebug() << info.absoluteFilePath() << " -> " << fi.absoluteFilePath();
                emit processUpdated(++counter * 100 / m_totalEntryCount);
            }
            else
            {
                auto error_str = "Failed copying files: " + info.absoluteFilePath() + " -> " + fi.absoluteFilePath();
                qDebug() << error_str;
                emit failed(error_str);
            }

            if (m_abort)
                return;
        }
    }

    if (m_totalEntryCount == 0)
    {
        qDebug() << "No files to backup/restore!";
        emit failed("No files to backup/restore!");
        return;
    }

    for (int i = 0; i < 9; i++)
    {
        if (!file_list_map.keys().contains(i))
            continue;

        QString dest_dir_path = tailPath(tailPath(m_destPath) + QString::number(i));

        QDir dir( dest_dir_path );
        dir.setFilter( QDir::AllEntries | QDir::NoDotAndDotDot );   // NOT case sensitive by default
        if( qApp->property("hoerbertModel")==2011 ){
            dir.setNameFilters( QStringList()<<"*.wav" );
        } else {
            dir.setNameFilters( QStringList()<<"*" );
        }
        int total_files_offset = dir.count();

        auto file_list = file_list_map.value(i);
        for (const auto& file_info : file_list)
        {
            bool success = false;

            if (m_isBackup)
            {
                if( qApp->property("hoerbertModel")==2011 ){
                    success = convertWav2Flac(file_info.filePath(), dest_dir_path + file_info.fileName().replace( DESTINATION_FORMAT_WAV, DESTINATION_FORMAT_FLAC, Qt::CaseInsensitive));
                } else {
                    success = QFile::copy(file_info.filePath(), dest_dir_path + file_info.fileName() );
                }
            }
            else
            {
                QString destinationFileNumber = file_info.fileName().replace(DESTINATION_FORMAT_FLAC, "", Qt::CaseInsensitive);

                bool ok;
                int decimalFileNumber = destinationFileNumber.toInt(&ok, 10);       // dec == 0, ok == false
                if( ok )
                {
                    decimalFileNumber += total_files_offset;
                    destinationFileNumber = QString::number(decimalFileNumber);
                }

                if( qApp->property("hoerbertModel")==2011 ){
                    QString destinationFileName;
                    destinationFileName = destinationFileNumber+DESTINATION_FORMAT_WAV;
                    success = convertFlac2Audio(file_info.filePath(), dest_dir_path + destinationFileName);
                } else {
                    success = QFile::copy(file_info.filePath(), dest_dir_path + file_info.fileName());
                }
            }

            if (success)
            {
                counter++;
                emit processUpdated(counter * 100 / m_totalEntryCount);
            }

            if (m_abort)
                return;
        }
    }

    // restore hoerbert setting files at the end of restoring
    if (m_isBackup)
    {
        QString xml_string_header = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                             "<hoerbert>\n";

        QString xml_string_tail = "</hoerbert>\n";

        QFile backup_info_file(tailPath(dest_dir.absolutePath()) + BACKUP_INFO_FILE);
        if (backup_info_file.exists())
        {
            backup_info_file.remove();
        }
        if (backup_info_file.open(QIODevice::WriteOnly))
        {
            QString xml_content = "";
            QString tag, value = "";
            for (const auto& line : m_stamp)
            {
                tag = line.section("!@#$%^&*", 0, 0);
                value = line.section("!@#$%^&*", 1);
                xml_content += "<" + tag + ">" + value + "</" + tag + ">\n";
            }

            backup_info_file.write(QString(xml_string_header + xml_content + xml_string_tail).toUtf8());
            backup_info_file.close();
        }
        else
        {
            qDebug() << "Failed writing backup file";
            emit failed("Failed writing backup file");
        }

        QFile card_info_file(tailPath(source_dir.absolutePath()) + CARD_INFO_FILE);
        if (card_info_file.exists())
        {
            card_info_file.remove();
        }
        if (card_info_file.open(QIODevice::WriteOnly))
        {
            QString xml_content = "";
            QString tag, value = "";
            for (const auto& line : m_stamp)
            {
                tag = line.section("!@#$%^&*", 0, 0);
                value = line.section("!@#$%^&*", 1);
                xml_content += "<" + tag + ">" + value + "</" + tag + ">\n";
            }

            card_info_file.write(QString(xml_string_header + xml_content + xml_string_tail).toUtf8());
            card_info_file.close();
        }
        else
        {
            qDebug() << "Failed writing backup file";
            emit failed("Failed writing backup file");
        }
    }
    else
    {
        auto hoerbert_xml = tailPath(source_dir.absolutePath()) + HOERBERT_XML;
        if (QFile::exists(hoerbert_xml))
        {
            if (QFile(tailPath(dest_dir.absolutePath()) + HOERBERT_XML).exists())
                QFile::remove(tailPath(dest_dir.absolutePath()) + HOERBERT_XML);

            if (!QFile::copy(hoerbert_xml, tailPath(dest_dir.absolutePath()) + HOERBERT_XML))
            {
                auto error_str = "Failed copying hoerbert setting file: " + hoerbert_xml;
                qDebug() << error_str << tailPath(dest_dir.absolutePath()) + HOERBERT_XML;
                emit failed(error_str);
            }
        }

        auto hoerbert_xml_backup = tailPath(source_dir.absolutePath()) + HOERBERT_XML_BACKUP;
        if (QFile::exists(hoerbert_xml))
        {
            if (QFile(tailPath(dest_dir.absolutePath()) + HOERBERT_XML_BACKUP).exists())
                QFile::remove(tailPath(dest_dir.absolutePath()) + HOERBERT_XML_BACKUP);

            if (!QFile::copy(hoerbert_xml, tailPath(dest_dir.absolutePath()) + HOERBERT_XML_BACKUP))
            {
                auto error_str = "Failed copying hoerbert backup information file: " + hoerbert_xml_backup;
                qDebug() << error_str << tailPath(dest_dir.absolutePath()) + HOERBERT_XML_BACKUP;
                emit failed(error_str);
            }
        }
    }
}

void BackupManager::abort()
{
    m_abort = true;
}

bool BackupManager::convertWav2Flac(const QString &sourcePath, const QString destPath)
{
    qDebug() << sourcePath << " -> " << destPath;
    QStringList arguments;
    arguments.append("-i");
    arguments.append(sourcePath);
    arguments.append("-c:a");
    arguments.append("flac");
    arguments.append("-y");
    arguments.append("-hide_banner");
    arguments.append(destPath);

    std::pair<int, QString> output = m_processExecutor.executeCommand(FFMPEG_PATH, arguments);

    return output.first==0;
}



bool BackupManager::convertFlac2Audio(const QString &sourcePath, const QString destPath)
{
    qDebug() << sourcePath << " -> " << destPath;
    QStringList arguments;
    arguments.append("-i");
    arguments.append(sourcePath);
    arguments.append("-y");
    arguments.append("-hide_banner");
    arguments.append("-v");
    arguments.append("quiet");
    arguments.append(destPath);

    std::pair<int, QString> output = m_processExecutor.executeCommand(FFMPEG_PATH, arguments);

    return output.first==0;
}

void BackupManager::getFileInfoList(const QString &dirPath, QFileInfoList *fileList)
{
    QDir dir(dirPath);

    if (!dir.exists())
        return;

    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::System);
    dir.setSorting(QDir::Name);

    QFileInfoList list = dir.entryInfoList();

    for (int i = 0; i < list.size(); i++)
    {
        QFileInfo fileInfo = list.at(i);
        if (fileInfo.isDir())
        {
            if( qApp->property("hoerbertModel")==2011 ){
                // hoerbert music files need to be converted, thus, should be excluded copy list
                if (m_excludeList.contains(fileInfo.absoluteFilePath()))
                    continue;
            }

            // recursively get file info list in the directory
            getFileInfoList(fileInfo.absoluteFilePath(), fileList);
        }
        else if (fileInfo.isFile())
        {
            fileList->append(fileInfo);
        }
        else
            qDebug() << "getFileInfoList: Invalid type!";
    }
}

void BackupManager::OnProcessStarted()
{
    qDebug() << " + FFmpeg process started!";
}

void BackupManager::OnProcessErrorOccurred(QProcess::ProcessError error)
{
    qDebug() << " - Process Error!";
    qDebug() << error;
    emit failed("BackupThread:" + PROCESS_ERROR.at(error) + "\n" + m_process->readAllStandardError() + "\n" + m_process->readAllStandardOutput() );
}

void BackupManager::OnProcessReadyReadStandardError()
{
    qDebug() << " - Process Error!";
    qDebug() << m_process->readAllStandardError();
}

void BackupManager::OnProcessReadyReadStandardOutput()
{
//    qDebug() << m_process->readAllStandardOutput();
}

void BackupManager::OnProcessStateChanged(QProcess::ProcessState newState)
{
    Q_UNUSED(newState)
//    qDebug() << " + Process state changed to" << newState;
}
