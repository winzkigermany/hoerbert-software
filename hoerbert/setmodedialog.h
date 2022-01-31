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

#ifndef SETMODEDIALOG_H
#define SETMODEDIALOG_H

#include <QDialog>

#include <QLabel>
#include <QLineEdit>
#include <QLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>
#include "mainwindow.h"

class MainWindow;

/**
 * @brief A dialog to set WiFi access data
 */
class SetModeDialog : public QDialog
{
    Q_OBJECT

public:
    SetModeDialog(QWidget* parent = Q_NULLPTR);

signals:

private:
    MainWindow* m_mainWindow;

    QVBoxLayout* m_dialogLayout;
    QHBoxLayout* m_bluetoothHBox;
    QHBoxLayout* m_volumeHBox;
    QHBoxLayout* m_wifiHBox;

    QLabel* m_instructionLabel;
    QLabel* m_readInfoLabel;

    QComboBox* m_sleepTimerComboBox;

    QButtonGroup* m_bluetoothRadioButtons;
    QRadioButton* m_bluetoothOnWithPairing;
    QRadioButton* m_bluetoothOn;
    QRadioButton* m_bluetoothOff;

    QButtonGroup* m_volumeLimiterRadioButtons;
    QRadioButton* m_forteMode;
    QRadioButton* m_pianissimoMode;

    QButtonGroup* m_wifiRadioButtons;
    QRadioButton* m_wifiOn;
    QRadioButton* m_wifiOff;

    QButtonGroup* m_microphoneRadioButtons;
    QRadioButton* m_micOn;
    QRadioButton* m_micOff;

    QCheckBox* m_bluetoothDeletePairingsCheckbox;
    QCheckBox* m_readWifiSettingsCheckbox;

    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;

    void readIndexM3uSettings();
    void writeIndexM3uSettings();
    void showEvent(QShowEvent * event);

    void extractSleepTimerSettings( QString& line );
    void extractBluetoothSettings( QString& line );
    void extractBluetoothResetSettings( QString& line );
    void extractVolumeLimiterSettings( QString& line );
    void extractWifiSettings( QString& line );
    void extractMicrophoneSettings( QString& line );

    bool getSleepTimerSettingsLine( QString* line );
    bool getBluetoothSettingsLine( QString* line );
    bool getBluetoothResetSettingsLine( QString* line );
    bool getVolumeLimiterSettingsLine( QString* line );
    bool getWifiSettingsLine( QString* line );
    bool getMicrophoneSettingsLine( QString* line );


};

#endif // SETMODEDIALOG_H
