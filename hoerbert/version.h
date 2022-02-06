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

#ifndef _WIN32
#include <QtGlobal>
#endif

#ifndef VERSION_H
#define VERSION_H

#define VER_FILEVERSION             0,1,0,0
#define VER_FILEVERSION_STR         "0.1.0.0\0"

#define VER_PRODUCTVERSION          3,1,1,0
#define VER_PRODUCTVERSION_STR      "3.1.1\0"

#define VER_COMPANYNAME_STR         "WINZKI GmbH & CO. KG"
#define VER_LEGALCOPYRIGHT_STR      "Copyright 2019 WINZKI GmbH & Co. KG"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR    VER_LEGALTRADEMARKS1_STR
//directly set in app.rc because of utf-8 problems: #define VER_PRODUCTNAME_STR         "hörbert Software"
//directly set in app.rc because of utf-8 problems: #define VER_FILEDESCRIPTION_STR     "hörbert Software"
//directly set in app.rc because of utf-8 problems: #define VER_INTERNALNAME_STR        "hörbert Software"

#ifdef _WIN32
    #define VER_ORIGINALFILENAME_STR    "hoerbert.exe"
#elif defined Q_OS_MACOS
    #define VER_ORIGINALFILENAME_STR    "hoerbert.app"
#else
    #define VER_ORIGINALFILENAME_STR    "hoerbert.AppImage"
#endif

#define VER_COMPANYDOMAIN_STR       "www.hoerbert.com"

#endif // VERSION_H
