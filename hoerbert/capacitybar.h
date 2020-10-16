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

#ifndef CAPACITYBAR_H
#define CAPACITYBAR_H

#include <QLabel>

/**
 * @brief Custom widget to show the rough values of used space and total space of memory card in minutes
 */
class CapacityBar : public QLabel
{
    Q_OBJECT
public:

    /**
     * @brief CapacityBar constructor
     * @param parent
     */
    explicit CapacityBar(QWidget *parent = Q_NULLPTR);

    /**
     * @brief set the used size and the total size
     * @param used_size in bytes
     * @param total_size in bytes
     */
    void setParams(quint64 usedSize, quint64 totalSize);

    /**
     * @brief update used space only
     * @param usedSize used space in bytes
     */
    void updateUsedSpace(quint64 usedSize);

    /**
     * @brief updateUsedSpaceInSeconds
     * @param sizeInMinutes the amount of seconds need to be added or subtracted
     */
    void addSpaceInSeconds(int sizeInSeconds);

    /**
     * @brief reset estimated capacity
     */
    void resetEstimation();

    /**
     * @brief estimatedSeconds
     * @return
     */
    quint64 estimatedSeconds();

private:

    void paintEvent(QPaintEvent *);

    quint64 m_usedBytes; // used size in bytes
    quint64 m_totalBytes; // total size in bytes
    quint64 m_estimatedSeconds; // size in seconds
};

#endif // CAPACITYBAR_H
