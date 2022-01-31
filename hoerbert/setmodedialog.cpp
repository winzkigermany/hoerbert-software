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
    m_sleepTimerComboBox->setEditable(true);
    m_sleepTimerComboBox->setValidator( new QIntValidator(1, 35000000, this) );     // must come after setEditable(true)
    for( int i=10; i<130; i+=10 ){
        m_sleepTimerComboBox->addItem( QString::number(i), i );
    }
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
    formLayout->addRow(new QLabel(tr("WiFi:")), m_wifiHBox);
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

#define SET_SETTING_SLEEP_TIMER_MINUTES "#hoerbert:set_mode_sleep_timer_minutes "
#define SET_SETTING_BLUETOOTH_MODE "#hoerbert:set_mode_bluetooth "
#define SET_SETTING_DELETE_PAIRINGS "#hoerbert:set_mode_bluetooth_delete_pairings "
#define SET_SETTING_VOLUME_LIMITER "#hoerbert:set_mode_volume_limiter "
#define SET_SETTING_WIFI_SETTINGS "#hoerbert:set_mode_wifi "
#define SET_SETTING_MICROPHONE_MODE "#hoerbert:set_mode_microphone "

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

                  if( line.startsWith(SET_SETTING_SLEEP_TIMER_MINUTES) ){
                      restOfLine = line.mid(39);
                      extractSleepTimerSettings(restOfLine);
                  }

                  if( line.startsWith(SET_SETTING_BLUETOOTH_MODE) ){
                      restOfLine = line.mid(29);
                      extractBluetoothSettings(restOfLine);
                  }

                  if( line.startsWith(SET_SETTING_DELETE_PAIRINGS) ){
                      restOfLine = line.mid(45);
                      extractBluetoothResetSettings(restOfLine);
                  }

                  if( line.startsWith(SET_SETTING_VOLUME_LIMITER) ){
                      restOfLine = line.mid(34);
                      extractVolumeLimiterSettings(restOfLine);
                  }

                  if( line.startsWith(SET_SETTING_WIFI_SETTINGS) ){
                      restOfLine = line.mid(24);
                      extractWifiSettings(restOfLine);
                  }

                  if( line.startsWith(SET_SETTING_MICROPHONE_MODE) ){
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


/**
 * @brief Output the sleep timer setting as a string
 * @param line
 * @return true if it worked.
 */
bool SetModeDialog::getSleepTimerSettingsLine( QString* line ){

    if( m_sleepTimerComboBox->currentIndex()>0 ){
        QString currentValue = m_sleepTimerComboBox->currentText();
        line->append( SET_SETTING_SLEEP_TIMER_MINUTES+QString::number(currentValue.toUInt()) );
        return true;
    } else {
        line->append( SET_SETTING_SLEEP_TIMER_MINUTES+QString::number(0) );
        return true;
    }
    return false;
}


/**
 * @brief Output the bluetooth setting as a string
 * @param line
 * @return true if it worked.
 */
bool SetModeDialog::getBluetoothSettingsLine( QString* line ){

    if( m_bluetoothOn->isChecked() ){
        line->append( SET_SETTING_BLUETOOTH_MODE+QString::number(1) );
        return true;
    } else if( m_bluetoothOnWithPairing->isChecked() ){
        line->append( SET_SETTING_BLUETOOTH_MODE+QString::number(2) );
        return true;
    } else if( m_bluetoothOff->isChecked() ){
        line->append( SET_SETTING_BLUETOOTH_MODE+QString::number(0) );
        return true;
    }
    return false;
}


/**
 * @brief Output the bluetooth reset setting as a string
 * @param line
 * @return true if it worked.
 */
bool SetModeDialog::getBluetoothResetSettingsLine( QString* line ){

    if( m_bluetoothDeletePairingsCheckbox->isChecked() ){
        line->append( SET_SETTING_DELETE_PAIRINGS+QString::number(1) );
        return true;
    }

    return false;
}


/**
 * @brief Output the volume limiter setting as a string
 * @param line
 * @return true if it worked.
 */
bool SetModeDialog::getVolumeLimiterSettingsLine( QString* line ){

    if( m_pianissimoMode->isChecked() ){
        line->append( SET_SETTING_VOLUME_LIMITER+QString::number(1) );
        return true;
    } else if( m_forteMode->isChecked() ) {
        line->append( SET_SETTING_VOLUME_LIMITER+QString::number(0) );
        return true;
    }

    return false;
}


/**
 * @brief Output the wifi setting as a string
 * @param line
 * @return true if it worked.
 */
bool SetModeDialog::getWifiSettingsLine( QString* line ){

    if( m_readWifiSettingsCheckbox->isChecked() ){
        line->append( SET_SETTING_WIFI_SETTINGS+QString::number(2) );
        return true;
    } else {
        if( m_wifiOn->isChecked() ){
            line->append( SET_SETTING_WIFI_SETTINGS+QString::number(1) );
            return true;
        } else if( m_wifiOff->isChecked() ){
            line->append( SET_SETTING_WIFI_SETTINGS+QString::number(0) );
            return true;
        }
    }

    return false;
}


/**
 * @brief Output the microphone setting as a string
 * @param line
 * @return true if it worked.
 */
bool SetModeDialog::getMicrophoneSettingsLine( QString* line ){

    if( m_micOn->isChecked() ){
        line->append( SET_SETTING_MICROPHONE_MODE+QString::number(1) );
        return true;
    } else if( m_micOff->isChecked() ) {
        line->append( SET_SETTING_MICROPHONE_MODE+QString::number(0) );
        return true;
    }

    return false;
}

/**
 * @brief write our index.m3u and add/modify the settings from this dialog.
 * We write the file to a temp file and on sucess overwrite the original file.
 */
void SetModeDialog::writeIndexM3uSettings(){

    QString indexM3uFileName = m_mainWindow->getCurrentDrivePath() + INDEX_M3U_FILE;

    bool foundSleepTimerSettings = false;
    bool foundBluetoothSettings = false;
    bool foundBluetoothResetSettings = false;
    bool foundVolumeLimiterSettings = false;
    bool foundWifiSettings = false;
    bool foundMicrophoneSettings = false;

    QFile inputFile(indexM3uFileName);
    QFile outputFile(indexM3uFileName+".tmp");
    QString outLine = "";

    bool writeSuccess = false;
    if( outputFile.open(QIODevice::WriteOnly)){

        if (inputFile.open(QIODevice::ReadOnly)){       // an input file exists. If we find our settings, modify them.
            QTextStream in(&inputFile);
            while (!in.atEnd())
            {
                outLine = "";
                QString line = in.readLine();
                bool handledLineInSomeWay = false;

                if( line.startsWith(SET_SETTING_SLEEP_TIMER_MINUTES) ){

                    if( !foundSleepTimerSettings  ){
                        if( getSleepTimerSettingsLine(&outLine) ){
                            outputFile.write(outLine.toUtf8()+"\n");
                        }
                    }
                    foundSleepTimerSettings = true;
                    handledLineInSomeWay = true;
                }

                if( line.startsWith(SET_SETTING_BLUETOOTH_MODE) ){
                    if( !foundBluetoothSettings ){
                        if( getBluetoothSettingsLine(&outLine) ){
                            outputFile.write(outLine.toUtf8()+"\n");
                        }
                    }
                    foundBluetoothSettings = true;
                    handledLineInSomeWay = true;
                }

                if( line.startsWith(SET_SETTING_DELETE_PAIRINGS) ){
                    if( !foundBluetoothResetSettings ){
                        if( getBluetoothResetSettingsLine(&outLine) ){
                            outputFile.write(outLine.toUtf8()+"\n");
                        }
                    }
                    foundBluetoothResetSettings = true;
                    handledLineInSomeWay = true;
                }

                if( line.startsWith(SET_SETTING_VOLUME_LIMITER) ){
                    if( !foundVolumeLimiterSettings ){
                        if( getVolumeLimiterSettingsLine(&outLine)){
                            outputFile.write(outLine.toUtf8()+"\n");
                        }
                    }
                    foundVolumeLimiterSettings = true;
                    handledLineInSomeWay = true;
                }

                if( line.startsWith(SET_SETTING_WIFI_SETTINGS) ){
                    if( !foundWifiSettings ){
                        if( getWifiSettingsLine(&outLine) ){
                            outputFile.write(outLine.toUtf8()+"\n");
                        }
                    }
                    foundWifiSettings = true;
                    handledLineInSomeWay = true;
                }

                if( line.startsWith(SET_SETTING_MICROPHONE_MODE) ){
                    if( !foundMicrophoneSettings ){
                        if( getMicrophoneSettingsLine(&outLine) ){
                            outputFile.write(outLine.toUtf8()+"\n");
                        }
                    }
                    foundMicrophoneSettings = true;
                    handledLineInSomeWay = true;
                }

                if( !handledLineInSomeWay ){
                    // just copy the line
                    outputFile.write( line.toUtf8()+"\n" );
                }
            }

            inputFile.close();
            if( QFile::exists(indexM3uFileName+".bak") ){
                QFile::remove(indexM3uFileName+".bak");
            }
            inputFile.rename(indexM3uFileName+".bak");
        }

        // maybe some settings were not found OR maybe there was no input file at all.
        outLine = "";
        if( !foundSleepTimerSettings && getSleepTimerSettingsLine(&outLine) ){
            outputFile.write( outLine.toUtf8()+"\n" );
        }

        outLine = "";
        if( !foundBluetoothSettings && getBluetoothSettingsLine(&outLine) ){
            outputFile.write( outLine.toUtf8()+"\n" );
        }

        outLine = "";
        if( !foundBluetoothResetSettings && getBluetoothResetSettingsLine(&outLine) ){
            outputFile.write( outLine.toUtf8()+"\n" );
        }

        outLine = "";
        if( !foundVolumeLimiterSettings && getVolumeLimiterSettingsLine(&outLine) ){
            outputFile.write( outLine.toUtf8()+"\n" );
        }

        outLine = "";
        if( !foundWifiSettings && getWifiSettingsLine(&outLine) ){
            outputFile.write( outLine.toUtf8()+"\n" );
        }

        outLine = "";
        if( !foundMicrophoneSettings && getMicrophoneSettingsLine(&outLine) ){
            outputFile.write( outLine.toUtf8()+"\n" );
        }

        writeSuccess = true;

        // we've got our output file with ".tmp" appended to its name.
        // now rename it to its real file name, and overwrite the original.
        if( !outputFile.rename(indexM3uFileName) ){
            QMessageBox::warning(this, tr("Unable to write index.m3u"), tr("Failed to write the index.m3u file to the memory card. Changing hörbert settings will not work after this try. Error was: %1").arg( outputFile.errorString() ) );
        }
    }

    this->close();     // close the dialog
}
