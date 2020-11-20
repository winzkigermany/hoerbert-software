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
#include "scaledsizeprovider.h"

CapacityBar::CapacityBar(QWidget *parent)
    : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);

    setFont(QFont(HOERBERT_FONTFAMILY, ScaledSizeProvider::getFontSizeFactor()*14));
    setObjectName("CardSizeBar");
    setStyleSheet(BAR_STYLE_TEMPLATE);
    setFixedSize( ScaledSizeProvider::getScaledSize(300, 25) );

    m_totalBytes = 0;
    m_usedBytes = 0;
    m_estimatedSeconds = 0;
    setToolTip(tr("Shows used space / total available space on the card in minutes"));

//    QFont f = font();
//    f.setPointSize(6);
//    setFont(f);
}

void CapacityBar::setParams(quint64 usedBytes, quint64 totalBytes)
{
    m_usedBytes = usedBytes;
    m_totalBytes = totalBytes;

    assert(m_usedBytes <= m_totalBytes);

    update();
}

void CapacityBar::updateUsedSpace(quint64 usedSize)
{
    setParams(usedSize, m_totalBytes);
}

void CapacityBar::addSpaceInSeconds(int sizeInSeconds)
{
    if (m_estimatedSeconds == 0)
        m_estimatedSeconds = bytesToSeconds(m_usedBytes);

    m_estimatedSeconds = int(m_estimatedSeconds) + int(sizeInSeconds);

    update();
}

void CapacityBar::resetEstimation()
{
    m_estimatedSeconds  = 0;
    update();
}

quint64 CapacityBar::estimatedSeconds()
{
    return m_estimatedSeconds;
}

void CapacityBar::paintEvent(QPaintEvent *e)
{
    if (m_totalBytes > 0)
        setText(QString("~ %1 / %2 min").arg((m_estimatedSeconds == 0) ? bytesToSeconds(m_usedBytes) / 60 : m_estimatedSeconds / 60).arg(bytesToSeconds(m_totalBytes)/60));
    else
        setText(tr("No memory card"));

    int percentage =  (m_estimatedSeconds == 0) ? int(double(m_usedBytes) / double(m_totalBytes) * 100) : int(double(secondsToBytes(m_estimatedSeconds)) / double(m_totalBytes) * 100);

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
