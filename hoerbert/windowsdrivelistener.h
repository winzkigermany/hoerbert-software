#ifndef WINDOWSDRIVELISTENER_H
#define WINDOWSDRIVELISTENER_H

#include <QObject>
#include <QStorageInfo>
#include <QList>
#include <QTimer>

class WindowsDriveListener : public QObject
{
    Q_OBJECT
public:
    explicit WindowsDriveListener(QObject *parent = nullptr);

signals:
    /**
     * @brief drivesHaveChanged signals a drive list change
     */
    void drivesHaveChanged();

private:
    QList<QStorageInfo> m_availableDrives;
    QTimer* m_timer;
    static QMutex m_checkingMutex;

private slots:
    /**
     * @brief update the list of drives
     */
    void update();
};

#endif // WINDOWSDRIVELISTENER_H
