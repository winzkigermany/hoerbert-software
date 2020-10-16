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

#ifndef CDRIPPER_H
#define CDRIPPER_H

#include <QThread>
#include <QObject>
#include <QFileInfoList>

class QProcess;

/**
 * @brief The CDRipper class rips audio cd tracks into file on harddisk concurrently along with GUI thread
 */
class CDRipper : public QThread
{
    Q_OBJECT
public:

    /**
     * @brief CDRipper constructor
     * @param sourceDirPath path to audio cd
     */
    CDRipper(const QFileInfoList &fileInfoList = QFileInfoList());

    /**
     * @brief set the list of file information
     * @param fileInfoList the list of file information
     */
    void setFileInfoList(const QFileInfoList &fileInfoList);

    /**
     * @brief terminates thread after current processing is completed
     */
    void abort();

protected:

    /**
     * @brief rips files on audio cd to harddisk
     */
    void run();

signals:

    /**
     * @brief this signal is emitted when the thread completes ripping all assigned audio tracks
     * @param list the list of audio entries retrieved
     */
    void taskCompleted(const QStringList &pathList);

    /**
     * @brief this signal is emitted when an error occurs while ripping
     * @param errorString the string to represent the error
     */
    void failed(const QString &errorString);

    /**
     * @brief this signal is emitted when each entry is processed
     * @param percentage percentage
     */
    void processUpdated(int percentage);

private:

    QProcess *m_process;
    QFileInfoList m_fileList;
    QString m_tmpDir;
    bool m_abort;

public:
    static int uniqueID;
};

#endif // CDRIPPER_H
