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

#include "debugdialog.h"

#include <QPlainTextEdit>
#include <QDateTime>
#include <QPushButton>
#include <QLayout>
#include <QClipboard>
#include <QApplication>
#include <QMutexLocker>
#include <QDebug>

#include "scaledsizeprovider.h"

DebugDialog::DebugDialog(QWidget *parent)
    : QDialog(parent)
{
    setFixedSize(ScaledSizeProvider::getScaledSize(480, 320));
    setWindowTitle(tr("An error has occurred"));
    setModal(true);

    m_logEdit = new QPlainTextEdit(this);
    m_logEdit->setReadOnly(true);

    m_sendButton = new QPushButton(this);
    m_sendButton->setText(tr("Send"));
    m_sendButton->setFixedWidth(100);
    m_sendButton->setVisible(false);        // Added this feature for use in the future

    m_copyButton = new QPushButton(this);
    m_copyButton->setText(tr("Copy to clipboard"));

    m_closeButton = new QPushButton(this);
    m_closeButton->setText(tr("Close"));
    m_closeButton->setFixedWidth(100);

    m_layout = new QVBoxLayout(this);

    m_buttonLayout = new QHBoxLayout;
    m_buttonLayout->setAlignment(Qt::AlignRight);

    m_buttonLayout->addWidget(m_sendButton);
    m_buttonLayout->addWidget(m_copyButton);
    m_buttonLayout->addWidget(m_closeButton);

    m_layout->addWidget(m_logEdit);
    m_layout->addLayout(m_buttonLayout);

    connect(m_sendButton, &QPushButton::clicked, this, &DebugDialog::sendErrorMessage);
    connect(m_copyButton, &QPushButton::clicked, this, [this] () {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(m_logEdit->toPlainText());
    });
    connect(m_closeButton, &QPushButton::clicked, this, &DebugDialog::close);

    m_autoVisible = true;

    m_logEdit->appendPlainText("[" + tr("Operating System") + "]");
    m_logEdit->appendPlainText(QSysInfo::productType() + " " + QSysInfo::kernelType() + " " + QSysInfo::kernelVersion());

    m_index = 0;
}

void DebugDialog::appendLog(const QString &text)
{
    QMutexLocker locker(&m_mutex);

    m_logEdit->appendPlainText("\n[" + tr("Error") + " #" + QString::number(m_index + 1) + "]");
    m_index++;

    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_logEdit->appendPlainText(QString("%1\n%2\n").arg("["+tr("Time") + "]\n" + time).arg(text));

    if (m_autoVisible)
        this->show();
}
void DebugDialog::setLog(const QString &text)
{
    QMutexLocker locker(&m_mutex);

    QString time = QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yyyy");
    m_logEdit->setPlainText(QString("%1\n%2").arg(time).arg(text));

    if (m_autoVisible)
        this->show();
}
void DebugDialog::setAutoVisable(bool flag)
{
    m_autoVisible = flag;
}
void DebugDialog::closeEvent(QCloseEvent *e)
{
    hide();
    e->ignore();
}
void DebugDialog::sendErrorMessage()
{
    qDebug() << "Not able to send message yet!";
    return;
}
