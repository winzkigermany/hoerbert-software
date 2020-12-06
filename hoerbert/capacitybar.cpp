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

#include "capacitybar.h"

#include <QDebug>
#include <QPainter>

#include "define.h"
#include "functions.h"

CapacityBar::CapacityBar(QWidget *parent)
    : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    setFont(QFont(HOERBERT_FONTFAMILY, 12));
    setObjectName("CardSizeBar");
    setStyleSheet(BAR_STYLE_TEMPLATE);
    setFixedSize(300, 25);

    m_totalBytes = 0;
    m_usedBytes = 0;
    setText(tr("Please select a device first."));
    setToolTip(tr("Shows used space / total available space on the card in minutes"));
}

void CapacityBar::setParams(quint64 usedBytes, quint64 totalBytes)
{
    m_usedBytes = usedBytes;
    m_totalBytes = totalBytes;

    if( m_usedBytes>m_totalBytes)
    {
        m_usedBytes = m_totalBytes;
    }

    update();
}

void CapacityBar::paintEvent(QPaintEvent *e)
{
    if (m_totalBytes > 0)
        setText(QString("~ %1 / %2 min").arg(bytesToSeconds(m_usedBytes) / 60 ).arg(bytesToSeconds(m_totalBytes)/60));
    else
        setText(tr("Please select a device first."));

    int percentage =  int(double(m_usedBytes) / double(m_totalBytes) * 100);

    if( !isEnabled() ){
        setText("");
        percentage=100;
    }

    if (percentage > 0 || (m_usedBytes > 0 && percentage == 0))
    {
        QPainter painter(this);

        if (percentage > 95) {
            painter.setBrush(QColor(225, 0, 0));
        } else if (percentage > 90) {
            painter.setBrush(QColor(225, 225, 0));
        } else {
            painter.setBrush(QColor(0, 225, 0));
        }
        painter.setPen(Qt::NoPen);

        if (percentage == 0)
            painter.drawRoundedRect(1, 1, 1, height() - 2, 2, 2);
        else
            painter.drawRoundedRect(1, 1, width() * percentage / 100 - 1, height() - 2, 2, 2);
    }
    QLabel::paintEvent(e);
}
