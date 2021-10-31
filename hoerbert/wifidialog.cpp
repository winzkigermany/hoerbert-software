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

#include "wifidialog.h"
#include "define.h"

#include <QLocale>
#include <QWidget>
#include <QApplication>
#include <QFormLayout>
#include <QFile>
#include <QTextStream>


WifiDialog::WifiDialog(QWidget* parent)
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint )
{
    if (objectName().isEmpty())
        setObjectName(QString("WifiDialog"));

    m_mainWindow = (MainWindow*) parent;

    resize(480, 480);
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setSizePolicy(sizePolicy);
    setWindowTitle(tr("WiFi settings"));

    QLabel instructionLabel(this);
    instructionLabel.setText(tr("Configure up to 5 WiFi networks for use with hörbert"));
    instructionLabel.setAlignment(Qt::AlignHCenter);

    m_wifiSsid0 = new QLineEdit(this);
    m_wifiKey0 = new QLineEdit(this);
    m_wifiKey0->setEchoMode(QLineEdit::Password);
    m_wifiSsid1 = new QLineEdit(this);
    m_wifiKey1 = new QLineEdit(this);
    m_wifiKey1->setEchoMode(QLineEdit::Password);
    m_wifiSsid2 = new QLineEdit(this);
    m_wifiKey2 = new QLineEdit(this);
    m_wifiKey2->setEchoMode(QLineEdit::Password);
    m_wifiSsid3 = new QLineEdit(this);
    m_wifiKey3 = new QLineEdit(this);
    m_wifiKey3->setEchoMode(QLineEdit::Password);
    m_wifiSsid4 = new QLineEdit(this);
    m_wifiKey4 = new QLineEdit(this);
    m_wifiKey4->setEchoMode(QLineEdit::Password);

    QVBoxLayout* layout = new QVBoxLayout(this);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("SSID:"), m_wifiSsid0);
    formLayout->addRow(tr("Passkey:"), m_wifiKey0);
    formLayout->addRow(" ", (QWidget*)nullptr);
    formLayout->addRow(tr("SSID:"), m_wifiSsid1);
    formLayout->addRow(tr("Passkey:"), m_wifiKey1);
    formLayout->addRow(" ", (QWidget*)nullptr);
    formLayout->addRow(tr("SSID:"), m_wifiSsid2);
    formLayout->addRow(tr("Passkey:"), m_wifiKey2);
    formLayout->addRow(" ", (QWidget*)nullptr);
    formLayout->addRow(tr("SSID:"), m_wifiSsid3);
    formLayout->addRow(tr("Passkey:"), m_wifiKey3);
    formLayout->addRow(" ", (QWidget*)nullptr);
    formLayout->addRow(tr("SSID:"), m_wifiSsid4);
    formLayout->addRow(tr("Passkey:"), m_wifiKey4);
    formLayout->addRow(" ", (QWidget*)nullptr);

    QWidget* formWidget = new QWidget(this);
    formWidget->setLayout( formLayout );

    layout->addWidget( &instructionLabel );
    layout->addWidget( formWidget );

    QHBoxLayout* bottomLine = new QHBoxLayout(this);
    bottomLine->setAlignment(Qt::AlignRight);
    bottomLine->setSpacing(10);

    m_cancelButton = new QPushButton(this);
    m_cancelButton->setGeometry(QRect(250, 100, 112, 32));
    m_cancelButton->setText(tr("Cancel"));
    bottomLine->addWidget(m_cancelButton, 0, Qt::AlignRight);
    connect( m_cancelButton, &QAbstractButton::clicked, this, &WifiDialog::close);

    m_saveButton = new QPushButton(this);
    m_saveButton->setGeometry(QRect(250, 100, 112, 32));
    m_saveButton->setText(tr("Save"));
    m_saveButton->setDefault(true);
    bottomLine->addWidget(m_saveButton, 0, Qt::AlignRight);
    connect( m_saveButton, &QAbstractButton::clicked, this, &WifiDialog::saveWifiSettings);

    layout->addItem( bottomLine );
}


void WifiDialog::saveWifiSettings(){
/*
    [network1]
    ssid=cyberspace24
    password=dubiduaa
*/
    QString s;
    QString p;
    QString wifiIniContents = "";

    // create wifi.ini on the memory card
    s = m_wifiSsid0->text().trimmed();
    s.truncate(32);
    if( !s.isEmpty() ){
        p = m_wifiKey0->text().trimmed();
        p.truncate(63);

        wifiIniContents += "[network1]\n";
        wifiIniContents += "ssid="+s+"\n";
        wifiIniContents += "password="+p+"\n";
        wifiIniContents += "\n";
    }

    s = m_wifiSsid1->text().trimmed();
    s.truncate(32);
    if( !s.isEmpty() ){
        p = m_wifiKey1->text().trimmed();
        p.truncate(63);

        wifiIniContents += "[network2]\n";
        wifiIniContents += "ssid="+s+"\n";
        wifiIniContents += "password="+p+"\n";
        wifiIniContents += "\n";
    }

    s = m_wifiSsid2->text().trimmed();
    s.truncate(32);
    if( !s.isEmpty() ){
        p = m_wifiKey2->text().trimmed();
        p.truncate(63);

        wifiIniContents += "[network3]\n";
        wifiIniContents += "ssid="+s+"\n";
        wifiIniContents += "password="+p+"\n";
        wifiIniContents += "\n";
    }

    s = m_wifiSsid3->text().trimmed();
    s.truncate(32);
    if( !s.isEmpty() ){
        p = m_wifiKey3->text().trimmed();
        p.truncate(63);

        wifiIniContents += "[network4]\n";
        wifiIniContents += "ssid="+s+"\n";
        wifiIniContents += "password="+p+"\n";
        wifiIniContents += "\n";
    }

    s = m_wifiSsid4->text().trimmed();
    s.truncate(32);
    if( !s.isEmpty() ){
        p = m_wifiKey4->text().trimmed();
        p.truncate(63);

        wifiIniContents += "[network5]\n";
        wifiIniContents += "ssid="+s+"\n";
        wifiIniContents += "password="+p+"\n";
        wifiIniContents += "\n";
    }

    QFile file( m_mainWindow->getCurrentDrivePath() + WIFI_INI_FILE );
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Writing wifi.ini file: " << file;
        // overwrite the file every time.
        QTextStream out(&file);
        out << wifiIniContents;
        file.close();
    }

    this->close();     // close the dialog
}
