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
#include <QAbstractButton>


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

    m_readInfoLabel = new QLabel();
    m_readInfoLabel->setAlignment( Qt::AlignHCenter );

    QHBoxLayout* sleepTime = new QHBoxLayout();
    QLabel* minutesLabel = new QLabel(tr("minutes"));
    m_sleepTimerComboBox = new QComboBox();
    m_sleepTimerComboBox->setMaximumWidth( 60 );
    m_sleepTimerComboBox->addItem( tr("off"), 0 );
    for( int i=10; i<130; i+=10 ){
        m_sleepTimerComboBox->addItem( QString::number(i), i );
    }
    m_sleepTimerComboBox->setEditable(true);
    sleepTime->addWidget( m_sleepTimerComboBox );
    sleepTime->addWidget( minutesLabel );

    m_bluetoothHBox = new QHBoxLayout();
    m_bluetoothHBox->setAlignment( Qt::AlignLeft );
    m_bluetoothOnWithPairing = new QRadioButton();
    m_bluetoothOnWithPairing->setText(tr("Allow with pairing"));
    m_bluetoothOn = new QRadioButton();
    m_bluetoothOn->setText(tr("Allow but without pairing"));
    m_bluetoothOff = new QRadioButton();
    m_bluetoothOff->setText(tr("Off"));
    m_bluetoothRadioButtons = new QButtonGroup();
    m_bluetoothRadioButtons->setExclusive(true);
    m_bluetoothRadioButtons->addButton( m_bluetoothOnWithPairing );
    m_bluetoothRadioButtons->addButton( m_bluetoothOn );
    m_bluetoothRadioButtons->addButton( m_bluetoothOff );
    m_bluetoothHBox->addWidget( m_bluetoothOnWithPairing );
    m_bluetoothHBox->addWidget( m_bluetoothOn );
    m_bluetoothHBox->addWidget( m_bluetoothOff );

    m_bluetoothDeletePairingsCheckbox = new QCheckBox();
    m_bluetoothDeletePairingsCheckbox->setText(tr("Delete all existing pairings"));

    m_volumeHBox = new QHBoxLayout();
    m_volumeHBox->setAlignment( Qt::AlignLeft );
    m_volumeLimiterRadioButtons = new QButtonGroup();
    m_volumeLimiterRadioButtons->setExclusive(true);
    m_forteMode = new QRadioButton();
    m_forteMode->setText(tr("forte (normal volume)"));
    m_pianissimoMode = new QRadioButton();
    m_pianissimoMode->setText(tr("pianissimo (limited volume)"));
    m_volumeLimiterRadioButtons->addButton( m_forteMode );
    m_volumeLimiterRadioButtons->addButton( m_pianissimoMode );
    m_volumeHBox->addWidget( m_forteMode );
    m_volumeHBox->addWidget( m_pianissimoMode );

    m_wifiHBox = new QHBoxLayout();
    m_wifiHBox->setAlignment( Qt::AlignLeft );
    m_wifiRadioButtons = new QButtonGroup();
    m_wifiRadioButtons->setExclusive(true);
    m_wifiOn = new QRadioButton();
    m_wifiOn->setText(tr("Allow"));
    m_wifiOff = new QRadioButton();
    m_wifiOff->setText(tr("Off"));
    connect( m_wifiOff, &QRadioButton::clicked, this, [=](){
        if( m_wifiOff->isChecked() ){
            m_readWifiSettingsCheckbox->setChecked(false);
        }
    });
    m_wifiRadioButtons->addButton( m_wifiOn );
    m_wifiRadioButtons->addButton( m_wifiOff );
    m_wifiHBox->addWidget( m_wifiOn );
    m_wifiHBox->addWidget( m_wifiOff );

    m_readWifiSettingsCheckbox = new QCheckBox();
    m_readWifiSettingsCheckbox->setText(tr("Configure wireless networks from configuration file"));
    connect( m_readWifiSettingsCheckbox, &QCheckBox::clicked, this, [=](){
        if( m_readWifiSettingsCheckbox->isChecked() ){
            m_wifiOn->setChecked(true);
        }
    });

    QHBoxLayout* microphoneHBox = new QHBoxLayout();
    microphoneHBox->setAlignment( Qt::AlignLeft );
    m_microphoneRadioButtons = new QButtonGroup();
    m_microphoneRadioButtons->setExclusive(true);
    m_micOn = new QRadioButton();
    m_micOn->setText(tr("Allow"));
    m_micOff = new QRadioButton();
    m_micOff->setText(tr("Off"));
    m_microphoneRadioButtons->addButton( m_micOn );
    m_microphoneRadioButtons->addButton( m_micOff );
    microphoneHBox->addWidget( m_micOn );
    microphoneHBox->addWidget( m_micOff );

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(new QLabel(tr("Sleep timer:")), sleepTime);
    formLayout->addRow(new QLabel(" "), (QWidget*)nullptr);
    formLayout->addRow(new QLabel(tr("Bluetooth:")), m_bluetoothHBox);
    formLayout->addRow(new QLabel(tr("Reset?")), m_bluetoothDeletePairingsCheckbox);
    formLayout->addRow(new QLabel(" "), (QWidget*)nullptr);
    formLayout->addRow(new QLabel(tr("Volume limit:")), m_volumeHBox);
    formLayout->addRow(new QLabel(" "), (QWidget*)nullptr);
    formLayout->addRow(new QLabel(tr("Wifi:")), m_wifiHBox);
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
    connect( m_cancelButton, &QAbstractButton::clicked, this, &SetModeDialog::close);
    bottomLine->addWidget(m_cancelButton, 0, Qt::AlignRight);

    m_saveButton = new QPushButton();
    m_saveButton->setGeometry(QRect(250, 100, 112, 32));
    m_saveButton->setText(tr("Save"));
    m_saveButton->setDefault(true);
    connect( m_saveButton, &QAbstractButton::clicked, this, &SetModeDialog::writeIndexM3uSettings);
    bottomLine->addWidget(m_saveButton, 0, Qt::AlignRight);

    m_dialogLayout = new QVBoxLayout();
    m_dialogLayout->addWidget( m_readInfoLabel );
    m_dialogLayout->addWidget( formWidget );
    m_dialogLayout->addItem( bottomLine );
    setLayout(m_dialogLayout);
}

void SetModeDialog::showEvent(QShowEvent * event){
    Q_UNUSED(event);
    readIndexM3uSettings();
}


/**
 * @brief get the number of minutes for the sleep timer from a line of text.
 * @param line
 */
void SetModeDialog::extractSleepTimerSettings( QString& line ){
    int minutes = line.trimmed().toInt();
    qDebug() << "Minutes: " << minutes;
    if( minutes>=0 ){
        int index = m_sleepTimerComboBox->findText( QString::number(minutes) );
        if( -1==index ){
            m_sleepTimerComboBox->insertItem( (minutes/10)+1, QString::number(minutes), minutes );
            m_sleepTimerComboBox->setCurrentIndex( (minutes/10)+1 );
        } else {
            m_sleepTimerComboBox->setCurrentIndex(index);
        }
    } else {
        m_sleepTimerComboBox->setCurrentIndex(0);
    }
}


/**
 * @brief get the bluetooth settings from a line of text.
 * @param line
 */
void SetModeDialog::extractBluetoothSettings( QString& line ){

    int bluetoothMode = line.trimmed().toInt();
    qDebug() << "bluetoothMode: " << bluetoothMode;
    if( bluetoothMode>=0 && bluetoothMode<=2){
        switch( bluetoothMode ){
        case 0:
            m_bluetoothOff->setChecked(true);
            break;
        case 1:
            m_bluetoothOn->setChecked(true);
            break;
        case 2:
            m_bluetoothOnWithPairing->setChecked(true);
            break;
        }
    }
}


/**
 * @brief get the bluetooth reset settings from a line of text.
 * @param line
 */
void SetModeDialog::extractBluetoothResetSettings( QString& line ){

    int bluetoothResetMode = line.trimmed().toInt();
    qDebug() << "bluetoothResetMode: " << bluetoothResetMode;
    if( bluetoothResetMode==1 ){
        m_bluetoothDeletePairingsCheckbox->setChecked(true);
    } else {
        m_bluetoothDeletePairingsCheckbox->setChecked(false);
    }
}


/**
 * @brief get the volume limit settings from a line of text.
 * @param line
 */
void SetModeDialog::extractVolumeLimiterSettings( QString& line ){

    int volumeLimiterMode = line.trimmed().toInt();
    qDebug() << "volumeLimiterMode: " << volumeLimiterMode;
    if( volumeLimiterMode==1 ){
        m_pianissimoMode->setChecked(true);
    } else {
        m_forteMode->setChecked(true);
    }
}


/**
 * @brief get the wifi settings from a line of text.
 * @param line
 */
void SetModeDialog::extractWifiSettings( QString& line ){

    int wifiMode = line.trimmed().toInt();
    qDebug() << "wifiMode: " << wifiMode;
    switch( wifiMode ){
    case 0:
        m_wifiOff->setChecked(true);
        break;
    case 1:
        m_wifiOn->setChecked(true);
        break;
    case 2:
        m_wifiOn->setChecked(true);
        m_readWifiSettingsCheckbox->setChecked(true);
        break;
    }

}


/**
 * @brief get the microphone settings from a line of text.
 * @param line
 */
void SetModeDialog::extractMicrophoneSettings( QString& line ){

    int microphoneMode = line.trimmed().toInt();
    qDebug() << "microphoneMode: " << microphoneMode;
    if( microphoneMode==1 ){
        m_micOn->setChecked(true);
    } else {
        m_micOff->setChecked(true);
    }
}


void SetModeDialog::readIndexM3uSettings(){

    QString indexM3uFileName = m_mainWindow->getCurrentDrivePath() + INDEX_M3U_FILE;

    if( QFile(indexM3uFileName).exists() ){        
        QFile inputFile(indexM3uFileName);
        if (inputFile.open(QIODevice::ReadOnly))
        {
           m_readInfoLabel->setText(tr("These settings have been read from the memory card"));
           QTextStream in(&inputFile);
           while (!in.atEnd())
           {
              QString line = in.readLine();
              QString restOfLine = "";

              if( line.startsWith("#hoerbert:")){

                  if( line.startsWith("#hoerbert:set_mode_sleep_timer_minutes ") ){
                      restOfLine = line.mid(39);
                      extractSleepTimerSettings(restOfLine);
                  }

                  if( line.startsWith("#hoerbert:set_mode_bluetooth ") ){
                      restOfLine = line.mid(29);
                      extractBluetoothSettings(restOfLine);
                  }

                  if( line.startsWith("#hoerbert:set_mode_bluetooth_delete_pairings ") ){
                      restOfLine = line.mid(45);
                      extractBluetoothResetSettings(restOfLine);
                  }

                  if( line.startsWith("#hoerbert:set_mode_volume_limiter ") ){
                      restOfLine = line.mid(34);
                      extractVolumeLimiterSettings(restOfLine);
                  }

                  if( line.startsWith("#hoerbert:set_mode_wifi ") ){
                      restOfLine = line.mid(24);
                      extractWifiSettings(restOfLine);
                  }

                  if( line.startsWith("#hoerbert:set_mode_microphone ") ){
                      restOfLine = line.mid(30);
                      extractMicrophoneSettings(restOfLine);
                  }


              }
           }
           inputFile.close();
        } else {
            m_readInfoLabel->setText(tr("The settings file could not be read from the memory card"));
        }

    } else {
        qDebug() << "File: " << indexM3uFileName << " does not exist.";
        m_readInfoLabel->setText(tr("Default settings: No settings were found on the memory card"));
        m_sleepTimerComboBox->setCurrentIndex(0);
        m_bluetoothOnWithPairing->setChecked(true);
        m_wifiOn->setChecked(true);
        m_micOn->setChecked(true);
    }

}


void SetModeDialog::writeIndexM3uSettings(){

    QString s;
    QString p;
    QString indexM3uFileName = m_mainWindow->getCurrentDrivePath() + INDEX_M3U_FILE;




    this->close();     // close the dialog
}
