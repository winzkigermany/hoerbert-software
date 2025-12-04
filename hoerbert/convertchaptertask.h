#ifndef CONVERTCHAPTERTASK_H
#define CONVERTCHAPTERTASK_H

#include <memory>
#include <QObject>
#include <QProcess>
#include <QThread>
#include <unordered_set>
class ConvertChapterTask : public QObject
{
    Q_OBJECT
public:
    //ConvertChapterTask(QString command, QString file, double adjustByDb, QString start, QString end, QString metadata, QString out, int id);
    ConvertChapterTask(QString command, QString file, QStringList chapters, double adjustByDb, int maxMetaLength, QString tmpPath, QString destination);

    void start();
    void stop();
private slots:
    void spawnNewProcesses();
private:
    bool m_stopped;

    QString m_command;
    QString m_file;
    QStringList m_chapters;
    double m_adjustByDb;
    int m_maxMetaLength;
    QString m_destination;
    QString m_tmpPath;

    const int m_maxProcessCount = QThread::idealThreadCount();
    std::vector<std::unique_ptr<QProcess>> m_runnedProcesses;
    std::vector<QString> m_outPaths;
    std::unordered_set<int> m_finishedProcesses;
    int m_activeProcesses = 0;
    int m_taskId = 0;
signals:
    void chapterFinished(int, QString, QString, int);
};
#endif // CONVERTCHAPTERTASK_H
