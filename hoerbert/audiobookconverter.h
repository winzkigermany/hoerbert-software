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

#ifndef AUDIOBOOKCONVERTER_H
#define AUDIOBOOKCONVERTER_H

#include <QThread>
#include <QFileInfoList>
#include "processexecutor.h"
#include "convertchaptertask.h"

/**
 * @brief The AudioBookConverter class converts audio book(*.m4b) into several audio files based on chapters in it
 */
class AudioBookConverter : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief AudioBookConverter constructor
     */
    AudioBookConverter();

    /**
     * @brief getConvertedFileInfoList
     * @return list of QFileInfo for converted files
     */
    void convert(const QString &absoluteFilePath = QString());

    bool is_finished() const
    {
        return m_is_finished;
    }
    /**
     * @brief abort cancel processing at current stage
     */
    void abort();

signals:

    /**
     * @brief failed is emitted when an error is occurred while processing
     * @param errorString indicates what error
     */
    void failed(const QString &errorString);

    /**
     * @brief processUpdated is emitted everytime when a file is converted from m4b to mp3
     * @param processedCount percentage of whole processing
     */
    void processUpdated(int processedCount);

    void finished(QFileInfoList);

private:

    /**
     * @brief parseForChapters parse ffmpeg output to get chapter start, end and title for metadata
     * @param output output string of ffmpeg
     * @return list of strings, each string represents a single chapter info
     *             (i.e. 0.000000<!@#^&>307.836000<!@#^&>West life - My love, <!@#^&> as delimiter)
     */
    QStringList parseForChapters(const QString &output);

    /**
     * @brief getVolumeDifference answers the question how much the volume deviates from the destination volume that is set for the app.
     * @param sourceFilePath source audio file
     * @return the correction in dB that must be applied to the file
     */
    double getVolumeDifference(const QString &sourceFilePath);

    std::unique_ptr<ConvertChapterTask> m_convertChapterTasks;
    QString m_filePath;
    bool m_is_finished;
    int m_counter;
    QStringList m_chapters;

    QString m_audioVolume;
    int m_maxMetadataLength;
    ProcessExecutor m_processExecutor;

    std::map<int,QString> info_list_map;
};

#endif // AUDIOBOOKCONVERTER_H
