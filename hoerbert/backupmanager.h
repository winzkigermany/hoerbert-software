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

#ifndef BACKUPMANAGER_H
#define BACKUPMANAGER_H

#include <QThread>
#include <QProcess>
#include <QFileInfoList>

#include "processexecutor.h"

/**
 * @brief The BackupManager class handles backup/restore memory card data
 */
class BackupManager : public QObject
{
    Q_OBJECT
public:

    /**
     * @brief BackupManager constructor
     * @param sourcePath absolute path to the directory it wants to back up
     * @param destPath absolute path to the directory where the backup files will reside
     * @param isBackup, determines whether this thread will do backup job(wav to flac) or restore job(flac to wav)
     */
    BackupManager(const QString &sourcePath = QString(), const QString &destpath = QString(), bool isBackup = true, bool restoreOnly = false);

    /**
     * @brief BackupManager destructor
     */
    ~BackupManager();

    /**
     * @brief set source directory path which contains files to be processed
     * @param path absolute directory path
     */
    void setSourcePath(const QString &path);

    /**
     * @brief set destination directory path which will contain processed files
     * @param path absolute directory path
     */
    void setDestPath(const QString &path);

    /**
     * @brief select this thread job type - backup / restore
     * @param isBackup true: backup(wav to flac), false: restore(flac to wav)
     */
    void setBackupOrRestore(bool isBackup);

    /**
     * @brief add stamp data, these data will be written to an exta file while backup
     * @param key key of json
     * @param value value of json
     */
    void addStamp(const QString &key, const QString &value);

    /**
     * @brief process files
     */
    void process();

    /**
     * @brief terminates the thread after current processing is completed
     */
    void abort();

signals:

    /**
     * @brief this signal is emitted every file is processed
     * @param percentage
     */
    void processUpdated(int percentage);

    /**
     * @brief this signal is emitted once the thread fails
     * @param statusCode
     * @param errorString
     */
    void failed(const QString &errorString);

private slots:

    void OnProcessStarted();
    void OnProcessErrorOccurred(QProcess::ProcessError error);
    void OnProcessReadyReadStandardError();
    void OnProcessReadyReadStandardOutput();
    void OnProcessStateChanged(QProcess::ProcessState newState);

private:

    /**
     * @brief convertWav2Flac converts wav to flac using ffmpeg
     * @param sourcePath absolute path to the source file
     * @param destPath absolute path to destination file
     * @return true if success, false otherwise
     */
    bool convertWav2Flac(const QString &sourcePath, const QString destPath);

    bool convertMp32Flac(const QString &sourcePath, const QString destPath);

    /**
     * @brief convertFlac2Wav converts flac to wav using ffmpeg
     * @param sourcePath absolute path to the source file
     * @param destPath absolute path to destination file
     * @return true if success, false otherwise
     */
    bool convertFlac2Audio(const QString &sourcePath, const QString destPath);

    /**
     * @brief recursively get all files' info in dirPath, excluding hoerbert music files
     * @param dirPath absolute path to target directory
     * @param fileList list to contain all the info
     */
    void getFileInfoList(const QString &dirPath, QFileInfoList *fileList);

    QProcess *m_process;

    QString m_sourcePath;
    QString m_destPath;
    bool m_isBackup;
    bool m_abort;

    // determines whether to overwrite or only copy new(none-existing on card) files
    bool m_restoreOnly;

    int m_totalEntryCount;

    // for stamp file
    QString m_byWho;
    QString m_driveLabel;

    QStringList m_stamp;

    QStringList m_excludeList;
    ProcessExecutor m_processExecutor;

};

#endif // BACKUPMANAGER_H
