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

#include "devicemanager.h"
#include "define.h"
#include "pleasewaitdialog.h"

#include <QStorageInfo>
#include <QDebug>
#include <QProcess>
#include <QProgressDialog>
#include <QProgressBar>
#include <QInputDialog>
#include <QLineEdit>
#include <QApplication>
#include <QDirIterator>


#ifdef _WIN32
#include <tchar.h>
#include <windows.h>
#include <direct.h>
#include <shlobj.h>
#include "helper.h"
#endif


struct VolumeInfo{

    VolumeInfo(const QString &n, const QStorageInfo &info):
    name(n),
    storageInfo(info){

    }
    QString name;
    QStorageInfo storageInfo;
};

DeviceManager::DeviceManager()
{
    m_currentDrive = "";
    // Get the list of available drives right from the start.
    getDeviceList();
}

void DeviceManager::refresh( const QString &driveName )
{
    std::map<QString, VolumeInfo_ptr>::const_iterator ret;
    if( !driveName.isEmpty() )
    {
        ret = _deviceName2Root.find(driveName);
        ret->second->storageInfo.refresh();
    }
}

bool DeviceManager::isRemovable(const QString &volumeRoot)
{
#ifdef _WIN32
    auto root = reinterpret_cast<LPCWSTR>(volumeRoot.utf16());
    UINT type = GetDriveType(root);
    return (type == DRIVE_REMOVABLE);
#elif defined (Q_OS_MACOS)
    QString cmd = "diskutil info " + volumeRoot;
    QString output = m_processExecutor.executeCommand(cmd).second;

    return output.contains(QRegExp("Removable Media:.*Removable", Qt::CaseInsensitive));
#elif defined (Q_OS_LINUX)
    QString cmd = "udevadm info --query=property --export --name=" + volumeRoot;
    QString output = m_processExecutor.executeCommand(cmd).second;

    return output.contains(QRegExp("ID_USB_DRIVER='usb-storage'", Qt::CaseSensitive));
#endif
}

bool DeviceManager::isEjectable(const QString &volumeRoot)
{
#ifdef _WIN32
    auto root = reinterpret_cast<LPCWSTR>(volumeRoot.utf16());
    UINT type = GetDriveType(root);
    return (type != DRIVE_FIXED);
#elif defined (Q_OS_MACOS)
    QString cmd = "diskutil info " + volumeRoot;
    QString output = m_processExecutor.executeCommand(cmd).second;

    return output.contains(QRegExp("Ejectable", Qt::CaseInsensitive));
#elif defined (Q_OS_LINUX)
    QString cmd = "udevadm info --query=property --export --name=" + volumeRoot;
    QString output = m_processExecutor.executeCommand(cmd).second;

    return output.contains(QRegExp("ID_USB_DRIVER='usb-storage'", Qt::CaseSensitive));
#endif
}

ListString DeviceManager::getDeviceList()
{
    ListString  driveList;
    driveList.clear();

    QList<QStorageInfo> volumeList= QStorageInfo::mountedVolumes();

    for(const auto& v: volumeList)
    {
        QString root = getRoot(v);

        if(v.isValid() && v.isReady() && isRemovable(root))
        {
            QString diskName = QObject::tr("USB Drive");
            if (v.name() != "")
            {
                diskName = v.displayName();
            }

            QString volume = diskName + " (" + root + ")";
            driveList.push_back(volume);

            VolumeInfo_ptr vol=std::make_shared<VolumeInfo>(root,v);
            _deviceName2Root.emplace(volume,vol);
        }
    }

    if( isWorkingOnCustomDirectory() ){
        QStorageInfo qsi(m_custom_destination_path);
        QString root = getRoot( qsi );
        driveList.push_back(m_custom_destination_path);

        VolumeInfo_ptr vol=std::make_shared<VolumeInfo>(root,qsi);
        _deviceName2Root.emplace(m_custom_destination_path, vol);
    }

  return driveList;
}

QString DeviceManager::getRoot(const QStorageInfo &info)
{
#ifdef _WIN32
    return info.rootPath();
#else
    return info.device();
#endif
}

QString DeviceManager::getDrivePath(const QString &driveName) const
{
    if( isWorkingOnCustomDirectory() ){
        return m_custom_destination_path;
    }

    if( driveName.isEmpty() )
    {
        return "";
    }

    auto drive = _deviceName2Root.find(driveName);

    QString path = drive->second->storageInfo.rootPath();
    return path;
}

QString DeviceManager::getDriveName(const QString &drivePath)
{
    QList<QStorageInfo> volumeList= QStorageInfo::mountedVolumes();

    for(const auto& v: volumeList)
    {
        auto root = getRoot(v);

        if(v.isValid() && v.isReady() && isRemovable(root) && v.rootPath().compare(drivePath) == 0)
        {
            QString diskName = QObject::tr("USB Drive");
            if (v.name() != "")
            {
                diskName = v.displayName();
            }

            qDebug() << "Drive Name: " << diskName + " (" + root + ")";
            return diskName + " (" + root + ")";
        }
    }

    // if we end up here, no storage volume was found with the given name.
    // hence, we assume a custom path that the user set.
    m_custom_destination_path = drivePath;

    return m_custom_destination_path;
}

#if defined (Q_OS_MACOS) || defined (Q_OS_LINUX)
/**
 * @brief Determine if the given device is valid(removable and storage).
 * @param device(root.device())
 * @return true if the given drive is valid
 * @note ensure you update device list by calling getDeviceList before calling this method.
 */
bool DeviceManager::isValidDevice(const QString &device)
{
    if( device==m_custom_destination_path )     // the custom directory *must* be valid, because it's what the user mandates.
    {
        return true;
    }

    for (const auto& drive : _deviceName2Root)
    {
        auto device_str = drive.second->storageInfo.device();
        if (QString(device_str).section("/",QString(device_str).count("/")).compare(device) == 0)
            return true;
    }
    return false;
}
#endif

RetCode DeviceManager::formatDrive(QWidget* parentWidget, const QString &driveName, const QString &newLabel, const QString &passwd )
{
#ifdef Q_OS_LINUX
    Q_UNUSED( parentWidget )
#else
    Q_UNUSED( passwd )
#endif

    auto new_drive_label = newLabel;
    if (new_drive_label.isEmpty()) {
        new_drive_label = DEFAULT_DRIVE_LABEL;
    }

    auto ret = _deviceName2Root.find(driveName);
    if (ret == _deviceName2Root.end())
    {
        return DEVICE_NOT_FOUND;
    }

    QString deviceName = ret->second->name;

    if ( !isRemovable( deviceName ) ){
        return FAILURE;
    }


#ifndef Q_OS_LINUX
    QString cmd = getFormatCommand(deviceName, new_drive_label);

    if ( !cmd.isEmpty() ){

        pleaseWait = new PleaseWaitDialog();
        connect( pleaseWait, &QDialog::finished, pleaseWait, &QObject::deleteLater);
        pleaseWait->setParent( parentWidget );
        pleaseWait->setWindowFlags(Qt::Window | Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
        pleaseWait->setWindowModality(Qt::ApplicationModal);
        pleaseWait->setWindowTitle(tr("Formatting memory card"));

        connect( &formatProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){
            Q_UNUSED( exitStatus )
            if (exitCode==0 )
            {
                pleaseWait->setResultString( tr("The memory card has been formatted successfully.") );
            }
            else
            {
                pleaseWait->setResultString( tr("Formatting the memory card failed.") );
            }
        });

        if( isRemovable(deviceName) )
        {
            pleaseWait->setWaitMessage(tr("Please wait while the memory card is being formatted."));
            pleaseWait->show();
            formatProcess.start(cmd);
        }
        else
        {
            pleaseWait->setResultString(tr("The selected drive seems not to be a removable drive\nThis app will not format it for safety reasons."));
            pleaseWait->show();
        }

        setCurrentDrive("");
        return SUCCESS;
    }
#else

    int ret_code = -1;
    QString output = "";

    ejectDrive( driveName );

    std::tie(ret_code, output) = m_processExecutor.executeCommandWithSudo( QString("mkfs.vfat -n %1 -I %2").arg(new_drive_label).arg(deviceName), deviceName, passwd);

    qDebug() << "mkfs.vfat:\n" << output;
    if (ret_code == PASSWORD_INCORRECT) {
        return PASSWORD_INCORRECT;
    }

    if (ret_code == FAILURE || output.contains("mkfs.vfat:")) {
        m_processExecutor.executeCommand("sudo -k");
        return FAILURE;
    }

    remountDrive( deviceName );

    // sudo should forget password, otherwise the commands will work even with incorrect password within 15 minutes
    m_processExecutor.executeCommand("sudo -k");

    return SUCCESS;
#endif
    return FAILURE;
}



RetCode DeviceManager::ejectDrive(const QString &driveName)
{
    if( isWorkingOnCustomDirectory() ){ // we're working on a custom directory, not a drive that's ejectable.
        m_custom_destination_path = "";
        m_currentDrive = "";
        return SUCCESS;
    }

    auto ret = _deviceName2Root.find(driveName);
    if (ret == _deviceName2Root.end())
    {
        return DEVICE_NOT_FOUND;
    }

    QString diskName=ret->second->name;

#ifdef _WIN32
    auto driveLetter = diskName.at(0);
  
    if ( 0 != EjectDriveWin(driveLetter.unicode()) ){
        return FAILURE;
    }

#elif defined (Q_OS_MACOS)
    // we use the "unmount" command, since it does not ask for admin privileges.
    QString output = m_processExecutor.executeCommand("diskutil unmount " + diskName).second;               // execute the umount command synchronously.

    if ( !(output.contains("Volume", Qt::CaseInsensitive) && output.contains("unmounted", Qt::CaseInsensitive)) )       // The positive result of "unmount" looks like this: Volume NO NAME on disk2s1 unmounted
    {
        return FAILURE;
    }
#elif defined (Q_OS_LINUX)
    // TODO: udisksctl is only for preliminary use, need to replace it with dbus-send and gdbus in the future

    QString output = m_processExecutor.executeCommand("udisksctl unmount -b " + diskName).second;

    if (!output.startsWith("Unmounted ", Qt::CaseInsensitive))
        return FAILURE;

// This also turns off my card reader device, we don't want that.
//    output = m_processExecutor.executeCommand("udisksctl power-off -b " + diskName);
//    if (!output.isEmpty())
//        return FAILURE;

#endif

    m_currentDrive = QString();
    m_custom_destination_path = QString();
    return SUCCESS;
}


RetCode DeviceManager::remountDrive(const QString &driveName)
{

    QString diskName=driveName;

#ifdef _WIN32
    //@TODO Is there a way to remount an ejected drive under windows?
/*
    auto driveLetter = diskName.at(0);

    if ( 0 != EjectDriveWin(driveLetter.unicode()) ){
        return FAILURE;
    }

*/
#elif defined (Q_OS_MACOS)
    QString output = m_processExecutor.executeCommand("diskutil mount " + diskName).second;               // execute the mount command synchronously.

    if ( !(output.contains("Volume", Qt::CaseInsensitive) && output.contains("unmounted", Qt::CaseInsensitive)) )       // The positive result of "unmount" looks like this: Volume NO NAME on disk2s1 unmounted
    {
        return FAILURE;
    }
#elif defined (Q_OS_LINUX)
    // TODO: udisksctl is only for preliminary use, need to replace it with dbus-send and gdbus in the future

    QString output = m_processExecutor.executeCommand("udisksctl mount -b " + diskName).second;

    if (!output.startsWith("Mounted "))
        return FAILURE;

// This also turns off my card reader device, I don't want that.
//    output = m_processExecutor.executeCommand("udisksctl power-off -b " + diskName);
//    if (!output.isEmpty())
//        return FAILURE;

#endif

    m_currentDrive = QString();
    return SUCCESS;
}

void DeviceManager::setCurrentDrive(const QString &driveName)
{
    if( !driveName.isEmpty() && driveName==m_custom_destination_path ){
        // from now on, we will work on the custom folder that the user selected.
        m_currentDrive = driveName;
        qDebug() << "working on custom directory: " << m_custom_destination_path;
        return;
    }

    if (driveName.isEmpty())
    {
        m_currentDrive = "";
        m_custom_destination_path = "";
        return;
    }
    auto ret = _deviceName2Root.find(driveName);
    if(ret == _deviceName2Root.end())
    {
        qDebug() << "Device not found!";
        return;
    }
    m_currentDrive = ret->second->name;
}

QString DeviceManager::getFormatCommand(const QString &root, const QString &driveLabel)
{
#ifdef Q_OS_LINUX
    Q_UNUSED( root )
    Q_UNUSED(driveLabel)
#endif

    QString cmd;
    QString newRoot = "";
    QString systemRoot = "";
#if defined (Q_OS_WIN)
    newRoot = root.at(0);
    cmd = "CMD /C format " + newRoot + ": /FS:FAT32 /Q /X /Y /V:" + driveLabel;
#elif defined (Q_OS_MACOS)
    newRoot = root.left(root.length() - 2);

    cmd = QString("diskutil eraseDisk FAT32 %1 MBRFormat %2").arg(driveLabel).arg(newRoot);
#endif
    return cmd;
}


qint64 DeviceManager::getVolumeSize(const QString &driveName)
{
    auto ret = _deviceName2Root.find(driveName);
    if (ret == _deviceName2Root.end())
    {
        return 0;
    }
    quint64 b = ret->second->storageInfo.bytesTotal()-KEEP_FREE_FOR_DIAGNOSTICS_MODE;
    return b;
}

qint64 DeviceManager::getPlaylistSize(const QString playlistPath)
{
    qint64 dirSize = 0;
    int fileCount = 0;

    for(QDirIterator itDir(playlistPath, QDir::NoDotAndDotDot|QDir::Files|QDir::Hidden|QDir::System,QDirIterator::Subdirectories); itDir.hasNext(); )
    {
        itDir.next();
        dirSize += itDir.fileInfo().size();
        ++fileCount;
    }

    return dirSize;
}

qint64 DeviceManager::getAvailableSize(const QString &driveName)
{
    auto ret = _deviceName2Root.find(driveName);
    if (ret == _deviceName2Root.end())
    {
        return 0;
    }

    quint64 b = ret->second->storageInfo.bytesAvailable();
    return b;
}

QString DeviceManager::getVolumeFileSystem(const QString &driveName)
{
    auto ret = _deviceName2Root.find(driveName);
    if (ret == _deviceName2Root.end())
    {
        return "";
    }

    return ret->second->storageInfo.fileSystemType();
}

bool DeviceManager::hasFat32FileSystem(const QString &driveName)
{
    auto ret = _deviceName2Root.find(driveName);
    if (ret == _deviceName2Root.end())
    {
        return false;
    }

    auto fileSystem=ret->second->storageInfo.fileSystemType();
    if(fileSystem == "msdos" || fileSystem == "FAT32" || fileSystem == "vfat")
    {
        return true;
    }
    return false;
}

bool DeviceManager::isWriteProtected(const QString &driveName)
{
    auto ret = _deviceName2Root.find(driveName);
    if (ret == _deviceName2Root.end())
    {
        return true;    // the device was not found. Let's not allow writing to it.
    }
    return ret->second->storageInfo.isReadOnly();
}

bool DeviceManager::isWorkingOnCustomDirectory() const {
    return !m_custom_destination_path.isEmpty();
}

void DeviceManager::addCustomPath( const QString &customPath )
{
    m_custom_destination_path = customPath;
}


