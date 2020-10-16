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

#include "functions.h"

#include <QDir>
#include <QDebug>

int sortByNumber(const QFileInfo &fileName1, const QFileInfo &fileName2)
{
    QString name1 = fileName1.fileName().section(".", 0, 0).section("-", 0, 0);
    QString name2 = fileName2.fileName().section(".", 0, 0).section("-", 0, 0);
    if (name1.compare(name2) == 0)
        return fileName1.fileName().section(".", 0, 0).section("-", 1).toInt() < fileName2.fileName().section(".", 0, 0).section("-", 1).toInt();
    else
        return name1.toInt() < name2.toInt();
}


int getFirstNumberInDirectory(const QString &dirPath)
{
    QDir dir(dirPath);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setNameFilters(QStringList() << DEFAULT_FORMAT);
    dir.setSorting(QDir::Name);

    QFileInfoList list = dir.entryInfoList();
    int first_number = 0;

    if (list.count() > 0) {
        std::sort(list.begin(), list.end(), sortByNumber);
        first_number = list.at(0).fileName().section(".", 0, 0).toInt();
    } else {
        first_number = -1;
    }
    return first_number;
}

int getLastNumberInDirectory(const QString &dirPath)
{
    QDir dir(dirPath);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setNameFilters(QStringList() << DEFAULT_FORMAT);
    dir.setSorting(QDir::Name);

    QFileInfoList list = dir.entryInfoList();
    int max_number = 0;

    if (list.count() > 0) {
        std::sort(list.begin(), list.end(), sortByNumber);
        max_number = list.at(list.size() - 1).fileName().section(".", 0, 0).toInt();
    } else {
        max_number = -1;
    }
    return max_number;
}

int getFileNameWithoutExtension(const QString &absoluteFilePath)
{
    QFileInfo fi(absoluteFilePath);
    if (!fi.exists())
    {
        qDebug() << "(GetFileNameWithoutExtension)File does not exist:" << absoluteFilePath;
        return -1;
    }
    auto file_name = fi.fileName().section(".", 0, 0);

    if (file_name.isEmpty() || (file_name.compare("0") != 0 && file_name.toInt() == 0))
        return -1;
    else
        return file_name.toInt();
}

QString increaseFileName(const QString &absoluteFilePath, int offset)
{
    QFileInfo fi(absoluteFilePath);
    auto file_number = getFileNameWithoutExtension(fi.absoluteFilePath());
    if (file_number == -1)
        return QString();

    QString increased_file_name = fi.absolutePath() + "/" +  QString::number(file_number + offset) + ".wav";
    return increased_file_name;
}

int getFileCount(const QString &absoluteDirPath)
{
    QDir dir(absoluteDirPath);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setNameFilters(QStringList() << DEFAULT_FORMAT);
    dir.setSorting(QDir::Name);

    return dir.entryInfoList().count();
}

bool moveFile(const QString &sourcePath, const QString &destPath)
{
    auto result = rename(sourcePath.toLocal8Bit().data(), destPath.toLocal8Bit().data());
    if (result == 0) {
        qDebug() << "Moving" << sourcePath << "to" << destPath << "succeeded!";
        return true;
    }
    else {
        perror("File rename error:");
        qDebug() << "Moving" << sourcePath << "to" << destPath << "failed!";
        return false;
    }
}

int batchRenameByIndex(const QString &dirPath, int from, int offset, int to)
{
    QDir dir(dirPath);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setNameFilters(QStringList() << DEFAULT_FORMAT);
    dir.setSorting(QDir::Name);

    QFileInfoList list = dir.entryInfoList();
    int max_number = 0;

    if (list.count() > 0) {
        std::sort(list.begin(), list.end(), sortByNumber);
        max_number = list.at(list.size() - 1).fileName().section(".", 0, 0).toInt();
    }
    else
        return max_number;

    int renamed_file_count = 0;
    if ((from > list.count() - 1))
        return renamed_file_count;

    if (to <= 0)
        to = list.count() - 1;

    if (to > list.count() - 1)
        to = list.count() - 1;

    assert(to >= from);

    for (int i = to; i >= from; i--)
    {
        auto file_name = list.at(i).absoluteFilePath();
        auto new_name = increaseFileName(file_name, offset);
        if (moveFile(file_name, new_name))
        {
            renamed_file_count++;
        }
    }
    return renamed_file_count;
}

bool batchRenameByName(const QString &dirPath, int from, int offset, int to)
{
    QDir dir(dirPath);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setNameFilters(QStringList() << DEFAULT_FORMAT);
    dir.setSorting(QDir::Name);

    QFileInfoList list = dir.entryInfoList();
    int max_number = 0;

    if (list.count() > 0) {
        std::sort(list.begin(), list.end(), sortByNumber);
        max_number = list.at(list.size() - 1).fileName().section(".", 0, 0).toInt();
    }
    else
        return 0;

    if ((from > getLastNumberInDirectory(dirPath))) {
        return true;
    }

    if (max_number < to || to == 0)
        to = max_number;

    //assert(!(to < from));
    qDebug() << "Batch renaming files from" << from << "to" << to;

    for (int i = list.count() - 1; i >= 0; i--)
    {
        auto source_file = list.at(i).absoluteFilePath();
        QString file_name = list.at(i).fileName().section(".", 0, 0);
        if (file_name.contains("-")) {
            file_name = file_name.section("-", 0, 0);
            if ((file_name.compare("0") != 0) && (file_name.toInt() == 0)) {
                qDebug() << "Invalid file" << list.at(i).fileName();
                return false;
            }
        }

        if (file_name.toInt() > to)
        {
            qDebug() << "Out of considered range(end)" << source_file << ">" << to;
            continue;
        }

        if (file_name.toInt() < from)
        {
            qDebug() << "Out of considered range(begin)" << source_file << "<" << from;
            break;
        }

        auto new_file = increaseFileName(source_file, offset);
        if (!moveFile(source_file, new_file))
        {
            qDebug() << "Failed to rename file" << source_file;
            return false;
        }
    }
    return true;
}

int countSubfiles(const QString &dirPath, int prefixNumber, const QString &extension)
{
    int count = 0;

    QDir dir(dirPath);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setNameFilters(QStringList() << extension);
    dir.setSorting(QDir::Name);

    QFileInfoList list = dir.entryInfoList();

    if (list.count() > 0)
        std::sort(list.begin(), list.end(), sortByNumber);
    else
        return count;

    QString prefix = QString("%1-").arg(prefixNumber);
    QString tmp_string;

    for (const auto& info : list)
    {
        tmp_string = info.fileName();
        if (tmp_string.startsWith(prefix))
        {
            count++;
            continue;
        }

        if (!tmp_string.contains("-") && tmp_string.toLower().section(extension.toLower(), 0, 0).toInt() > prefixNumber)
            break;
    }

    return count;
}


QString attachSuffixToFileName(const QString &absoluteFilePath, const QString &suffix)
{
    QFileInfo fi(absoluteFilePath);
    auto file_number = getFileNameWithoutExtension(fi.absoluteFilePath());
    if (file_number == -1)
        return QString();

    QString new_file_name = fi.absolutePath() + "/" +  QString::number(file_number) + suffix + QString(DEFAULT_FORMAT).replace("*.", ".");

    bool moved = moveFile(absoluteFilePath, new_file_name);
    if (moved)
        return new_file_name;
    else
        return QString();
}

bool deleteAllFilesInDirecotry(const QString &absolutePath)
{
    QDir dir(absolutePath);
    if (dir.exists())
    {
        dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);

        QFileInfoList info_list = dir.entryInfoList();
        for (const auto& info : info_list)
            if (!QFile::remove(info.absoluteFilePath()))
                return false;
    }

    return true;
}

QString tailPath(const QString &path)
{
    QString separator = "/";
    if (path.endsWith(separator))
        return path;
    else
        return path + separator;
}

void recursivelyGetDirectoryContent(QStringList *list, const QString &destDir, int depth)
{
    QDir dir(destDir);
    if (dir.exists()) {
        dir.setFilter(QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        dir.setSorting(QDir::Name);

        QFileInfoList infoList = dir.entryInfoList();
        std::sort(infoList.begin(), infoList.end(), sortByNumber);
        for (auto fileInfo : infoList) {
            if (fileInfo.isDir()) {
                list->append(QString("%1+%2").arg(QString("|").repeated(depth + 1)).arg(fileInfo.fileName()));
                recursivelyGetDirectoryContent(list, fileInfo.absoluteFilePath(), depth + 1);
            } else {
                list->append(QString("%1%2 [%3]").arg(QString("|").repeated(depth + 1)).arg(fileInfo.fileName()).arg(fileInfo.size()));
            }
        }
    }
}

quint64 bytesToSeconds(quint64 byteSize)
{
    return quint64(double(byteSize) / 32000 / 2 / 1.05);
}

quint64 secondsToBytes(int seconds)
{
    return quint64(double(seconds) * 32000 * 2 * 1.05);
}
