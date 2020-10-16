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

#ifndef TRIPLECHECKBOX_H
#define TRIPLECHECKBOX_H

#include <QWidget>

/*!
 * @brief Custom checkbox widget which has three states - unchecked, checked and "3"
 */
class TripleCheckBox : public QWidget
{
    Q_OBJECT
public:

    /**
     * @brief TripleCheckBox constructor
     * @param parent
     */
    explicit TripleCheckBox(QWidget *parent = Q_NULLPTR);

    /**
     * @brief getState
     * @return the state of the checkbox
     */
    int getState();

signals:
    /**
     * @brief clicked
     */
    void clicked();

    /**
     * @brief stateChanged
     */
    void stateChanged(int );

public slots:

private:
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

    int m_state; // 0: unchecked, 1: checked, 2: three marked
    bool m_isPressed;
};

#endif // TRIPLECHECKBOX_H
