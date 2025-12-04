#include "convertchaptertask.h"
#include "processexecutor.h"
#include <unordered_set>

/*
ConvertChapterTask::ConvertChapterTask(QString command, QString file, double adjustByDb, QString start, QString end, QString metadata, QString out, int id) :
    command(command),
    arguments({
        "-i",
        file,
        "-ss",
        start,
        "-to",
        end, // index = 5
        "-acodec",
        "pcm_s16le",
        "-ar",
        "32k",
        "-ac",
        "1",
        "-metadata",
        metadata, // index = 13
        "-y",
        "-hide_banner"
    }),
    file(file),
    out(out),
    id(id)
{
    if( qAbs(adjustByDb)>0.1 && qAbs(adjustByDb)<20.0 )
    {
        arguments.append("-af");
        arguments.append(QString("volume=%1dB").arg( adjustByDb, 0, 'f', 1 ) );
    }
    arguments.append(out);
}
*/

ConvertChapterTask::ConvertChapterTask(QString command, QString file, QStringList chapters, double adjustByDb, int maxMetaLength, QString tmpPath, QString destination) :
    m_command(command),
    m_file(file),
    m_chapters(chapters),
    m_adjustByDb(adjustByDb),
    m_maxMetaLength(maxMetaLength),
    m_destination(destination),
    m_tmpPath(tmpPath)
{
}

void ConvertChapterTask::stop()
{
    m_stopped = true;
    for(int i = 0; i < m_runnedProcesses.size(); i++)
    {
        auto & p = *m_runnedProcesses[i];
        if(!(p.state() == QProcess::NotRunning))
        {
            p.kill();
            emit chapterFinished(-1, "killed", "", i);
        }
    }
    for(int i = m_runnedProcesses.size(); i < m_chapters.count(); i++)
    {
        emit chapterFinished(-1, "aborted", "", i);
    }
}

void ConvertChapterTask::spawnNewProcesses()
{
    if(m_stopped)
        return;
    m_activeProcesses = 0;
    for(int i = 0; i < m_runnedProcesses.size(); i++)
    {
        auto & p = *m_runnedProcesses[i];
        if(!(p.state() == QProcess::NotRunning))
        {
            m_activeProcesses++;
        } else
        {
            if(m_finishedProcesses.find(i) == m_finishedProcesses.end())
            {
                emit chapterFinished(p.exitCode(), p.readAllStandardOutput(), m_outPaths[i], i);
                m_finishedProcesses.insert(i);
            }
        }
    }

    while(m_taskId < m_chapters.count() && m_activeProcesses < m_maxProcessCount)
    {
        m_activeProcesses++;
        QString chapter = m_chapters[m_taskId++];
        auto start = chapter.section("<!@#^&>", 0, 0);
        auto end = chapter.section("<!@#^&>", 1, 1);
        auto metadata = chapter.section("<!@#^&>", 2);
        metadata.resize(m_maxMetaLength, ' ');
        metadata = QString("title=%1").arg(metadata.trimmed());
        QStringList args = {
            "-i",
            m_file,
            "-ss",
            start,
            "-to",
            end, // index = 5
            "-acodec",
            "pcm_s16le",
            "-ar",
            "32k",
            "-ac",
            "1",
            "-metadata",
            metadata, // index = 13
            "-y",
            "-hide_banner"
        };
        if( qAbs(m_adjustByDb)>0.1 && qAbs(m_adjustByDb)<20.0 )
        {
            args.append("-af");
            args.append(QString("volume=%1dB").arg( m_adjustByDb, 0, 'f', 1 ) );
        }
        auto output_path = m_tmpPath + QDateTime::currentDateTime().toString("yyyyMMddHHmmss") + QString("-%1").arg(m_taskId-1) + m_destination;
        m_outPaths.push_back(output_path);
        args.append(output_path);
        //runnedProcesses.push_back(pe.runProcess(m_command,args));

        m_runnedProcesses.push_back(std::make_unique<QProcess>());
        m_runnedProcesses.back()->setProcessChannelMode(QProcess::MergedChannels);
        m_runnedProcesses.back()->start(m_command,args);
        m_runnedProcesses.back()->waitForStarted();

        QObject::connect(m_runnedProcesses.back().get(),&QProcess::stateChanged,
                         this,&ConvertChapterTask::spawnNewProcesses);
    }
}

void ConvertChapterTask::start()
{
    m_stopped = false;
    spawnNewProcesses();
}
