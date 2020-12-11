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
#include <QFuture>
#include <QtConcurrent>

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
    m_currentDriveName = "";
    // Get the list of available drives right from the start.
    getDeviceList();
}

void DeviceManager::refresh( const QString &driveName )
{
    if( isWorkingOnCustomDirectory() ){
        QStorageInfo qsi;
        if( driveName!=NULL )
        {
            qsi.setPath( driveName );
        }
        else
        {
            qsi.setPath( m_currentDriveName );
        }
        qsi.refresh();

        return;
    }

    std::map<QString, VolumeInfo_ptr>::const_iterator ret;
    if( driveName != NULL ){
        ret = _deviceName2Root.find(driveName);
    } else {
        ret = _deviceName2Root.find(m_currentDriveName);
    }
    ret->second->storageInfo.refresh();
}

bool DeviceManager::isRemovable(const QString &volumeRoot)
{
#ifdef _WIN32
    auto root = reinterpret_cast<LPCWSTR>(volumeRoot.utf16());
    UINT type = GetDriveType(root);
    return (type == DRIVE_REMOVABLE);
#elif defined (Q_OS_MACOS)
    QString cmd = "diskutil info " + volumeRoot;
    QString output = executeCommand(cmd);

    return output.contains(QRegExp("Removable Media:.*Removable", Qt::CaseInsensitive));
#elif defined (Q_OS_LINUX)
    QString cmd = "udevadm info --query=property --export --name=" + volumeRoot;
    QString output = executeCommand(cmd);

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
    QString output = executeCommand(cmd);

    return output.contains(QRegExp("Ejectable", Qt::CaseInsensitive));
#elif defined (Q_OS_LINUX)
    QString cmd = "udevadm info --query=property --export --name=" + volumeRoot;
    QString output = executeCommand(cmd);

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

    QString path = QString("");
    if (m_currentDriveName.isEmpty())
        return path;

    QString theDriveName = m_currentDriveName;
    if( driveName!=nullptr ){
        theDriveName = driveName;
    }
    auto drive = _deviceName2Root.find(theDriveName);

    path = drive->second->storageInfo.rootPath();
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
#ifndef Q_OS_LINUX
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

        setCurrentDrive(QString());
        return SUCCESS;
    }
#else

    int ret_code = -1;
    QString output = "";

    ejectDrive( driveName );

    std::tie(ret_code, output) = executeCommandWithSudo( &formatProcess, QString("mkfs.vfat -n %1 -I %2").arg(new_drive_label).arg(deviceName), deviceName, passwd, true);

    qDebug() << "mkfs.vfat:\n" << output;
    if (ret_code == PASSWORD_INCORRECT) {
        return PASSWORD_INCORRECT;
    }

    if (ret_code == FAILURE || output.contains("mkfs.vfat:")) {
        executeCommand("sudo -k");
        return FAILURE;
    }
/*
    QString user_name = qgetenv("USER");
    QDir dir(QString("/media/%1").arg(user_name));
    if (dir.exists())
    {
        ret_code = -1;
        output = "";

        std::tie(ret_code, output) = executeCommandWithSudo(QString("mkdir \"/media/%1/%2\"").arg(user_name).arg(new_drive_label), deviceName, passwd);

        qDebug() << "mkdir:\n" << output;
        if (ret_code == PASSWORD_INCORRECT) {
            return PASSWORD_INCORRECT;
        }

        if (!output.isEmpty())
        {
            qDebug() << "Failed creating mount point";
        }

        ret_code = -1;
        output = "";

        std::tie(ret_code, output) = executeCommandWithSudo(QString("sudo -S mount %1 \"/media/%2/%3\" -o uid=%2 -o gid=%2").arg(root).arg(user_name).arg(new_drive_label), deviceName, passwd);

        qDebug() << "mount:\n" << output;

        if (ret_code == PASSWORD_INCORRECT) {
            return PASSWORD_INCORRECT;
        }

        if (output.contains("mount:"))
        {
            qDebug() << "Failed mounting formatted drive";
        }
    }
*/
    // sudo should forget password, otherwise the commands will work even with incorrect password within 15 minutes
    executeCommand("sudo -k");

    return SUCCESS;
#endif
    return FAILURE;
}


std::pair<int, QString> DeviceManager::executeCommandWithSudo( QProcess* theProcess, const QString &newCmd, const QString &devicePath, const QString &passwd, bool showPleaseWaitDialog, QWidget* parentWidget)
{
    int ret_code = -1;
    QString cmd = newCmd;
    if (!cmd.startsWith("sudo -S"))
    {
        cmd = "sudo -S " + cmd;
    }



    qDebug() << cmd;

    if( showPleaseWaitDialog ){
        pleaseWait = new PleaseWaitDialog();
        connect( pleaseWait, &QDialog::finished, pleaseWait, &QObject::deleteLater);
        pleaseWait->setParent( parentWidget );
        pleaseWait->setWindowFlags(Qt::Window | Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
        pleaseWait->setWindowTitle(tr("Formatting memory card..."));
        pleaseWait->setWindowModality(Qt::ApplicationModal);
        pleaseWait->show();
    }

    theProcess->setProcessChannelMode(QProcess::MergedChannels);

    if( showPleaseWaitDialog ){
        connect( theProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){
            Q_UNUSED( exitStatus )

            if( exitCode==0 )
            {
                pleaseWait->setResultString( tr("The memory card has been formatted successfully"));
            }
            else
            {
                pleaseWait->setResultString( tr("Formatting the memory card failed.") );
            }

            remountDrive( devicePath );
        });
    }


    connect( theProcess, &QProcess::readyReadStandardOutput, [=] () {
        QString output = theProcess->readAllStandardOutput();
        if (output.contains("Sorry, try again")) {      //@TODO This is debatable... does it work with other system languages at all?
            if( showPleaseWaitDialog ){
                pleaseWait->setResultString( tr("Password is incorrect. Please try again.") );
            }
        }

        if( output.contains("[sudo]") ){        // This is the prompt: [sudo] password for xyz
            // send the password to the process
            theProcess->write(QString("%1\n").arg(passwd).toUtf8().constData());
        }
        qDebug() << output;
    });


    bool returnValue = false;
    QEventLoop loop;
    connect(theProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [&returnValue, &loop](int result){
        returnValue = (result==0);
        loop.quit();
    });
    theProcess->start(cmd);
    if (!theProcess->waitForStarted())
    {
        qDebug() << "Process failed to start!";
        ret_code = FAILURE;
        theProcess->disconnect();
        return std::pair<int, QString>(ret_code, theProcess->readAll());
    }

    if( !showPleaseWaitDialog ){
        theProcess->write(QString("%1\n").arg(passwd).toUtf8().constData());
    }
    loop.exec();
    theProcess->disconnect();

    if (returnValue)
    {
        if (ret_code != PASSWORD_INCORRECT)
            ret_code = SUCCESS;

        return std::pair<int, QString>(ret_code, theProcess->readAll());
    }
    else
    {
        if (ret_code != PASSWORD_INCORRECT)
            ret_code = FAILURE;
        return std::pair<int, QString>(ret_code, theProcess->readAll());
    }

    return std::pair<int, QString>(SUCCESS, "");    // in case we're only starting formatting, there's nothing better we can return (if the process START was successful.)
}

RetCode DeviceManager::ejectDrive(const QString &driveName)
{
    if( isWorkingOnCustomDirectory() ){ // we're working on a custom directory, not a drive that's ejectable.
        m_custom_destination_path = QString();
        m_currentDrive = QString();
        m_currentDriveName = QString();
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
    QString output = executeCommand("diskutil unmount " + diskName);               // execute the umount command synchronously.

    if ( !(output.contains("Volume", Qt::CaseInsensitive) && output.contains("unmounted", Qt::CaseInsensitive)) )       // The positive result of "unmount" looks like this: Volume NO NAME on disk2s1 unmounted
    {
        return FAILURE;
    }
#elif defined (Q_OS_LINUX)
    // TODO: udisksctl is only for preliminary use, need to replace it with dbus-send and gdbus in the future

    QString output = executeCommand("udisksctl unmount -b " + diskName);

    if (!output.startsWith("Unmounted ", Qt::CaseInsensitive))
        return FAILURE;

// This also turns off my card reader device, we don't want that.
//    output = executeCommand("udisksctl power-off -b " + diskName);
//    if (!output.isEmpty())
//        return FAILURE;

#endif

    m_currentDrive = QString();
    m_currentDriveName = QString();
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
    QString output = executeCommand("diskutil mount " + diskName);               // execute the mount command synchronously.

    if ( !(output.contains("Volume", Qt::CaseInsensitive) && output.contains("unmounted", Qt::CaseInsensitive)) )       // The positive result of "unmount" looks like this: Volume NO NAME on disk2s1 unmounted
    {
        return FAILURE;
    }
#elif defined (Q_OS_LINUX)
    // TODO: udisksctl is only for preliminary use, need to replace it with dbus-send and gdbus in the future

    QString output = executeCommand("udisksctl mount -b " + diskName);

    if (!output.startsWith("Mounted "))
        return FAILURE;

// This also turns off my card reader device, I don't want that.
//    output = executeCommand("udisksctl power-off -b " + diskName);
//    if (!output.isEmpty())
//        return FAILURE;

#endif

    m_currentDrive = QString();
    m_currentDriveName = QString();
    return SUCCESS;
}

void DeviceManager::setCurrentDrive(const QString &driveName)
{
    if( !driveName.isEmpty() && driveName==m_custom_destination_path ){
        // from now on, we will work on the custom folder that the user selected.
        m_currentDrive = driveName;
        m_currentDriveName = driveName;
        qDebug() << "working on custom directory: " << m_custom_destination_path;
        return;
    }

    if (driveName.isEmpty())
    {
        m_currentDrive = driveName;
        return;
    }
    auto ret = _deviceName2Root.find(driveName);
    if(ret == _deviceName2Root.end())
    {
        qDebug() << "Device not found!";
        return;
    }
    m_currentDrive = ret->second->name;
    m_currentDriveName = driveName;

    qDebug() << m_currentDrive << m_currentDriveName;
}

QString DeviceManager::selectedDrive() const
{
    return m_currentDrive;
}

QString DeviceManager::selectedDriveName() const
{
    return m_currentDriveName;
}

QString DeviceManager::getFormatCommand(const QString &root, const QString &driveLabel)
{
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

QString DeviceManager::executeCommand(const QString &cmdString)
{
    qDebug() << QString( "executing %1").arg(cmdString);

    QFuture<QString> future = QtConcurrent::run([cmdString]() {
        // Code in this block will run in another thread
        QProcess p;
        p.start(cmdString);
        p.waitForFinished(-1);
        QString standardOut = p.readAllStandardOutput();
        return standardOut;
    });

    QString result = future.result();

    return result;
}

qint64 DeviceManager::getVolumeSize(const QString &driveName)
{
    auto ret = _deviceName2Root.find(driveName);
    if (ret == _deviceName2Root.end())
    {
        return 0;
    }
    return ret->second->storageInfo.bytesTotal()-KEEP_FREE_FOR_DIAGNOSTICS_MODE;
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
    return ret->second->storageInfo.bytesAvailable();
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


