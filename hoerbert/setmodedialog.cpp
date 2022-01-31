/***************************************************************************
 * hörbert Software
 * Copyright (C) 2022 WINZKI GmbH & Co. KG
 *
 * Author: Rainer Brang
 * Jan. 2022
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

#include "setmodedialog.h"
#include "define.h"

#include <QLocale>
#include <QWidget>
#include <QApplication>
#include <QFormLayout>
#include <QFile>
#include <QTextStream>
#include <QButtonGroup>
#include <QRect>


SetModeDialog::SetModeDialog(QWidget* parent)
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint )
{
    if (objectName().isEmpty())
        setObjectName(QString("SetModeDialog"));

    m_mainWindow = (MainWindow*) parent;

    resize(640, 480);
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setSizePolicy(sizePolicy);
    setWindowTitle(tr("Configure SET mode settings"));

    m_instructionLabel = new QLabel();
    m_instructionLabel->setText(tr("To activate these settings in hörbert:\n1) Save your changes and eject the memory card\n2) put the memory card back in hörbert\n4) turn hörbert on\n4) Press the SET button inside hörbert to light up the green light\n5) Press the SET button again to save these settings."));
    m_instructionLabel->setWordWrap(true);

    QHBoxLayout* sleepTime = new QHBoxLayout();
    QLabel* minutesLabel = new QLabel(tr("minutes"));
    m_sleepTimerComboBox = new QComboBox();
    m_sleepTimerComboBox->setMaximumWidth( 60 );
    for( int i=10; i<130; i+=10 ){
        m_sleepTimerComboBox->addItem( QString::number(i), i );
    }
    m_sleepTimerComboBox->setEditable(true);
    sleepTime->addWidget( m_sleepTimerComboBox );
    sleepTime->addWidget( minutesLabel );

    QHBoxLayout* bluetoothHBox = new QHBoxLayout();
    bluetoothHBox->setAlignment( Qt::AlignLeft );
    m_bluetoothRadioButtons = new QButtonGroup();
    m_bluetoothRadioButtons->setExclusive(true);
    QRadioButton* bluetoothOnWithPairing = new QRadioButton();
    bluetoothOnWithPairing->setText(tr("Allowed with pairing"));
    QRadioButton* bluetoothOn = new QRadioButton();
    bluetoothOn->setText(tr("Allowed, no pairing"));
    QRadioButton* bluetoothOff = new QRadioButton();
    bluetoothOff->setText(tr("Off"));
    m_bluetoothRadioButtons->addButton( bluetoothOnWithPairing );
    m_bluetoothRadioButtons->addButton( bluetoothOn );
    m_bluetoothRadioButtons->addButton( bluetoothOff );
    bluetoothHBox->addWidget( bluetoothOnWithPairing );
    bluetoothHBox->addWidget( bluetoothOn );
    bluetoothHBox->addWidget( bluetoothOff );

    m_bluetoothDeletePairingsCheckbox = new QCheckBox();
    m_bluetoothDeletePairingsCheckbox->setText(tr("Delete all existing pairings"));

    QHBoxLayout* volumeHBox = new QHBoxLayout();
    volumeHBox->setAlignment( Qt::AlignLeft );
    m_volumeLimiterRadioButtons = new QButtonGroup();
    m_volumeLimiterRadioButtons->setExclusive(true);
    QRadioButton* forteMode = new QRadioButton();
    forteMode->setText(tr("forte (normal volume)"));
    QRadioButton* pianissimoMode = new QRadioButton();
    pianissimoMode->setText(tr("pianissimo (limited volume)"));
    m_volumeLimiterRadioButtons->addButton( forteMode );
    m_volumeLimiterRadioButtons->addButton( pianissimoMode );
    volumeHBox->addWidget( forteMode );
    volumeHBox->addWidget( pianissimoMode );

    QHBoxLayout* wifiHBox = new QHBoxLayout();
    wifiHBox->setAlignment( Qt::AlignLeft );
    m_wifiRadioButtons = new QButtonGroup();
    m_wifiRadioButtons->setExclusive(true);
    QRadioButton* wifiOn = new QRadioButton();
    wifiOn->setText(tr("Allowed"));
    QRadioButton* wifiOff = new QRadioButton();
    wifiOff->setText(tr("Off"));
    m_wifiRadioButtons->addButton( wifiOn );
    m_wifiRadioButtons->addButton( wifiOff );
    wifiHBox->addWidget( wifiOn );
    wifiHBox->addWidget( wifiOff );

    m_readWifiSettingsCheckbox = new QCheckBox();
    m_readWifiSettingsCheckbox->setText(tr("Configure wireless networks from configuration file"));

    QHBoxLayout* microphoneHBox = new QHBoxLayout();
    microphoneHBox->setAlignment( Qt::AlignLeft );
    m_microphoneRadioButtons = new QButtonGroup();
    m_microphoneRadioButtons->setExclusive(true);
    QRadioButton* micOn = new QRadioButton();
    micOn->setText(tr("Allow"));
    QRadioButton* micOff = new QRadioButton();
    micOff->setText(tr("Off"));
    m_microphoneRadioButtons->addButton( micOn );
    m_microphoneRadioButtons->addButton( micOff );
    microphoneHBox->addWidget( micOn );
    microphoneHBox->addWidget( micOff );

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(new QLabel(tr("Sleep timer:")), sleepTime);
    formLayout->addRow(new QLabel(" "), (QWidget*)nullptr);
    formLayout->addRow(new QLabel(tr("Bluetooth:")), bluetoothHBox);
    formLayout->addRow(new QLabel(tr("Reset?")), m_bluetoothDeletePairingsCheckbox);
    formLayout->addRow(new QLabel(" "), (QWidget*)nullptr);
    formLayout->addRow(new QLabel(tr("Volume limit:")), volumeHBox);
    formLayout->addRow(new QLabel(" "), (QWidget*)nullptr);
    formLayout->addRow(new QLabel(tr("Wifi:")), wifiHBox);
    formLayout->addRow(new QLabel(tr("Configure?")), m_readWifiSettingsCheckbox);
    formLayout->addRow(new QLabel(" "), (QWidget*)nullptr);
    formLayout->addRow(new QLabel(tr("Microphone:")), microphoneHBox);
    formLayout->addRow(new QLabel(" "), (QWidget*)nullptr);
    formLayout->addRow(new QLabel(" "), m_instructionLabel);
    formLayout->addRow(new QLabel(" "), (QWidget*)nullptr);


    QWidget* formWidget = new QWidget();
    formWidget->setLayout( formLayout );

    QHBoxLayout* bottomLine = new QHBoxLayout();
    bottomLine->setAlignment(Qt::AlignRight);
    bottomLine->setSpacing(10);

    m_cancelButton = new QPushButton();
    m_cancelButton->setGeometry(QRect(250, 100, 112, 32));
    m_cancelButton->setText(tr("Cancel"));
    bottomLine->addWidget(m_cancelButton, 0, Qt::AlignRight);
    connect( m_cancelButton, &QAbstractButton::clicked, this, &WifiDialog::close);

    m_saveButton = new QPushButton();
    m_saveButton->setGeometry(QRect(250, 100, 112, 32));
    m_saveButton->setText(tr("Save"));
    m_saveButton->setDefault(true);
    bottomLine->addWidget(m_saveButton, 0, Qt::AlignRight);
    connect( m_saveButton, &QAbstractButton::clicked, this, &SetModeDialog::writeIndexM3uSettings);

    QVBoxLayout* dialogLayout = new QVBoxLayout();
    dialogLayout->addWidget( formWidget );
    dialogLayout->addItem( bottomLine );
    setLayout(dialogLayout);
}

void SetModeDialog::showEvent(QShowEvent * event){
    Q_UNUSED(event);
    readIndexM3uSettings();
}


void SetModeDialog::readIndexM3uSettings(){

    QString indexM3uFileName = m_mainWindow->getCurrentDrivePath() + INDEX_M3U_FILE;

    if( QFile(indexM3uFileName).exists() ){
        ;
    } else {
        qDebug() << "File: " << indexM3uFileName << " does not exist.";
    }

}


void SetModeDialog::writeIndexM3uSettings(){

    QString s;
    QString p;
    QString indexM3uFileName = m_mainWindow->getCurrentDrivePath() + INDEX_M3U_FILE;


    this->close();     // close the dialog
}
