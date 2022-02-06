/***************************************************************************
 * hörbert Software
 * Copyright (C) 2019 WINZKI GmbH & Co. KG
 *
 * Author: Rainer Brang
 * Feb. 2022
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

#ifndef DOWNLOADDIALOG_H
#define DOWNLOADDIALOG_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QUrl>

class QDialogButtonBox;
class QFile;
class QLabel;
class QLineEdit;
class QPushButton;
class QSslError;
class QAuthenticator;
class QNetworkReply;
class QProgressBar;
class MainWindow;


class DownloadDialog : public QDialog
{
    Q_OBJECT

public:
    DownloadDialog(QWidget *parent = 0);
    void downloadFile();

private slots:
    void cancelDownload();
    void httpFinished();
    void httpReadyRead();
    void updateDataReadProgress(qint64 bytesRead, qint64 totalBytes);
   // void slotAuthenticationRequired(QNetworkReply*,QAuthenticator *);
#ifndef QT_NO_SSL
    //void sslErrors(QNetworkReply*,const QList<QSslError> &errors);
#endif

private:
    QLabel *m_statusLabel;
    QPushButton *m_quitButton;
    QPushButton *m_instructionButton;
    QDialogButtonBox *m_buttonBox;
    QProgressBar *m_progressBar;
    QString m_redirectedFileName;

    MainWindow* m_mainWindow;

    QUrl m_url;
    QNetworkAccessManager m_qnam;
    QNetworkReply *m_reply;
    QFile *m_downloadedFile;
    int m_httpGetId;
    bool m_httpRequestAborted;

    void startRequest(QUrl url);
};


#endif // DOWNLOADDIALOG_H
