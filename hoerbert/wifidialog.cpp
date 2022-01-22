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
    setWindowTitle(tr("Configure up to 5 WiFi networks for use with hörbert"));

    m_instructionLabel = new QLabel(this);
    m_instructionLabel->setText(tr("Please enter the wireless network's name in the SSID field, and its password in the passkey field. Remember that hörbert can only connect to wireless networks with 2.4 GHz."));
    m_instructionLabel->setWordWrap(true);

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

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget( m_instructionLabel );
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

void WifiDialog::showEvent(QShowEvent * event){
    Q_UNUSED(event);
    readWifiSettings();
}


void WifiDialog::readWifiSettings(){

    QString iniFileName = m_mainWindow->getCurrentDrivePath() + WIFI_INI_FILE;

    if( QFile(iniFileName).exists() ){
        QSettings settings( iniFileName, QSettings::IniFormat);
        settings.beginGroup("network1");
        m_wifiSsid0->setText( settings.value("ssid").toString() );
        m_wifiKey0->setText( settings.value("password").toString() );
        settings.endGroup();

        settings.beginGroup("network2");
        m_wifiSsid1->setText( settings.value("ssid").toString() );
        m_wifiKey1->setText( settings.value("password").toString() );
        settings.endGroup();

        settings.beginGroup("network3");
        m_wifiSsid2->setText( settings.value("ssid").toString() );
        m_wifiKey2->setText( settings.value("password").toString() );
        settings.endGroup();

        settings.beginGroup("network4");
        m_wifiSsid3->setText( settings.value("ssid").toString() );
        m_wifiKey3->setText( settings.value("password").toString() );
        settings.endGroup();

        settings.beginGroup("network5");
        m_wifiSsid4->setText( settings.value("ssid").toString() );
        m_wifiKey4->setText( settings.value("password").toString() );
        settings.endGroup();
    } else {
        qDebug() << "ini file: " << iniFileName << " does not exist.";
    }

}


void WifiDialog::saveWifiSettings(){

    QString s;
    QString p;
    QString iniFileName = m_mainWindow->getCurrentDrivePath() + WIFI_INI_FILE;
    QSettings* settings = new QSettings( iniFileName, QSettings::IniFormat, nullptr);

    settings->beginGroup("network1");
    s = m_wifiSsid0->text().trimmed();
    s.truncate(32);
    if( !s.isEmpty() ){
        p = m_wifiKey0->text().trimmed();
        p.truncate(63);

        settings->setValue("password", p);
        settings->setValue("ssid", s);
    }
    settings->endGroup();

    settings->beginGroup("network2");
    s = m_wifiSsid1->text().trimmed();
    s.truncate(32);
    if( !s.isEmpty() ){
        p = m_wifiKey1->text().trimmed();
        p.truncate(63);

        settings->setValue("password", p);
        settings->setValue("ssid", s);
    }
    settings->endGroup();

    settings->beginGroup("network3");
    s = m_wifiSsid2->text().trimmed();
    s.truncate(32);
    if( !s.isEmpty() ){
        p = m_wifiKey2->text().trimmed();
        p.truncate(63);

        settings->setValue("password", p);
        settings->setValue("ssid", s);
    }
    settings->endGroup();

    settings->beginGroup("network4");
    s = m_wifiSsid3->text().trimmed();
    s.truncate(32);
    if( !s.isEmpty() ){
        p = m_wifiKey3->text().trimmed();
        p.truncate(63);

        settings->setValue("password", p);
        settings->setValue("ssid", s);
    }
    settings->endGroup();

    settings->beginGroup("network5");
    s = m_wifiSsid4->text().trimmed();
    s.truncate(32);
    if( !s.isEmpty() ){
        p = m_wifiKey4->text().trimmed();
        p.truncate(63);

        settings->setValue("password", p);
        settings->setValue("ssid", s);
    }
    settings->endGroup();

    delete settings;    // forces the ini file to be written

    this->close();     // close the dialog
}
