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

#include "cdripper.h"

#include <QProcess>
#include <QDir>
#include <QApplication>
#include <QDebug>

extern QString FREAC_PATH;
extern QStringList PROCESS_ERROR;
extern QString HOERBERT_TEMP_PATH;

CDRipper::CDRipper(const QFileInfoList &fileInfoList)
    : QThread ()
{
    m_fileList = fileInfoList;
    m_abort = false;

    QDir dir(HOERBERT_TEMP_PATH);
    if (!dir.exists())
        dir.mkpath(HOERBERT_TEMP_PATH);

    qRegisterMetaType <QProcess::ProcessState> ("QProcess::ProcessState");
    qRegisterMetaType <QProcess::ExitStatus> ("QProcess::ExitStatus");
}
void CDRipper::setFileInfoList(const QFileInfoList &fileInfoList)
{
    m_fileList = fileInfoList;
}
void CDRipper::abort()
{
    m_abort = true;
}
void CDRipper::run()
{
    QStringList file_path_list;
    if (m_fileList.count() <= 0)
    {
        qDebug() << "No audio track to rip";
        emit failed("No audio track to rip");
        return;
    }

    m_process = new QProcess();
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::errorOccurred, [this] (QProcess::ProcessError error) {
        qDebug() << "Ripper process error occurred";
        qDebug() << error;
        emit failed("RipperThread:" + PROCESS_ERROR.at(error) + "\n" + m_process->readAllStandardError() + "\n" + m_process->readAllStandardOutput());
    });
    connect(m_process, &QProcess::readyReadStandardError, [this] () {
        qDebug() << "Ripper process standard error";
        qDebug() << m_process->readAllStandardError();
    });
    /*connect(m_process, &QProcess::readyReadStandardOutput, [this] () {
        qDebug() << "Ripper process standard output";
        qDebug() << m_process->readAllStandardOutput();
    });*/
    connect(m_process, &QProcess::started, [] () {
        qDebug() << "Ripper process started";
    });
    connect(m_process, &QProcess::stateChanged, [] (QProcess::ProcessState state) {
        qDebug() << "Ripper process state changed to" << state;
    });
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [] (int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << " - FFmpeg process finished! Exit Code:" << exitCode << ", Exit Status:" << exitStatus;
    });

    QStringList arguments;
    arguments.append("-e");
    arguments.append("flac");
    arguments.append("-o");
    arguments.append("<DEST_FILE>");
    arguments.append("<SOURCE_PATH>");

    int counter = 0;

    for (const auto& track : m_fileList)
    {
        if (!QFile::exists(track.filePath()))
        {
            emit failed(tr("Track does not exist in the path:").arg(track.filePath()));
            m_process->close();
            delete m_process;
            //m_process->deleteLater();
            return;
        }

        arguments[3] = HOERBERT_TEMP_PATH + QString::number(uniqueID) + ".FLAC";
        arguments[4] = track.filePath();

        qDebug() << "UniqueID:" << uniqueID;
        uniqueID++;

        processUpdated(1);      // show at least a bit of progress
        std::pair<int, QString> output = m_processExecutor.executeCommand(FREAC_PATH, arguments);

        file_path_list.append(arguments[3]);
        file_path_list.append(arguments[4]);

        if (m_abort)
        {
            //@TODO make processExecutor abortable?
            qDebug() << "Ripper thread aborted";
            return;
        }
        counter++;
        emit processUpdated(counter * 100 / m_fileList.count());
    }

    emit taskCompleted(file_path_list);
}
