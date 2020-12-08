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

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QString>

class QFileInfo;

const QString DEFAULT_FORMAT    = "*.WAV";

// directory and file operations

/**
 * @brief sortByNumber
 * @param fileName1
 * @param fileName2
 * @return
 */
int sortByNumber(const QFileInfo &fileName1, const QFileInfo &fileName2);

/**
 * @brief Get first(minimum) number among filenames without extension in given directory.
 * @param dirPath absolute directory path
 * @return first file's number
 */
int getFirstNumberInDirectory(const QString &dirPath);

/**
 * @brief Get last(maximum) number among filenames without extension in given directory.
 * @param dirPath absolute directory path
 * @return last file's number
 */
int getLastNumberInDirectory(const QString &dirPath);

/**
 * @brief Get filename as number by removing extension
 * @param absoluteFilePath absolute file path
 * @return file number
 */
int getFileNameWithoutExtension(const QString &absoluteFilePath);

/**
 * @brief increase a file number by an offset
 * @param absoluteFilePath the current file name
 * @param offset
 * @return the new file name with the offset added
 */
QString increaseFileName(const QString &absoluteFilePath, int offset);

/**
 * @brief getFileCount
 * @param absoluteDirPath
 * @return number of files in the given directory
 */
int getFileCount(const QString &absoluteDirPath);

/**
 * @brief moveFile
 * @param sourcePath
 * @param destPath
 * @return
 */
bool moveFile(const QString &sourcePath, const QString &destPath);

/**
 * @brief moveDirectory
 * @param sourcePath
 * @param destPath
 * @return
 */
bool moveDirectory(const QString &sourcePath, const QString &destPath, bool overwrite=false);

/**
 * @brief copyRecursively
 * @param sourcePath
 * @param destPath
 * @return
 */
bool copyRecursively(const QString &sourcePath, const QString &destPath, bool overwrite=false);


/**
 * @brief Rename batch of files in given dirPath
 * @param dirPath absolute directory path
 * @param from start index of being renamed files
 * @param offset number to be added to each file
 * @param to end index of being renamed files
 * @return number of files renamed
 *
 * The opeartion is done in the same directory
 */
int batchRenameByIndex(const QString &dirPath, int from, int offset, int to = 0);

/**
 * @brief Rename batch of files in given dirPath
 * @param dirPath absolute directory path
 * @param from name of start file without extension
 * @param offset number to be added to each file
 * @param to name of end file without extension
 * @return number of files renamed
 *
 * The opeartion is done in the same directory
 */
bool batchRenameByName(const QString &dirPath, int from, int offset, int to = 0);

/**
 * @brief count number of files of type <prefixNumber>-%d.<extension>
 * @param dirPath absolute directory path where files are
 * @param prefixNumber target files' names start with this number
 * @param target files' extension, don't forget attaching * (*.wav, *.mp3, etc)
 * @return number of matching files
 */
int countSubfiles(const QString &dirPath, int prefixNumber, const QString &extension = "*.WAV");

/**
 * @brief attach certain suffix to the given file (i.e. .../../3.wav -> .../../3A.wav)
 * @param absoluteFilePath absolute path of the file
 * @param suffix a string to be attached at the end of file name
 * @return renamed file name if renaming is successful, otherwise empty string.
 */
QString attachSuffixToFileName(const QString &absoluteFilePath, const QString &suffix = "A");

/**
 * @brief deletes all files in the directory but not the directory itself
 * @param absolutePath absolute path to the directory
 * @return true if successful, false if error occurs
 */
bool deleteAllFilesInDirecotry(const QString &absolutePath);

/**
 * @brief tailPath adds directory separator at the end if not exist according to the platform
 * @param path the path
 * @return path with directory separator at the end
 */
QString tailPath(const QString &path);

/**
 * @brief recursively get directory content
 * @param list each entry is added to this list
 * @param destDir the directory we are working on
 */
void recursivelyGetDirectoryContent(QStringList *list, const QString &destDir, int depth);

/**
 * @brief bytesToSeconds converts size in bytes to length in seconds
 * @param byteSize size in bytes
 * @return
 */
quint64 bytesToSeconds(quint64 byteSize);

/**
 * @brief secondsToBytes converts length in seconds to size in bytes
 * @param minutes length in minutes
 * @return
 */
quint64 secondsToBytes(int seconds);

#endif // FUNCTIONS_H
