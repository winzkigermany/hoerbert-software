/*************************************************************************************
 * RemoveDriveByLetter.cpp by Uwe Sieber - www.uwe-sieber.de
 *
 * Simple demonstration how to prepare a disk drive for save removal
 *
 * Works with removable and fixed drives under W2K, XP, W2K3, Vista
 *
 * Console application - expects the drive letter of the drive to remove as parameter
 *
 * you are free to use this code in your projects
 *************************************************************************************/

#ifndef HELPER_H
#define HELPER_H

#ifdef _WIN32
#include <tchar.h>

int EjectDriveWin(TCHAR DriveLetter);

#endif

#endif // HELPER_H
