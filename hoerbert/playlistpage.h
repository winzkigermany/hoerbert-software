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

#ifndef PLAYLISTPAGE_H
#define PLAYLISTPAGE_H

#include <QWidget>

#include "define.h"

class QComboBox;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QFileInfo;
class QMenu;

class PieButton;
class PlaylistView;

/**
 * @brief The PlaylistPage class represents a page to work on a playlist
 */
class PlaylistPage : public QWidget
{
    Q_OBJECT
public:

    /**
     * @brief PlaylistPage constructor
     * @param parent
     */
    explicit PlaylistPage(QWidget *parent = Q_NULLPTR);

    /**
     * @brief set the absolute path to selected directory and directory number, list all files in it
     */
    void setListData(const QString &, quint8 , const AudioList &);

    /**
     * @brief return the directory number
     * @return
     */
    quint8 directory();

    /**
     * @brief return path to current directory
     * @return
     */
    QString directoryPath();

    /**
     * @brief set background color of container frame
     * @param color
     */
    void setBackgroundColor(const QColor &color);

    /**
     * @brief move selected entries from current directory to desired directory
     * @param to_dir
     * @param add2beginning
     */
    void moveSelectedEntriesTo(quint8 to_dir, bool add2beginning);

    /**
     * @brief clearDirectoryEstimation once processing a dir is completed, need to clear out estimation
     * @param dirIndex directory index
     */
    void clearDirectoryEstimation(quint8 dirIndex);

    /**
     * @brief getPlaylistView return a pointer to the playlistView
     * @return
     */
    const PlaylistView *getPlaylistView();

signals:

    /**
     * @brief cancelClicked
     */
    void cancelClicked();

    /**
     * @brief commitChanges
     * @param list
     */
    void commitChanges(const QMap<ENTRY_LIST_TYPE, AudioList>&, const quint8);

    /**
     * @brief emitted when an entry is added or removed
     * @param seconds seconds added or removed
     */
    void durationChanged(int seconds);

    /**
     * @brief emitted when error occurs
     * @param errorString
     */
    void errorOccurred(const QString &errorString);

public slots:
    /**
     * @brief add silence to the position
     */
    void addSilence();

    /**
     * @brief add files through file dialog
     */
    void add();

    /**
     * @brief remove selected files from playlist
     */
    void remove();

    /**
     * @brief discard any changes and close playlist
     */
    void discard();

    /**
     * @brief setDriveSpaceInfo
     * @param used
     * @param total
     */
    void setDriveSpaceDetails(quint64 used, quint64 total, quint64 estimatedSeconds);
protected:
    void contextMenuEvent(QContextMenuEvent *e);

private slots:

    /**
     * @brief this signal is emitted when the page is about to close(commit/cancel)
     * @param doCommitChanges indicates whether the closing is due to commit or cancel
     */
    void onClosePage(bool doCommitChanges);

    /**
     * @brief this signal must open the configuration menu for the view configuration
     */
    void setTableColumns();

private:
    /**
     * @brief getSelectedSilenceDurationInSeconds get duration of silence from combobox in seconds
     * @return duration in seconds
     */
    int getSelectedSilenceDurationInSeconds();

    /**
     * @brief updateColumnVisibleSettings update settings of column visibility
     * @param columnName name of column to be set
     * @param visible visibility flag
     */
    void updateColumnVisibleSettings(const QString &columnName, bool visible);

    /**
     * @brief columnVisibleFromSettings retrieve visibility of column from settings
     * @param columnName name of column to be considered
     * @return visibility of the column
     */
    bool columnVisibleFromSettings(const QString &columnName);

    quint8 m_dirNum;
    QString m_dir;

    AudioList m_clipBoard;

    QMap<int, AudioList> m_originalList;
    QMap<int, AudioList> m_implicitlyMovedList;

    QPushButton *m_viewConfigButton;
    QLabel *m_silenceLabel;
    QComboBox *m_silenceDuration;
    PieButton *m_addSilenceButton;
    PieButton *m_addButton;
    PieButton *m_removeButton;
    PieButton *m_commitButton;
    PieButton *m_cancelButton;
    PlaylistView *m_playlistView;

    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_toolLayout;
    QHBoxLayout *m_leftToolLayout;
    QHBoxLayout *m_centerToolLayout;
    QHBoxLayout *m_rightToolLayout;

    QPalette m_pal;

    QMenu *m_configViewMenu;
    QAction *m_actionAlbumVisible;
    QAction *m_actionCommentVisible;
};

#endif // PLAYLISTPAGE_H
