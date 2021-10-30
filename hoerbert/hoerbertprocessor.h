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

#ifndef HOERBERTPROCESSOR_H
#define HOERBERTPROCESSOR_H

#include <QThread>
#include <QObject>
#include <QProcess>
#include <QMutex>

#include "define.h"
#include "processexecutor.h"

const int PROCESS_LIFECYCLE                  = -1; // in msec, -1 means no time out
const int SPLIT_AUDIO_SEGMENT_LENGTH         = 180; // in seconds

/**
 * @brief The HoerbertProcessor class processes user changes using ffmpeg and file operations concurrently along with GUI thread
 */
class HoerbertProcessor : public QThread
{
    Q_OBJECT
public:

    /**
     * @brief HoerbertProcessor constructor
     * @param dirPath
     */
    HoerbertProcessor(const QString &dirPath, int dirNum);

    /**
     * @brief HoerbertProcessor destructor
     */
    ~HoerbertProcessor() override;

    /**
     * @brief add entries to be processed to the thread
     * @param type
     * @param list
     */
    void addEntryList(ENTRY_LIST_TYPE type, const AudioList &list);

    /**
     * @brief set entry data
     * @param list
     */
    void setEntryList(const QMap<ENTRY_LIST_TYPE, AudioList> &list);

    /**
     * @brief terminate override
     */
    void abort();

    /**
     * @brief returns the directory number the thread is working on
     * @return directory number
     */
    int directoryNumber();

signals:
    /**
     * @brief this signal is emitted every processing entry is done
     * @param percentage
     */
    void processUpdated(int percentage);

    /**
     * @brief this signal is emitted when error occurs while executing ffmpeg command line
     * @param errorType
     * @param errorString
     */
    void errorOccurred(QProcess::ProcessError errorType, const QString &errorString);

    /**
     * @brief this signal is emitted when the thread run loop is over
     * @param failCount number of entries failed to process
     * @param totalCount number of total entries
     */
    void taskCompleted(int failCount, int totalCount);

    /**
     * @brief this signal is emitted when no silence is detected while working on splitting on silence
     */
    void noSilenceDetected();

    /**
     * @brief this signal is emitted when error occurrs
     * @param errorString
     */
    void failed(const QString &errorString);

private slots:

    void OnProcessStarted();
    void OnProcessErrorOccurred(QProcess::ProcessError error);
    void OnProcessReadyReadStandardError();
    void OnProcessReadyReadStandardOutput();
    void OnProcessStateChanged(QProcess::ProcessState newState);

protected:

    /**
     * @brief process entries concurrently
     */
    virtual void run() override;

private:

    /**
     * @brief removeEntry delete the file from the drive
     * @return true if success, false otherwise
     */
    bool removeEntry(const AudioEntry &);

    /**
     * @brief addEntry convert the file to hoerbert format and put into the directory
     * @return true if success, false otherwise
     */
    bool addEntry(const AudioEntry &);

    /**
     * @brief moveEntry rename the file
     * @return true if success, false otherwise
     */
    bool moveEntry(const AudioEntry &);

    /**
     * @brief splitEntry split audio per 3 mins or by silence
     * @return true if success, false otherwise
     */
    bool splitEntry(const AudioEntry &);

    /**
     * @brief changeEntryMetadata change metadata only
     * @return true if success, false otherwise
     */
    bool changeEntryMetadata(const AudioEntry &);

    /**
     * @brief convertToWav convert all supported file formats to hoerbert format
     * @param sourceFilePath absolute path of the file to be converted
     * @param destFilePath absolute path of converted file
     * @param metadata metadata of audio
     * @return true if success, false otherwise
     */
    bool convertToAudioFile(const QString &sourceFilePath, const QString &destFilePath, const MetaData &metadata);

    /**
     * @brief splitPer3Mins split audio per 3 minutes length, the last chunk would be the shortest
     * @param sourceFilePath absolute path of the file to be split
     * @param destDir absolute path where we want to put converted file
     * @param outfileName output filename without extension
     * @param metadata metadata title of audio
     * @return true if success, false otherwise
     */
    bool splitPer3Mins(const QString &sourceFilePath, const QString &destDir, int outfileName, const MetaData &metadata);

    /**
     * @brief splitOnSilence split audio by silence, chunks are larger than 3 minutes except the last one
     * @param sourceFilePath absolute path of the file to be split
     * @param destDir absolute path where we want to put converted file
     * @param outfileName output filename without extension
     * @param metadata metadata of audio
     * @return true if success, false otherwise
     */
    bool splitOnSilence(const QString &sourceFilePath, const QString &destDir, int outfileName, const MetaData &metadata);

    /**
     * @brief createSilenceWav create a wav file with silence for whole duration
     * @param destFilePath absolute path of the file
     * @param duration duration of audio
     * @param metadata metadata of audio
     * @return true if success, false otherwise
     */
    bool createSilenceAudioFile(const QString &destFilePath, int duration, const MetaData &metadata);

    /**
     * @brief changeMetaData change metadata only
     * @param sourceFilePath absolute path to the file to be changed with metadata
     * @param metadata metadata of audio
     * @return true if success, false otherwise
     */
    bool changeMetaData(const QString &sourceFilePath, const MetaData &metadata, const QString &suffix = "");

    /**
     * @brief renameSplitFiles rename split files in order
     * @param destDir absolute directory path where the files exist
     * @param suffix suffix for split files
     * @return true if success, false otherwise
     */
    bool renameSplitFiles(const QString &destDir);

    /**
     * @brief getVolumeDifference answers the question how much the volume deviates from the destination volume that is set for the app.
     * @param sourceFilePath source audio file
     * @return the correction in dB that must be applied to the file
     */
    double getVolumeDifference(const QString &sourceFilePath);

    QProcess *m_process;
    QString m_dirPath;
    int m_dirNum;
    bool m_abort;

    QMap<ENTRY_LIST_TYPE, AudioList> m_entries;

    int m_maxMetadataLength; // maximum length of metadata in bytes
    QString m_audioVolume; // audio volume of all converted audios in dB
    QString m_split3SegmentLength; // minimum length of split audio per 3 minutes in seconds
    QString m_silenceNoise; // noise tolerance range for silence detection in dB
    QString m_silenceDetectDuration; // delay of silence for this period is considered as silence, in seconds
    QString m_splitOnSilenceSegmentLength; // minmum length of split audio by silence detection, in seconds

    int m_counter;
    int m_failCounter;
    int m_totalEntryCount;

    int m_splitOffset; // this offset is used to get correct split files while dealing with split files.

    static QMutex m_processingMutex;
    ProcessExecutor m_processExecutor;

};

#endif // HOERBERTPROCESSOR_H
