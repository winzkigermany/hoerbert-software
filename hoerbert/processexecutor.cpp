#include "processexecutor.h"
#include "pleasewaitdialog.h"

ProcessExecutor::ProcessExecutor(QObject *parent) : QObject(parent)
{

}

std::unique_ptr<QProcess> ProcessExecutor::runProcess(const QString &cmdString, const QStringList& arguments)
{
    qDebug() << QString( "executeCommand: %1 %2").arg(cmdString).arg( arguments.join(" ") );
    std::unique_ptr<QProcess> p = std::make_unique<QProcess>(this);
    p->setProcessChannelMode(QProcess::MergedChannels);
    if( arguments.count()>0 )
    {
        p->start(cmdString, arguments);
    }
    else
    {
        p->start(cmdString);
    }
    return p;
}


std::pair<int, QString> ProcessExecutor::executeCommand(const QString &cmdString, const QStringList& arguments, const QString &workingDirectory)
{
    qDebug() << QString( "executeCommand: %1 %2").arg(cmdString).arg( arguments.join(" ") );

    QFutureWatcher< std::pair<int, QString> > watcher;
    QFuture< std::pair<int, QString>> future;
    watcher.setFuture(future);

    future = QtConcurrent::run([cmdString, arguments, workingDirectory]() {
        // Code in this block will run in another thread
        QProcess p;
        p.setProcessChannelMode(QProcess::MergedChannels);
        if( !workingDirectory.isEmpty() )
        {
            p.setWorkingDirectory( workingDirectory );
        }
        if( arguments.count()>0 )
        {
            p.start(cmdString, arguments);
        }
        else
        {
            p.start(cmdString);
        }
        p.waitForFinished(-1);
        int exitCode = p.exitCode();
        QString standardOut = p.readAllStandardOutput();
        return std::pair<int, QString>(exitCode, standardOut);
    });

    QEventLoop loop;
    connect( &watcher, &QFutureWatcher<QString>::finished, this, [&loop]{   // QFutureWatcher remembers the ::finished signal until we connect to it. THAT saves us from loosing it before connecting to it.
        loop.quit();
    });
    loop.exec();

    std::pair<int, QString> result = future.result();

    return result;     // this is blocking while waiting for result. That's why we need the QEventLoop above.
}



std::pair<int, QString> ProcessExecutor::executeCommandWithSudo( const QString &newCmd, const QString &devicePath, const QString &passwd, QWidget* parentWidget)
{
    int ret_code = -1;
    QString cmd = newCmd;
    if (!cmd.startsWith("sudo -S"))
    {
        cmd = "sudo -S " + cmd;
    }

    qDebug() << QString( "executeCommandWithSudo: %1").arg(cmd);
    PleaseWaitDialog* pleaseWait;
    QProcess process;

    QFutureWatcher< std::pair<int, QString> > watcher;
    QFuture< std::pair<int, QString>> future;
    watcher.setFuture(future);

    pleaseWait = new PleaseWaitDialog();
    connect( pleaseWait, &QDialog::finished, pleaseWait, &QObject::deleteLater);
    pleaseWait->setParent( parentWidget );
    pleaseWait->setWindowFlags(Qt::Window | Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    pleaseWait->setWindowTitle(tr("Formatting memory card..."));
    pleaseWait->setWindowModality(Qt::ApplicationModal);
    pleaseWait->show();


    future = QtConcurrent::run([this, &process, pleaseWait, cmd, devicePath, passwd]() -> std::pair<int, QString> {
        // Code in this block will run in another thread
        process.setProcessChannelMode(QProcess::MergedChannels);

        connect( this, &ProcessExecutor::kill, &process, &QProcess::kill);

        connect( &process, &QProcess::readyReadStandardOutput, [&process, pleaseWait, passwd] () {
            QString output = process.readAllStandardOutput();
            qDebug() << output;

            if (output.contains("Sorry, try again")) {      //@TODO This is debatable... does it work with other system languages at all?
                pleaseWait->setResultString( tr("Password is incorrect. Please try again.") );
            }

            if( output.contains("[sudo]") ){        // This is the prompt: [sudo] password for xyz
                // send the password to the process
                process.write(QString("%1\n").arg(passwd).toUtf8().constData());
            }
        });

        process.start(cmd);
        process.waitForFinished(-1);
        int exitCode = process.exitCode();

        if( exitCode==0 )
        {
            pleaseWait->setResultString( tr("The memory card has been formatted successfully"));
        }
        else
        {
            pleaseWait->setResultString( tr("Formatting the memory card failed.") );
        }

        QString standardOut = process.readAll();
        return std::pair<int, QString>(exitCode, standardOut);
    });

    QEventLoop loop;
    connect( &watcher, &QFutureWatcher<std::pair<int, QString>>::finished, this, [&loop]{   // QFutureWatcher remembers the ::finished signal until we connect to it. THAT saves us from loosing it before connecting to it.
        loop.quit();
    });
    loop.exec();

    std::pair<int, QString> result = future.result();

    if (result.first)
    {
        if (ret_code != PASSWORD_INCORRECT)
            ret_code = SUCCESS;

    }
    else
    {
        if (ret_code != PASSWORD_INCORRECT)
            ret_code = FAILURE;
    }

    return std::pair<int, QString>(ret_code, result.second);
}


