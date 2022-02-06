/***************************************************************************
 * hörbert Software
 * Copyright (C) 2019 WINZKI GmbH & Co. KG
 *
 * Author: Rainer Brang
 * Feb. 2022
 *
 * Kudos to
 * https://www.bogotobogo.com/Qt/Qt5_QNetworkRequest_Http_File_Download.php
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

#include <QtWidgets>
#include <QtNetwork>

#include "downloaddialog.h"
#include "functions.h"
#include "mainwindow.h"
#include "define.h"

extern QString HOERBERT_TEMP_PATH;

DownloadDialog::DownloadDialog(QWidget *parent)
    : QDialog(parent)
{
    if (objectName().isEmpty())
        setObjectName(QString("DownloadDialog"));

    m_mainWindow = dynamic_cast<MainWindow*>(parent);

    resize(320, 120);
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setSizePolicy(sizePolicy);

    m_statusLabel = new QLabel();
    m_statusLabel->setWordWrap(true);

    m_quitButton = new QPushButton(tr("Close"));
    m_quitButton->setAutoDefault(false);
    connect( m_quitButton, &QPushButton::clicked, this, &DownloadDialog::close);

    m_instructionButton = new  QPushButton(tr("Read online: Next steps to update the firmware"));
    connect( m_instructionButton, &QPushButton::clicked, this, [=](){
        QDesktopServices::openUrl(QUrl(tr("https://en.hoerbert.com/firmware-update-en/?downloaded=%1").arg(m_redirectedFileName)));
        this->close();
    });

    m_buttonBox = new QDialogButtonBox;
    m_buttonBox->addButton(m_instructionButton, QDialogButtonBox::AcceptRole);
    m_buttonBox->addButton(m_quitButton, QDialogButtonBox::RejectRole);

    m_progressBar = new QProgressBar(this);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget( m_progressBar );
    mainLayout->addWidget(m_buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Firmware download"));
}

void DownloadDialog::startRequest(QUrl url)
{
    m_reply = m_qnam.get(QNetworkRequest(url));
    connect(m_reply, &QNetworkReply::finished, this, &DownloadDialog::httpFinished);
    connect(m_reply, &QNetworkReply::readyRead, this, &DownloadDialog::httpReadyRead);
    connect(m_reply, &QNetworkReply::downloadProgress, this, &DownloadDialog::updateDataReadProgress);
}

void DownloadDialog::downloadFile()
{
    m_url.setUrl( FIRMWARE_DOWNLOAD_URL );
    QString fileName = tailPath(HOERBERT_TEMP_PATH)+FIRMWARE_BINARY_FILE;

    if (QFile::exists(fileName)) {
        qDebug() << "removing existing file: " << fileName;
        QFile::remove(fileName);
    }

    m_downloadedFile = new QFile(fileName);
    if (!m_downloadedFile->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("Firmware download"), tr("Unable to save the file %1: %2.").arg(fileName).arg(m_downloadedFile->errorString()));
        delete m_downloadedFile;
        m_downloadedFile = 0;
        return;
    }

    // schedule the request
    m_httpRequestAborted = false;
    startRequest(m_url);
}

void DownloadDialog::cancelDownload()
{
    m_statusLabel->setText(tr("Download canceled."));
    m_httpRequestAborted = true;
    if( m_reply ){
        m_reply->abort();
    }
}

void DownloadDialog::httpFinished()
{
    if (m_httpRequestAborted) {
        if (m_downloadedFile) {
            m_downloadedFile->close();
            m_downloadedFile->remove();
            delete m_downloadedFile;
            m_downloadedFile = 0;
        }
        m_reply->deleteLater();
        return;
    }

    m_downloadedFile->flush();
    m_downloadedFile->close();

    QVariant redirectionTarget = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (m_reply->error()) {
        m_downloadedFile->remove();
        QMessageBox::information(this, tr("Firmware download error"), tr("Download failed:")+"\n"+m_reply->errorString() );

    } else if (!redirectionTarget.isNull()) {
        QUrl newUrl = m_url.resolved(redirectionTarget.toUrl());

        m_reply->deleteLater();
        m_downloadedFile->open(QIODevice::WriteOnly);
        m_downloadedFile->resize(0);
        startRequest(newUrl.toString());
        qDebug() << "http redirect to: " << newUrl.toString();

        m_redirectedFileName = newUrl.toString().section("/", -2, -2);
        return;

    } else {
        m_statusLabel->setText( tr("Successufully downloaded the firmware") );

        QString fileName =  tailPath(HOERBERT_TEMP_PATH)+FIRMWARE_BINARY_FILE;
        QFile downloadedFile( fileName );
        QString destinationFilename = m_mainWindow->getCurrentDrivePath()+FIRMWARE_BINARY_FILE;

        if (QFile::exists(destinationFilename)) {
            qDebug() << "removing existing file: " << destinationFilename;
            QFile::remove(destinationFilename);
        }

        if( downloadedFile.copy(destinationFilename) ){
            qDebug() << "file " << fileName << " downloaded and copied to " << destinationFilename;
            m_instructionButton->show();

        } else {
            m_mainWindow->processorErrorOccurred( "Error saving the downloaded firmware" + fileName + " to " + destinationFilename );
        }

    }

    m_reply->deleteLater();
    m_reply = 0;
    delete m_downloadedFile;
    m_downloadedFile = 0;
}

void DownloadDialog::httpReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    if (m_downloadedFile)
        m_downloadedFile->write(m_reply->readAll());
}

void DownloadDialog::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if (m_httpRequestAborted)
        return;

    m_progressBar->setMaximum(totalBytes);
    m_progressBar->setValue(bytesRead);
}
