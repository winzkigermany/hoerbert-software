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

#include "recordingselector.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QPushButton>

RecordingSelector::RecordingSelector(QWidget *parent, int id) : QWidget(parent)
{
    setObjectName("RecordingSelector");

#ifdef Q_OS_MACOS
    setFont(QFont(HOERBERT_FONTFAMILY, 21, QFont::DemiBold));
#elif defined (Q_OS_LINUX)
    setFont(QFont(HOERBERT_FONTFAMILY, 18, QFont::Normal));
#elif defined(Q_OS_WIN)
    setFont(QFont(HOERBERT_FONTFAMILY, 18, QFont::DemiBold));
#endif

    m_id = id;      // id of the playlist this selector belongs to

    m_allowMicrophone = new QPushButton(this);
    m_allowMicrophone->setCheckable(true);
    m_allowMicrophone->setIcon(QIcon(":/images/rec_mic_inactive_512.png"));
    m_allowMicrophone->setIconSize(QSize(16, 16));
    m_allowMicrophone->setStyleSheet("background: transparent; border: none");
    connect( m_allowMicrophone, &QCheckBox::clicked, this, [=] (){
        emit selectedMicrophone( m_id, m_allowMicrophone->isChecked());
        emit valuesHaveChanged();
    });


    m_allowBluetooth = new QPushButton(this);
    m_allowBluetooth->setCheckable(true);
    m_allowBluetooth->setIcon(QIcon(":/images/rec_bt_inactive_512.png"));
    m_allowBluetooth->setIconSize(QSize(16, 16));
    m_allowBluetooth->setStyleSheet("background: transparent; border: none");
    connect( m_allowBluetooth, &QCheckBox::clicked, this, [=] (){
        emit selectedBluetooth( m_id, m_allowBluetooth->isChecked());
        emit valuesHaveChanged();
    });


    m_allowWifi = new QPushButton(this);
    m_allowWifi->setCheckable(true);
    m_allowWifi->setIcon(QIcon(":/images/rec_wifi_inactive_512.png"));
    m_allowWifi->setIconSize(QSize(16, 16));
    m_allowWifi->setStyleSheet("background: transparent; border: none");

    connect( m_allowWifi, &QCheckBox::clicked, this, [=] (){
        emit selectedWifi( m_id, m_allowWifi->isChecked());
        emit valuesHaveChanged();
    });

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget( m_allowMicrophone );
    layout->addWidget( m_allowWifi );
    layout->addWidget( m_allowBluetooth );

    setLayout( layout );
}


void RecordingSelector::setID(qint8 id)
{
    assert(id >= 0);
    m_id = id;
}

qint8 RecordingSelector::ID()
{
    return m_id;
}


void RecordingSelector::setMicrophone( bool yesNo ){
    m_allowMicrophone->setChecked(yesNo);
    if( yesNo ){
        m_allowMicrophone->setIcon(QIcon(":/images/rec_mic_active_512.png"));
    } else {
        m_allowMicrophone->setIcon(QIcon(":/images/rec_mic_inactive_512.png"));
    }
}

bool RecordingSelector::getMicrophone(){
    return m_allowMicrophone->isChecked();
}

void RecordingSelector::setWifi( bool yesNo ){
    m_allowWifi->setChecked(yesNo);
    if( yesNo ){
        m_allowWifi->setIcon(QIcon(":/images/rec_wifi_active_512.png"));
    } else {
        m_allowWifi->setIcon(QIcon(":/images/rec_wifi_inactive_512.png"));
    }
}

bool RecordingSelector::getWifi(){
    return m_allowWifi->isChecked();
}

void RecordingSelector::setBluetooth( bool yesNo ){
    m_allowBluetooth->setChecked(yesNo);
    if( yesNo ){
        m_allowBluetooth->setIcon(QIcon(":/images/rec_bt_active_512.png"));
    } else {
        m_allowBluetooth->setIcon(QIcon(":/images/rec_bt_inactive_512.png"));
    }
}

bool RecordingSelector::getBluetooth(){
    return m_allowBluetooth->isChecked();
}
