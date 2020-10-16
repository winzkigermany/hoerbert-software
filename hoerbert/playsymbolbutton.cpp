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

#include "playsymbolbutton.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>

PlaySymbolButton::PlaySymbolButton(QWidget *parent, int id) : QWidget(parent)
{
    m_ID = id;
    m_isPlaying = false;

    if (id == -1)
    {
        qDebug() << "Warning. ID should be specified for each symbol buttons.";
    }

    m_pic = new QLabel(this);
    m_pic->setPixmap(QPixmap(":/images/play.png"));
    m_pic->setScaledContents(true);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_layout->addWidget(m_pic, 0, Qt::AlignCenter);
}
void PlaySymbolButton::play()
{
    m_isPlaying = true;
    updateState();
}

void PlaySymbolButton::stop()
{
    m_isPlaying = false;
    updateState();
}

void PlaySymbolButton::setButtonSize(int w, int h)
{
    m_pic->setFixedSize(w, h);
}

bool PlaySymbolButton::isPlaying()
{
    return m_isPlaying;
}

void PlaySymbolButton::updateState()
{
    if (m_isPlaying)
        m_pic->setPixmap(QPixmap(":/images/stop.png"));
    else
        m_pic->setPixmap(QPixmap(":/images/play.png"));

    emit stateChanged(m_ID, m_isPlaying);
}

void PlaySymbolButton::mousePressEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);
    m_isPressed = true;
}
void PlaySymbolButton::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);
    if (m_isPressed)
    {
        emit clicked();
        m_isPlaying = !m_isPlaying;
        updateState();
    }
}
