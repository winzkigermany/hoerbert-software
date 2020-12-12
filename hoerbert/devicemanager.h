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

#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <memory>
#include <map>

#include <QList>
#include <QObject>
#include <QProcess>

#include "define.h"
#include "pleasewaitdialog.h"
#include "processexecutor.h"

class QStorageInfo;
class PleaseWaitDialog;

struct VolumeInfo;
typedef std::shared_ptr<VolumeInfo> VolumeInfo_ptr;
typedef QStringList ListString;
typedef std::map<QString, VolumeInfo_ptr> MapString2VolumeInfo;

const QString DEFAULT_DRIVE_LABEL   = "HOERBERT";

/**
 * @brief The DeviceManager class provides methods to manage removable devices
 */
class DeviceManager : public QObject
{
    Q_OBJECT
public:
    DeviceManager();

    /**
     * @brief getDeviceList
     * @return a list of available removable devices.
     *
     * It is platform-dependent. For example, on windows, it uses windows API to carryout its function.
     */
    ListString getDeviceList();

    /**
     * @brief formatDrive format given drive to FAT32 with default label
     * @param driveName drive label + root path
     * @param passwd current user's password in case permission is required
     * @return result code which indicates success, failure or incorrect password
     */
    RetCode formatDrive( QWidget* parentWidget, const QString &driveName, const QString &newLabel = QString(), const QString &passwd = QString());


    /**
     * @brief ejectDrive eject given drive (involves unmount and NO power-off)
     * @param driveName drive label + root path
     * @return result code
     */
    RetCode ejectDrive(const QString &driveName);

    /**
     * @brief remountDrive try to ask automount to remount given drive
     * @param driveName drive label + root path
     * @return result code
     */
    RetCode remountDrive(const QString &driveName);

    /**
     * @brief getVolumeSize
     * @param driveName
     * @return the volume size in bytes, 0 means volume not found
     */
    qint64 getVolumeSize(const QString &driveName);

    /**
     * @brief getPlaylistSize
     * @param playlistPath
     * @return the playlist size in bytes
     */
    qint64 getPlaylistSize(const QString playlistPath);

    /**
     * @brief getAvailableSize
     * @param driveName
     * @return available size of volume in bytes
     */
    qint64 getAvailableSize(const QString &driveName);

    /**
     * @brief getVolumeFileSystem
     * @param driveName
     * @return volume file system ("" mean not known)
     */
    QString getVolumeFileSystem(const QString &driveName);

    /**
     * @brief hasFat32FileSystem
     * @param driveName
     * @return true if the drive has a fat32 file system
     */
    bool hasFat32FileSystem(const QString &driveName);

    /**
     * @brief check if the provided drive is write protected
     * @param driveName
     * @return
     */
    bool isWriteProtected(const QString &driveName);

    /**
     * @brief set current drive name to class variable
     * @param driveName
     */
    void setCurrentDrive(const QString &driveName);

    /**
     * @brief DeviceManager::getDrivePath
     * @return path of given drive on system. If no drive is given, return the path of the current drive.
     *
     * If no drive is given, return the full drive path of the current drive.
     */
    QString getDrivePath(const QString &driveName) const;

    /**
     * @brief getDriveName
     * @param drivePath absolute path to mounted drive storage
     * @return formatted drive name
     */
    QString getDriveName(const QString &drivePath);

    /**
     * @brief refresh the underlying StorageObject
     */
    void refresh( const QString &driveName );

    /**
     * @brief isWorkingOnCustomDirectory
     * @return true if the user manually selected a path to work on instead of selecting a drive.
     */
    bool isWorkingOnCustomDirectory() const;

#if defined (Q_OS_MACOS) || defined (Q_OS_LINUX)
    /**
     * @brief check if device is valid(removable drive) or not
     * @param device
     * @return
     *
     * Only fot macOS
     */
    bool isValidDevice(const QString &device);
#endif

    /**
     * @brief addCustomPath adds the user selected path as a possible drive to work on
     * @param customPath an absolute path
     */
    void addCustomPath( const QString &customPath );

protected:

    /**
     * @brief check if the given drive is removable or not
     * @param volumeRoot
     * @return true if removable
     */
    bool isRemovable(const QString &volumeRoot);

    /**
     * @brief check if the provided driver is ejectable
     * @param driveName
     * @return true if ejectable
     */
    bool isEjectable(const QString &driveName);

    /**
     * @brief getFormatCommand returns platform specific command used to format drive
     * @param root root path of device
     * @param driveLabel label to be set to drive
     * @return
     */
    QString getFormatCommand(const QString &root, const QString &driveLabel);

    /**
     * @brief DeviceManager::getRoot
     * @param info
     * @return the root directory of a storage device
     */
    QString getRoot(const QStorageInfo &info);

private:
    /**
     * @brief a mapping of deviceName->root
     */
    MapString2VolumeInfo _deviceName2Root;

    /**
     * @brief the currently selected drive
     */
    QString m_currentDrive;

    /**
     * @brief a custom path that was selected by the user manually
     */
    QString m_custom_destination_path;

    /**
     * @brief pleaseWait A universal "please wait" dialog
     */
    PleaseWaitDialog* pleaseWait;

    /**
     * @brief formatProcess is a longer running process that shoul not block the main loop
     */
    QProcess formatProcess;        // this is a potentially long running process

    ProcessExecutor m_processExecutor;

};


#endif // DEVICEMANAGER_H
