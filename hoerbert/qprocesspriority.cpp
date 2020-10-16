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

#include "qprocesspriority.h"

#if defined(Q_OS_WIN)
#include <Windows.h>

static DWORD win32_priority_class( QProcessPriority::Priority p ) {
    switch( p ) {
        case QProcessPriority::IdlePriority:            return IDLE_PRIORITY_CLASS;
        case QProcessPriority::LowestPriority:          // Fallthru
        case QProcessPriority::LowPriority:             return BELOW_NORMAL_PRIORITY_CLASS;
        case QProcessPriority::NormalPriority:          return NORMAL_PRIORITY_CLASS;
        case QProcessPriority::HighPriority:            //Fallthru
        case QProcessPriority::HighestPriority:         return ABOVE_NORMAL_PRIORITY_CLASS;
        case QProcessPriority::TimeCriticalPriority:    return REALTIME_PRIORITY_CLASS;
    }

    return NORMAL_PRIORITY_CLASS;
}
#elif defined(Q_OS_UNIX)
#include <unistd.h>
#include <sys/resource.h>

static int unix_priority_class( QProcessPriority::Priority p ) {
    switch( p ) {
        case QProcessPriority::IdlePriority:            return 19;
        case QProcessPriority::LowestPriority:          return 15;
        case QProcessPriority::LowPriority:             return 10;
        case QProcessPriority::NormalPriority:          return 0;
        case QProcessPriority::HighPriority:            return -10;
        case QProcessPriority::HighestPriority:         return -15;
        case QProcessPriority::TimeCriticalPriority:    return -20;
    }

    return 0;
}
#endif



QProcessPriority::QProcessPriority(QObject *parent /*= nullptr*/) : QProcess( parent ), m_processPriority(NormalPriority) {
#if defined(Q_OS_WIN)
   setCreateProcessArgumentsModifier([this] (QProcess::CreateProcessArguments *args)
    {
        args->flags |= win32_priority_class(this->priority());
    });
#endif
}

void QProcessPriority::setPriority( QProcessPriority::Priority p ) {
    m_processPriority = p;
    if ( state() == QProcess::Running ) {
#if defined(Q_OS_WIN)
        ::SetPriorityClass( (HANDLE)processId(), win32_priority_class(p) );
#elif defined(Q_OS_UNIX)
        ::setpriority(PRIO_PROCESS, processId(), unix_priority_class(p) );
#endif
    }
}

void QProcessPriority::setupChildProcess()
{

#if defined Q_OS_UNIX
    ::nice( unix_priority_class(priority()) );
#endif
}
