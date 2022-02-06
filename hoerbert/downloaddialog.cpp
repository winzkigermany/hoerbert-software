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

    statusLabel = new QLabel();
    statusLabel->setWordWrap(true);

    quitButton = new QPushButton(tr("Close"));
    quitButton->setAutoDefault(false);
    connect( quitButton, &QPushButton::clicked, this, &DownloadDialog::close);

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    progressBar = new QProgressBar(this);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget( progressBar );
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Firmware download"));
}

void DownloadDialog::startRequest(QUrl url)
{
    reply = qnam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, &DownloadDialog::httpFinished);
    connect(reply, &QNetworkReply::readyRead, this, &DownloadDialog::httpReadyRead);
    connect(reply, &QNetworkReply::downloadProgress, this, &DownloadDialog::updateDataReadProgress);
}

void DownloadDialog::downloadFile()
{
    url.setUrl( FIRMWARE_DOWNLOAD_URL );
    QString fileName = tailPath(HOERBERT_TEMP_PATH)+FIRMWARE_BINARY_FILE;

    if (QFile::exists(fileName)) {
        qDebug() << "removing existing file: " << fileName;
        QFile::remove(fileName);
    }

    file = new QFile(fileName);
    if (!file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("Firmware download"), tr("Unable to save the file %1: %2.").arg(fileName).arg(file->errorString()));
        delete file;
        file = 0;
        return;
    }

    // schedule the request
    httpRequestAborted = false;
    startRequest(url);
}

void DownloadDialog::cancelDownload()
{
    statusLabel->setText(tr("Download canceled."));
    httpRequestAborted = true;
    if( reply ){
        reply->abort();
    }
}

void DownloadDialog::httpFinished()
{
    if (httpRequestAborted) {
        if (file) {
            file->close();
            file->remove();
            delete file;
            file = 0;
        }
        reply->deleteLater();
        return;
    }

    file->flush();
    file->close();

    QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (reply->error()) {
        file->remove();
        QMessageBox::information(this, tr("Firmware download error"), tr("Download failed:")+"\n"+reply->errorString() );

    } else if (!redirectionTarget.isNull()) {
        QUrl newUrl = url.resolved(redirectionTarget.toUrl());

        reply->deleteLater();
        file->open(QIODevice::WriteOnly);
        file->resize(0);
        startRequest(newUrl.toString());
        qDebug() << "http redirect to: " << newUrl.toString();
        return;

    } else {
        statusLabel->setText( tr("Successufully downloaded the firmware") );

        QString fileName =  tailPath(HOERBERT_TEMP_PATH)+FIRMWARE_BINARY_FILE;
        QFile downloadedFile( fileName );
        QString destinationFilename = m_mainWindow->getCurrentDrivePath()+FIRMWARE_BINARY_FILE;

        if( downloadedFile.copy(destinationFilename) ){
            qDebug() << "file " << fileName << " downloaded and copied to " << destinationFilename;

        } else {
            qDebug() << "error copying the downloaded file" << fileName << "to" << destinationFilename;

        }

    }

    reply->deleteLater();
    reply = 0;
    delete file;
    file = 0;
}

void DownloadDialog::httpReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    if (file)
        file->write(reply->readAll());
}

void DownloadDialog::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if (httpRequestAborted)
        return;

    progressBar->setMaximum(totalBytes);
    progressBar->setValue(bytesRead);
}
