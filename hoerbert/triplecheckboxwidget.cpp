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

#include "triplecheckboxwidget.h"

#include <QHBoxLayout>

TripleCheckBoxWidget::TripleCheckBoxWidget(QWidget *parent, int id)
    : QWidget(parent)
{
    m_id = id;
    m_box = new TripleCheckBox(this);

    m_layout = new QHBoxLayout(this);
    m_layout->setAlignment(Qt::AlignCenter);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_box);

    connect(m_box, &TripleCheckBox::clicked, [this]() {
        emit clicked(m_id);
    });

    connect(m_box, &TripleCheckBox::stateChanged, [this](qint8 state) {
        emit stateChanged(m_id, state);
    });
}
