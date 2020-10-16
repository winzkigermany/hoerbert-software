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

#ifndef AUDIOINFOTHREAD_H
#define AUDIOINFOTHREAD_H

#include <QThread>
#include <QFileInfoList>

#include "define.h"

/**
 * @brief The AudioInfoThread class reads audio info from audio files using ffprobe
 */
class AudioInfoThread : public QThread
{
    Q_OBJECT
public:

    /**
     * @brief AudioInfoThread contructor
     */
    AudioInfoThread();

    /**
     * @brief AudioInfoThread constructor
     * @param fileList the file info list to work on
     * @param idBeginsFrom
     * @param flag default flag for entries
     */
    AudioInfoThread(const QFileInfoList &fileList, int idBeginsFrom = 0, int flag = -1);

    /**
     * @brief AudioInfoThread destructor
     */
    ~AudioInfoThread();

    /**
     * @brief set list of files to read audio information from
     * @param fileList list of files to be read
     */
    void setFileList(const QFileInfoList &fileList);

    /**
     * @brief set list of files and list of metadata correspond to the given file list
     * @param fileList list of files
     * @param metadataList list of metadata
     */
    void setFileListWithMetadata(const QFileInfoList &fileList, const QStringList &metadataList);

    /**
     * @brief set identifier of first entry in the list
     * @param beginID first ID available now
     */
    void setBeginID(int beginID);

    /**
     * @brief set default flag for entries
     * @param flag default flag for entries
     */
    void setDeafultFlag(int flag);

signals:

    /**
     * @brief this signal is emitted everytime when thread has finished processing a single entry
     * @param percentage
     */
    void processUpdated(int percentage);

    /**
     * @brief taskCompleted is emitted when the thread has completed processing all entries
     */
    void taskCompleted(const AudioList &);

protected:

    /**
     * @brief run
     */
    virtual void run() override;

private:

    QFileInfoList m_fileList;
    QStringList m_metadataList;
    QString m_ffmpegPath;
    int m_flag; // represents entries' status
    int m_IDBeginsFrom;
};

#endif // AUDIOINFOTHREAD_H
