/***************************************************************************
 * hörbert Software
 * Copyright (C) 2022 WINZKI GmbH & Co. KG
 *
 * Author: Rainer Brang
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

#ifndef RECORDINGSELECTOR_H
#define RECORDINGSELECTOR_H

#include <QWidget>
#include <QApplication>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>

#include "define.h"

/**
 * @brief Custom styled button with pie progress
 */
class RecordingSelector : public QWidget
{
    Q_OBJECT

public:

    /**
     * @brief RecordingSelector constructor
     * @param parent
     */
    explicit RecordingSelector(QWidget *parent = Q_NULLPTR, int id = -1);

    /**
     * @brief set the button/directory/playlist id for this button
     */
    void setID(qint8 );

    qint8 ID();    

    bool getMicrophone();
    bool getWifi();
    bool getBluetooth();
    void setMicrophone( bool yesNo );
    void setWifi( bool yesNo );
    void setBluetooth( bool yesNo );

signals:
    void selectedBluetooth( quint8 index, bool onOff );
    void selectedMicrophone( quint8 index, bool onOff );
    void selectedWifi( quint8 index, bool onOff );
    void valuesHaveChanged();

public slots:

private:
    int m_id;

    QPushButton* m_allowMicrophone;
    QPushButton* m_allowWifi;
    QPushButton* m_allowBluetooth;
};

#endif // RECORDINGSELECTOR_H
