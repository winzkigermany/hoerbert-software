#ifndef PROCESSEXECUTOR_H
#define PROCESSEXECUTOR_H

#include "define.h"

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

class ProcessExecutor : public QObject
{
    Q_OBJECT
public:
    explicit ProcessExecutor(QObject *parent = nullptr);

    std::unique_ptr<QProcess> runProcess(const QString &cmdString, const QStringList& arguments);

     /**
     * @brief execute a command and return the output
     * @param cmdString
     * @return the output of the command
     */
    std::pair<int, QString> executeCommand(const QString &cmdString, const QStringList& arguments=QStringList(), const QString& workingDirectory="");

    /**
     * @brief executeCommandWithSudo execute command with sudo and automatically interact with password input
     * @param cmd command to be executed
     * @param passwd user's password
     * @return result code and output of command execution
     */
    std::pair<int, QString> executeCommandWithSudo( const QString &cmd, const QString &drivePath, const QString &passwd = QString(), QWidget* parentWidget=nullptr );

signals:
    /**
     * @brief kill We may try, but we may not succeed.
     */
    void kill();
};

#endif // PROCESSEXECUTOR_H
