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

#include "triplecheckbox.h"

#include <QPainter>
#include <QDebug>

TripleCheckBox::TripleCheckBox(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(20, 20);
    m_state = 0;
    m_isPressed = false;
}

int TripleCheckBox::getState()
{
    return m_state;
}

void TripleCheckBox::mousePressEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);
    m_isPressed = true;
}

void TripleCheckBox::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);
    if (m_isPressed) {
        emit clicked();
        m_state = m_state < 2 ? m_state + 1 : 0;
        emit stateChanged(m_state);
        m_isPressed = false;
        update();
    }
}

void TripleCheckBox::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    const QPoint x1(5, 18);
    const QPoint x2(13, 27);
    const QPoint x3(27, 5);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    int side = qMin(width(), height());
    painter.scale(side / 32.0, side / 32.0);

    painter.setFont(QFont("Monospace", 25, QFont::DemiBold));

    painter.setPen(QPen(Qt::black, 2, Qt::SolidLine));
    painter.setBrush(QBrush(Qt::white));
    painter.drawRect(QRect(1, 1, 30, 30));

    QPainterPath checkPath;
    checkPath.moveTo(x1);
    checkPath.lineTo(x2);
    checkPath.lineTo(x3);

    switch (m_state)
    {
    case 0:
        this->setToolTip(tr("Cut up at silence or in 3-minute-chunks?"));
        break;
    case 1:
        painter.setPen(QPen(Qt::black, 3, Qt::SolidLine));
        painter.drawPath(checkPath);
        this->setToolTip(tr("Cut at silent parts, at most every 3 minutes"));
        break;
    case 2:
        painter.setPen(QColor(255, 0, 0));
        painter.drawText(QRectF(0, 0, 30, 30), Qt::AlignCenter, "3");
        this->setToolTip(tr("Cut in chunks of 3 minutes"));
        break;
    }
}
