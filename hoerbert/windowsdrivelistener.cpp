#include "windowsdrivelistener.h"

QMutex WindowsDriveListener::m_checkingMutex;

WindowsDriveListener::WindowsDriveListener(QObject *parent) : QObject(parent)
{    
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &WindowsDriveListener::update);
    m_timer->start(800);    // msec
}


void WindowsDriveListener::update()
{

    if( !WindowsDriveListener::m_checkingMutex.tryLock() ){
        return;
    }

    QList<QStorageInfo> newDrives = QStorageInfo::mountedVolumes();
    QList<QStorageInfo> newDrivesCopy = newDrives;

    if( m_availableDrives==newDrives ){
        WindowsDriveListener::m_checkingMutex.unlock();
        return;
    }

    if( (m_availableDrives.count()==0 && newDrives.count()>0) || (m_availableDrives.count()>0 && newDrives.count()==0) )
    {
        m_availableDrives = newDrives;
        emit drivesHaveChanged();
        WindowsDriveListener::m_checkingMutex.unlock();
        return;
    }

    QList<QStorageInfo> currentDrives = m_availableDrives;

    foreach (const QStorageInfo &currentStorage, currentDrives)
    {
        if (currentStorage.isValid() && currentStorage.isReady())
        {

            foreach (const QStorageInfo &newStorage, newDrives)
            {
                if (newStorage.isValid() && newStorage.isReady())
                {
                    if( newStorage==currentStorage )
                    {
                        currentDrives.removeOne( currentStorage );
                        newDrives.removeOne( newStorage );
                        break;
                    }

                }
            }

        }
    }


    if( currentDrives.count()==0 && newDrives.count()==0 )     // all existing drives are still there and no new drive has appeared
    {
        WindowsDriveListener::m_checkingMutex.unlock();
        return;
    }

    if( currentDrives.count()==0 && newDrives.count()>0 )       // all existing drives are still here, but a 1..n drives appeared
    {
        m_availableDrives = newDrivesCopy;
        emit drivesHaveChanged();
        WindowsDriveListener::m_checkingMutex.unlock();
        return;
    }

    if( currentDrives.count()>0 && newDrives.count()==0 )       // old drives have disappeared
    {
        m_availableDrives = newDrivesCopy;
        emit drivesHaveChanged();
        WindowsDriveListener::m_checkingMutex.unlock();
        return;
    }

    if( currentDrives.count()>0 && newDrives.count()>0 )       // old drives have disappeared and new drives have appeared
    {
        m_availableDrives = newDrivesCopy;
        emit drivesHaveChanged();
        WindowsDriveListener::m_checkingMutex.unlock();
        return;
    }


}


