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

#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <set>

#include <QTableWidget>
#include <QFileInfoList>

#include "audiobookconverter.h"
#include "define.h"

class QObject;
class QDragMoveEvent;
class QDragEnterEvent;
class QProcess;
class QProgressDialog;
class QPushButton;
class QMenu;
class QWidgetAction;

class CDRipper;
class AudioInfoThread;

const int COLUMN_COUNT                      = 8;
const int SCISSORS_COLUMN_INDEX             = 0;
const int SPLIT_COLUMN_INDEX                = 1;
const int DURATION_COLUMN_INDEX             = 2;
const int PLAYBUTTON_COLUMN_INDEX           = 3;
const int METADATA_TITLE_COLUMN_INDEX       = 4;
const int METADATA_ALBUM_COLUMN_INDEX       = 5;
const int METADATA_COMMENT_COLUMN_INDEX     = 6;
const int ID_COLUMN_INDEX                   = 7;

const int DEFAULT_ROW_HEIGHT                = 25;

const int SPLITABLE_AUDIO_LENGTH_LIMIT      = 4; // in mins

/**
 * @brief The PlaylistView class is custom table widget to show playlist
 */
class PlaylistView : public QTableWidget
{
    Q_OBJECT
public:

    /**
     * @brief PlaylistView constructor
     * @param parent
     */
    PlaylistView(QWidget *parent = nullptr);

    ~PlaylistView();

    /**
     * @brief load
     */
    void load(const AudioList &, quint8 dirNum);

    /**
     * @brief addEntries
     */
    void addEntries(const AudioList &, bool readFromDrive);

    /**
     * @brief addEntry
     */
    void addEntry(const AudioEntry &, bool readFromDrive);

    /**
     * @brief append one or more items to this playlist
     * @param list the list to append to this playlist
     */
    void insertBatch(const AudioList &list, bool readFromDrive);

    /**
     * @brief insert one or more items to this playlist at a specific position
     * @param list the list to insert into this playlist
     * @param index the index where to insert the items
     */
    void insertBatchAt(const AudioList &list, int index, bool readFromDrive);

    /**
     * @brief insertEntry
     * @return false if memory is full
     */
    bool insertEntry(AudioEntry, int , bool readFromDrive);

    /**
     * @brief remove
     */
    void remove();

    /**
     * @brief removeEntryByTableIndex
     * @return
     */
    bool removeEntryByTableIndex(int );

    /**
     * @brief removeEntryByEntryID
     * @return
     */
    bool removeEntryByEntryID(int );

    /**
     * @brief setText
     */
    void setText(int, int, const QString &);

    /**
     * @brief text
     * @return
     */
    QString text(int, int);

    /**
     * @brief addSilence
     * @param secs
     */
    void addSilence(int secs);

    /**
     * @brief insertSilence
     * @param secs
     * @param insertAt
     */
    void insertSilence(int secs, int insertAt);

    /**
     * @brief useID
     * @return
     */
    int useID();

    /**
     * @brief maxID
     * @return
     */
    int maxID();

    /**
     * @brief getSelectedEntries
     * @return
     */
    AudioList getSelectedEntries();

    /**
     * @brief removeEntries
     * @param list
     */
    void removeEntries(const AudioList &list);

    /**
     * @brief getEntryList
     * @return
     */
    AudioList getEntryList();

    /**
     * @brief stopPlayer
     */
    void stopPlayer();

    /**
     * @brief read audio files and audio tracks from drives or from Audio CD
     * @param fileList string list of absolute file paths
     * @param rowIndex row index where the entries would be added
     */
    void readEntries(const QFileInfoList &fileInfoList, int rowIndex);

    /**
     * @brief setDropIndicatorColor sets color of drop indicator
     * @param color QColor we want to set to indicator
     */
    void setDropIndicatorColor(const QColor& color);

    /**
     * @brief setDriveSpaceDetails
     * @param used
     * @param total
     */
    void setDriveSpaceDetails(quint64 used, quint64 total);

    /**
     * @brief setColumnVisible
     * @param index
     * @param visible
     */
    void setColumnVisible(int index, bool visible);

signals:

     /**
      * @brief this signal is emitted when an entry is added or removed
      * @param seconds
      */
     void durationChanged(int playlistIntex, int seconds);

     /**
      * @brief this signal is emitted when error occurs
      * @param errorString
      */
     void errorOccurred(const QString &errorString);

     /**
      * @brief this signal is emmitted when the playlist is pristine or modified
      * @param errorString
      */
     void currentPlaylistIsUntouched(bool isUntouched);


protected:
     virtual void dragEnterEvent(QDragEnterEvent *);
     virtual void dragLeaveEvent(QDragLeaveEvent *);
     virtual void dragMoveEvent(QDragMoveEvent *);
     virtual void dropEvent(QDropEvent *);
     virtual void paintEvent(QPaintEvent *);
     virtual void resizeEvent(QResizeEvent *);
     virtual void keyPressEvent(QKeyEvent *e);
     virtual void contextMenuEvent(QContextMenuEvent *e);

protected slots:

     /**
      * @brief onCellChanged called when a cell value is changed
      * @param row row index
      * @param col column index
      */
     void onCellChanged(int row, int col);

private:

     /**
     * @brief updatePlayButton update play button image according to its status
     * @param id identifier of entry
     * @param flag indicates the state of button
     */
     void updatePlayButton(int id, bool flag);

     /**
      * @brief setRowBackground set background of specific row
      * @param brush brush to paint the row
      * @param model model of table
      * @param row row index to be coloured
      * @param parent parent model index
      */
     void setRowBackground(const QBrush& brush, QAbstractItemModel* model, int row, const QModelIndex& parent = QModelIndex());

     /**
      * @brief convertSecToDesired convert seconds(duration) into printable format
      * @param secs number of seconds
      * @return string in printable format
      */
     QString convertSecToDesired(int secs);

     /**
      * @brief getRowIndexFromEntryID gets row index with entry ID
      * @param id entry id
      * @return row index where the entry is located
      */
     int getRowIndexFromEntryID(int id);

     /**
      * @brief droppingOnItself determines whether dragged item is dropped on the row itself
      * @param event
      * @param index
      * @return
      */
     bool droppingOnItself(QDropEvent *event, const QModelIndex &index);

     /**
      * @brief dropOn determines where the dragged item is dropped
      * @param event drop event
      * @param dropRow row index where the dragged item is dropped
      * @param dropCol column index where the dragged item is dropped
      * @param dropIndex model index of dropped item
      * @return
      */
     bool dropOn(QDropEvent *event, int *dropRow, int *dropCol, QModelIndex *dropIndex);

     /**
      * @brief position gets drop indicator position
      * @param pos
      * @param rect
      * @param index
      * @return
      */
     QAbstractItemView::DropIndicatorPosition position(const QPoint &pos, const QRect &rect, const QModelIndex &index) const;
     std::set<int> getSelectedRowsFast(QItemSelection const &selection);

     /**
      * @brief changeEntryState modify entry state
      * @param id entry identifier
      * @param state entry's new state
      */
     void changeEntryState(int id, int state);

    /**
     * @brief parsePlaylist parses playlist file and gets list of file information for each file in playlist
     * @param absoluteFilePath absolute path to the playlist file
     * @return QFileInfoList
     */
    QFileInfoList parsePlaylist(const QString &absoluteFilePath);

    /**
     * @brief parseAudioBook parses m4b audio book file and convert it to mp3 files based on chapters
     * @param absoluteFilePath absolute path to the m4b file
     * @return QFileInfoList list of converted mp3 files in temp directory
     *
     * @note this will use ffmpeg to get information of audio book and to convert to mp3 files
     */
    QFileInfoList parseAudioBook(const QString &absoluteFilePath);

    QList<QStringList> m_commands;
    QList<QStringList> m_itemList;
    AudioList m_data;

    QPixmap *m_backgroundPix;   // the translucent background image

    int m_maxID;
    quint8 m_dirNum;

    quint64 m_usedSpace;
    quint64 m_totalSpace;

    int m_playingEntryID;
    QProcess *m_player;

    AudioBookConverter m_audioBookConverter;

    CDRipper *m_ripperThread;
    AudioInfoThread *m_infoThread;

    QProgressDialog *m_progress;
    QPushButton *m_abortButton;
    bool m_isAborted;

    int m_prevIndicatorRowIndex;
    QColor m_indicatorColor;

    bool m_isDirty;     // has anything been modified that needs to be written to the card?
};

#endif // PLAYLISTVIEW_H
