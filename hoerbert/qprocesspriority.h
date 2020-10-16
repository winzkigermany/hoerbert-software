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

#ifndef QPROCESSPRIORITY_H
#define QPROCESSPRIORITY_H

#include <QProcess>

/**
 * @brief The QProcessPriority class provides additional method to set CPU priority
 */
class QProcessPriority : public QProcess {
    Q_OBJECT

public:
    enum Priority {
        IdlePriority,
        LowestPriority,
        LowPriority,
        NormalPriority,
        HighPriority,
        HighestPriority,
        TimeCriticalPriority
    };
    Q_ENUM(Priority);

    explicit QProcessPriority(QObject *parent = nullptr);

    /**
     * @brief setPriority set process priority
     */
    void setPriority( Priority );

    /**
     * @brief priority gets process's current priority
     * @return
     */
    Priority priority() const { return m_processPriority; };

protected:
     void setupChildProcess() override;

private:

    Priority m_processPriority;
    Q_DISABLE_COPY(QProcessPriority);
};

#endif // QPROCESSPRIORITY_H
