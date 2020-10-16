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

#ifndef XMLWRITER_H
#define XMLWRITER_H

#include <QObject>

#include "define.h"

/**
 * @brief The XmlWriter class
 */
class XmlWriter
{
public:
    /**
     * @brief XmlWriter constructor
     * @param path absolute path xml file to be created
     * @param fileList list of QFileInfo
     */
    XmlWriter(const QString &path, const AudioList &audioInfoList, bool isDiagmode = false);

    /**
     * @brief create hoerbert.xml(old one is replaced with new one)
     * @return true if no error
     */
    bool create();

    /**
     * @brief getDirectoryNumber gets directory number from absolute path of audio file
     * @param path absolute path to the file
     * @return directory number as integer
     */
    int getDirectoryNumber(const QString &path);

private:
    QString m_path;
    AudioList m_audioInfoList;
    bool m_isDiagnosticsMode;
};

#endif // XMLWRITER_H
