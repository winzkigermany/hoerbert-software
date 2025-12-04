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

#include "playlistview.h"

#include <QHeaderView>
#include <QMimeData>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QProgressDialog>
#include <QPushButton>
#include <QProcess>
#include <QCoreApplication>
#include <QLabel>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMenu>
#include <QSettings>
#include <QDebug>

#include "triplecheckboxwidget.h"
#include "audioinfothread.h"
#include "playsymbolbutton.h"
#include "cdripper.h"
#include "playlistparser.h"
#include "functions.h"

int CDRipper::uniqueID = 0;

using namespace std;

extern QString FFPLAY_PATH;
extern QString HOERBERT_TEMP_PATH;

PlaylistView::PlaylistView(QWidget *parent)
    : QTableWidget(parent)
{
    m_isDirty = false;

    m_maxID = 0;

    m_backgroundPix = new QPixmap(":/images/hoerbert.png");

    m_playingEntryID = -1;

    m_prevIndicatorRowIndex = -1;

    m_indicatorColor = QColor(0, 75, 215);

    m_usedSpace = 0;

    m_totalSpace = 0;

    m_player = new QProcess(this);
    m_player->setProcessChannelMode(QProcess::MergedChannels);

    horizontalHeader()->setMinimumSectionSize(DEFAULT_ROW_HEIGHT);
    horizontalHeader()->hide();
    verticalHeader()->hide();
    setShowGrid(false);
    setDropIndicatorShown(true);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDragEnabled(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    //setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropOverwriteMode(false);
    setAlternatingRowColors(true);
    setObjectName("PlaylistTable");
    setTabKeyNavigation(false);
    horizontalHeader()->setStretchLastSection(true);


    setColumnCount(8);
    hideColumn(ID_COLUMN_INDEX);

    setColumnWidth(SCISSORS_COLUMN_INDEX, DEFAULT_ROW_HEIGHT + 10);
    setColumnWidth(SPLIT_COLUMN_INDEX, DEFAULT_ROW_HEIGHT + 10);
#if defined (Q_OS_WIN)
    setColumnWidth(DURATION_COLUMN_INDEX, 60);
#else
    setColumnWidth(DURATION_COLUMN_INDEX, 75);
#endif
    setColumnWidth(PLAYBUTTON_COLUMN_INDEX, 40);

    connect(m_player, &QProcess::stateChanged, this, [this] (QProcess::ProcessState newState) {
       if (newState == QProcess::NotRunning)
       {
           updatePlayButton(m_playingEntryID, false);
       }
    });

    connect(m_player, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this] (int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitCode)
        Q_UNUSED(exitStatus)
        updatePlayButton(m_playingEntryID, false);
    });

    connect(this, &QTableWidget::currentCellChanged, this, [this] (int currentRow, int currentColumn, int previousRow, int previousColumn) {
        Q_UNUSED(currentColumn)
        Q_UNUSED(previousColumn)
        if (currentRow != previousRow)
        {
            stopPlayer();
        }
    });
}

PlaylistView::~PlaylistView()
{
    if (m_player->state() == QProcess::Running)
    {
        m_player->close();
        m_player->deleteLater();
    }
}

void PlaylistView::load(const AudioList &list, quint8 dirNum)
{
    m_data.clear();
    clear();
    setRowCount(0);
    m_dirNum = dirNum;

    addEntries(list, true);
    resizeColumnsToContents();
    currentPlaylistIsUntouched( true );
}

void PlaylistView::addEntries(const AudioList &list, bool readFromDrive)
{
    for (const auto& entry : list)
    {
        addEntry(entry, readFromDrive);
    }
}

void PlaylistView::addEntry(const AudioEntry &entry, bool readFromDrive)
{
    auto index = rowCount();
    insertEntry(entry, index, readFromDrive);
}

void PlaylistView::insertBatch(const AudioList &list, bool readFromDrive)
{
    int index = -1;
    if (currentRow() != -1)
        index = currentRow() + 1;
    else
        index = rowCount() + 1;
    insertBatchAt(list, index, readFromDrive);
}

void PlaylistView::insertBatchAt(const AudioList &list, int index, bool readFromDrive)
{
    if (index == -1 || index > rowCount())
        index = rowCount();

    for (const auto& entry : list)
    {
        if (!insertEntry(entry, index, readFromDrive))
            break;
        index++;
    }

    resizeColumnsToContents();
}

bool PlaylistView::insertEntry(AudioEntry entry, int index, bool readFromDrive=false)
{
    if (!readFromDrive) {
        quint64 expectedFileSizeInBytes = WAV_HEADER_SIZE_IN_BYTES + (entry.duration * 32000 * 16 / 8) + MEMORY_SPARE_SPACE_IN_BYTES;

        qDebug() << expectedFileSizeInBytes << bytesToSeconds(m_totalSpace);
        if ( expectedFileSizeInBytes > (m_totalSpace-m_usedSpace) ) {
            QMessageBox::information(this, tr("Memory card is full"), tr("The card is already full, you can't add more files to it."), QMessageBox::Ok);
            return false;
        }
    }

    currentPlaylistIsUntouched( false );

    if (index >= rowCount())
    {
        index = rowCount();
        setRowCount(rowCount() + 1);
    } else {
        insertRow(index);
        setRowHeight(index, DEFAULT_ROW_HEIGHT);
    }
    setRowHeight(index, DEFAULT_ROW_HEIGHT);

    if (entry.duration > SPLITABLE_AUDIO_LENGTH_LIMIT * 60)
    {
        QWidget *scissors_widget = new QWidget(this->viewport());

        QLabel *scissors_pix = new QLabel(scissors_widget);
        scissors_pix->setScaledContents(true);

        QSettings settings;
        settings.beginGroup("Global");
        bool darkMode = settings.value("darkMode").toBool();
        settings.endGroup();

        if( darkMode )
        {
            scissors_pix->setPixmap(QPixmap(":/images/scissors_dark.png"));
        }
        else
        {
            scissors_pix->setPixmap(QPixmap(":/images/scissors.png"));
        }

        scissors_pix->setFixedSize(DEFAULT_ROW_HEIGHT * 0.8, DEFAULT_ROW_HEIGHT * 0.8);


        QHBoxLayout *scissors_layout = new QHBoxLayout(scissors_widget);
        scissors_layout->setAlignment(Qt::AlignCenter);
        scissors_layout->setContentsMargins(0, 0, 0, 0);
        scissors_layout->addWidget(scissors_pix);

        setCellWidget(index, SCISSORS_COLUMN_INDEX, scissors_widget);

        QTableWidgetItem *item = new QTableWidgetItem();
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        setItem(index, SPLIT_COLUMN_INDEX, item);

        TripleCheckBoxWidget *check_box = new TripleCheckBoxWidget(this->viewport(), entry.id);
        setCellWidget(index, SPLIT_COLUMN_INDEX, check_box);
        connect(check_box, &TripleCheckBoxWidget::stateChanged, [this](int id, quint8 state) {
            this->changeEntryState(id, state);
        });
    }
    else
    {
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        setItem(index, SCISSORS_COLUMN_INDEX, item);

        item = new QTableWidgetItem();
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        setItem(index, SPLIT_COLUMN_INDEX, item);
    }

    QTableWidgetItem *item = new QTableWidgetItem();
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    setItem(index, PLAYBUTTON_COLUMN_INDEX, item);

    PlaySymbolButton *play_button = new PlaySymbolButton(this->viewport(), entry.id);
    play_button->setButtonSize(16, 16);
    play_button->setFixedHeight(DEFAULT_ROW_HEIGHT);
    setCellWidget(index, PLAYBUTTON_COLUMN_INDEX, play_button);
    connect(play_button, &PlaySymbolButton::stateChanged, [this](int id, bool state) {
        // play audio

        if (m_playingEntryID != id && m_player->state() == QProcess::Running)
        {
            m_player->close();

            updatePlayButton(m_playingEntryID, false);
        }
        m_playingEntryID = id;
        if (!state)
            m_player->close();
        else {

            QStringList arguments;
            arguments.append("-nodisp");
            arguments.append("-autoexit");
            arguments.append(m_data[id].path);

            m_player->start(FFPLAY_PATH, arguments);
        }
    });

    this->setText(index, DURATION_COLUMN_INDEX, convertSecToDesired(entry.duration));

    if (!readFromDrive) {
        entry.metadata.comment = entry.path;
    }
    this->setText(index, METADATA_TITLE_COLUMN_INDEX, entry.metadata.title);
    this->setText(index, METADATA_ALBUM_COLUMN_INDEX, entry.metadata.album);
    this->setText(index, METADATA_COMMENT_COLUMN_INDEX, entry.metadata.comment);
    this->setText(index, ID_COLUMN_INDEX, QString::number(entry.id));

    entry.order = index;

    m_maxID = entry.id;

    m_data.insert(entry.id, entry);

    quint64 duration = entry.duration;

    if (!readFromDrive) {
        emit durationChanged(m_dirNum, duration);
    }

    this->resizeColumnsToContents();
    return true;
}

void PlaylistView::remove()
{
    std::list<int> remove_list;
    for (const auto& index : selectionModel()->selectedRows())
    {
        remove_list.push_back(index.row());
    }

    /* we do sort the list by descending order and remove from the last
       in other way, from beginning, the indices will be updated so we will remove wrong rows */
    remove_list.sort(greater<int>());

    for (std::list<int>::iterator it = remove_list.begin(); it != remove_list.end(); it++)
    {
        removeEntryByTableIndex(*it);
    }
}

bool PlaylistView::removeEntryByTableIndex(int index)
{
    if (index == -1 || index >= rowCount())
        return false;
    // do actual stuff here to remove audio entry and row

    // we get entry id from the row
    auto entry_id = item(index, ID_COLUMN_INDEX)->text().toInt();

    // remove the row on the table
    removeRow(index);

    emit durationChanged(m_dirNum, -(m_data[entry_id].duration));
    currentPlaylistIsUntouched( false );
    return m_data.remove(entry_id);
}

bool PlaylistView::removeEntryByEntryID(int id)
{
    for (int i=0; i<rowCount(); i++)
    {
        if (text(i, ID_COLUMN_INDEX).toInt() == id)
        {
            removeRow(i);
            emit durationChanged(m_dirNum, -(m_data[id].duration));
            currentPlaylistIsUntouched( false );
            return m_data.remove(id);
        }
    }
    return false;
}

void PlaylistView::changeEntryState(int id, int state)
{
    m_data[id].state = state;
}

void PlaylistView::addSilence(int secs)
{
    if (currentRow() == -1)
        insertSilence(secs, rowCount());
    else
        insertSilence(secs, currentRow() + 1);
}

void PlaylistView::insertSilence(int secs, int index)
{
    if (secs < 0)
    {
        qDebug() << "Invalid number of seconds:" << secs;
        return;
    }

    currentPlaylistIsUntouched( false );
    if (index == -1)
        index = rowCount();

    if (index > rowCount())
    {
        index = rowCount();
    }

    AudioEntry entry;
    entry.id = useID();
    entry.order = index;
    entry.path = "";
    entry.state = 0;
    entry.duration = secs;
    entry.metadata.title = "silence";
    entry.flag = 5; // silence flag

    m_data[entry.id] = entry;
    insertEntry(entry, index);
}

void PlaylistView::setText(int row, int col, const QString &str)
{
    if (item(row, col) == nullptr)
    {
        QTableWidgetItem *item = new QTableWidgetItem();
        if (col != METADATA_TITLE_COLUMN_INDEX)
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        setItem(row, col, item);
    }
    item(row, col)->setText(str);
    if (col == DURATION_COLUMN_INDEX)
    {
        item(row, col)->setTextAlignment(Qt::AlignCenter);
    }
    else if (col == METADATA_TITLE_COLUMN_INDEX)
    {
        item(row, col)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    }
}

QString PlaylistView::text(int row, int col)
{
    return item(row, col)->text();
}

void PlaylistView::updatePlayButton(int id, bool flag)
{
    PlaySymbolButton *button = static_cast<PlaySymbolButton*>(cellWidget(getRowIndexFromEntryID(id), PLAYBUTTON_COLUMN_INDEX));
    if (button != nullptr)
    {
        if (!flag)
            button->stop();
        else
            button->play();
    }
}

void PlaylistView::setRowBackground(const QBrush &brush, QAbstractItemModel *model, int row, const QModelIndex &parent)
{
    if(!model || row < 0 || row >= model->rowCount(parent))
        return;

    if(parent.isValid() && parent.model() != model)
        return;

    for(int i = 0; i < model->columnCount(parent); ++i)
        Q_ASSUME(model->setData(model->index(row, i, parent), brush, Qt::BackgroundRole));
}

QString PlaylistView::convertSecToDesired(int secs)
{
    if (secs > 60 * 60)
        return QDateTime::fromTime_t(static_cast<uint>(secs)).toUTC().toString("[hh:mm:ss]");
    else
        return QDateTime::fromTime_t(static_cast<uint>(secs)).toUTC().toString("[mm:ss]");
}

int PlaylistView::getRowIndexFromEntryID(int id)
{
    if (id < 0)
        return -1;

    for (int i=0; i< rowCount(); i++)
    {
        if (text(i, ID_COLUMN_INDEX).toInt() == id)
            return i;
    }

    return -1;
}

int PlaylistView::useID()
{
    return ++m_maxID;
}

int PlaylistView::maxID()
{
    return m_maxID;
}

AudioList PlaylistView::getSelectedEntries()
{
    AudioList ret;
    int entry_id = 0;
    for (const auto& index : selectionModel()->selectedRows())
    {
        entry_id = text(index.row(), ID_COLUMN_INDEX).toInt();
        ret[entry_id] = m_data[entry_id];
    }
    return ret;
}

void PlaylistView::removeEntries(const AudioList &list)
{
    for (const auto& item : list)
        removeEntryByEntryID(item.id);
}

AudioList PlaylistView::getEntryList()
{
    for (int i = 0; i < rowCount(); i++)
    {
        int entry_id = text(i, ID_COLUMN_INDEX).toInt();
        if (m_data[entry_id].order != i) {
            m_data[entry_id].order = i;
        }
        m_data[entry_id].metadata.title = text(i, METADATA_TITLE_COLUMN_INDEX);
    }
    return m_data;
}

void PlaylistView::stopPlayer()
{
    if (m_player->state() == QProcess::Running)
    {
        updatePlayButton(m_playingEntryID, false);
        m_player->close();
    }
}

void PlaylistView::readEntries(const QFileInfoList &fileInfoList, int rowIndex)
{
    qDebug() << "read entries call ";
    qDebug() << fileInfoList;
    setCursor(Qt::WaitCursor);
    QFileInfoList rip_info_list;
    QFileInfoList *file_info_list = new QFileInfoList;
    QStringList *metadata_list = new QStringList;

    QString fileName = QString();

    for (const auto& file_info : fileInfoList)
    {
        fileName = file_info.fileName();
        if (fileName.endsWith(".cda") || fileName.endsWith(".aiff"))
        {
            rip_info_list.append(file_info);
        }
        else if (fileName.endsWith(".m3u") || fileName.endsWith(".m3u8") || fileName.endsWith("pls")
                 || fileName.endsWith("wpl") || fileName.endsWith("xml") || fileName.endsWith("xspf"))
        {
            file_info_list->append(parsePlaylist(file_info.absoluteFilePath()));
        }
        else if (fileName.endsWith(".m4b"))
        {
            file_info_list->append(parseAudioBook(file_info.absoluteFilePath()));
        }
        else {
            file_info_list->append(file_info);
        }
    }

    if (rip_info_list.count() > 0)
    {
        m_ripperThread = new CDRipper();

        m_progress = new QProgressDialog(this);
        m_progress->setWindowTitle(tr("Ripping Audio CD"));
        m_progress->setLabelText(tr("Ripping audio tracks..."));
        m_progress->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
        m_progress->setModal(true);
        m_progress->setFixedWidth(360);
        m_progress->setRange(0, 100);

        m_isAborted = false;

        m_abortButton = new QPushButton(m_progress);
        m_abortButton->setText(tr("Abort"));

        // setting cancel button connects the button to the dialog to hide the dialog on button click
        m_progress->setCancelButton(m_abortButton);
        // need to disconnect the connections since clicking on the button immediately hides the dialog
        m_progress->disconnect(m_abortButton);

        // then define custom connections
        connect(m_abortButton, &QPushButton::clicked, this, [this] () {
            m_progress->setLabelText(tr("Aborting..."));
            m_abortButton->setDisabled(true);
            m_ripperThread->abort();
            m_isAborted = true;
        });

        m_ripperThread->setFileInfoList(rip_info_list);

        connect(m_ripperThread, &CDRipper::taskCompleted, this, [file_info_list, metadata_list] (const QStringList &tmpFileList) {
            // even number indices represent actual file paths while odd number indices represent metadata of previous audio
            for (int i = 0; i < tmpFileList.count(); i++)
            {
                if (i % 2 == 0)
                    file_info_list->append(QFileInfo(tmpFileList.at(i)));
                else
                    metadata_list->append(tmpFileList.at(i));
            }
        });

        connect(m_ripperThread, &CDRipper::processUpdated, this, [this] (int percentage) {
            m_progress->setLabelText(tr("Ripping audio tracks..."));
            m_progress->setValue(percentage);
        });

        connect(m_ripperThread, &CDRipper::failed, this, [this] (const QString &errorString) {
            this->errorOccurred("CDRipper\n" + errorString);
        });

        connect(m_ripperThread, &QThread::finished, this, [this] () {
            m_ripperThread->deleteLater();
            m_progress->setValue(0);
            m_progress->close();
            m_progress->deleteLater();
            this->setCursor(Qt::ArrowCursor);
        });

        m_progress->show();

        QEventLoop loop;
        connect(m_ripperThread, &QThread::finished, &loop, &QEventLoop::quit);
        m_ripperThread->start();
        loop.exec();

        if (m_isAborted == true) {
            m_isAborted = false;
            return;
        }
    }

    m_infoThread = new AudioInfoThread();

    if (rip_info_list.count() > 0) {
        if (file_info_list->count() != metadata_list->count())
        {
            QStringList tmp_list;
            for (int i = 0; i < file_info_list->count() - metadata_list->count(); i++)
            {
                tmp_list.append(file_info_list->at(i).absoluteFilePath());
            }
            *metadata_list = tmp_list + *metadata_list;
        }
        m_infoThread->setFileListWithMetadata(*file_info_list, *metadata_list);
    }
    else
        m_infoThread->setFileList(*file_info_list);

    m_infoThread->setBeginID(maxID() + 1);
    m_infoThread->setDeafultFlag(0);

    connect(m_infoThread, &AudioInfoThread::taskCompleted, this, [this, rowIndex] (const AudioList &result) {
        this->insertBatchAt(result, rowIndex, false);
    });

    connect(m_infoThread, &QThread::finished, this, [this, file_info_list, metadata_list] () {
        delete file_info_list;
        delete metadata_list;
        m_infoThread->deleteLater();
        setCursor(Qt::ArrowCursor);
    });
    m_infoThread->start();
}

void PlaylistView::setDropIndicatorColor(const QColor &color)
{
    m_indicatorColor = color;
}

void PlaylistView::setDriveSpaceDetails(quint64 used, quint64 total)
{
    m_usedSpace = used;
    m_totalSpace = total;
}


void PlaylistView::setColumnVisible(int index, bool visible)
{
    if (visible)
        showColumn(index);
    else
        hideColumn(index);

    resizeColumnsToContents();
}

QFileInfoList PlaylistView::parsePlaylist(const QString &absoluteFilePath)
{
    QString extension = QFileInfo(absoluteFilePath).suffix().section(".", -1, -1);

    QFileInfoList info_list;
    PlaylistParser parser;
    QStringList path_list;

    if (extension.compare("m3u") == 0)
    {
        parser.setTextParseDetails();
    }
    else if (extension.compare("m3u8") == 0)
    {
        parser.setTextParseDetails();
    }
    else if (extension.compare("pls") == 0)
    {
        parser.setTextParseDetails();
    }
    else if (extension.compare("wpl") == 0)
    {
        parser.setTagParseDetails("media", "src");
    }
    else if (extension.compare("xml") == 0)
    {
        parser.setTextParseDetails(true, "<key>Location</key>", "", true, "<string>");
    }
    else if (extension.compare("xspf") == 0)
    {
        parser.setTextParseDetails(false, "#", "", true, "<location>");
    }
    else
    {
        qDebug() << "Unknown playlist type:" << absoluteFilePath;
        return info_list;
    }
    path_list = parser.get(absoluteFilePath);

    for (const auto& path : path_list)
    {
        if (QFile::exists(path))
        {
            info_list << QFileInfo(path);
        }
        else
        {
            if (path.startsWith("..") || path.startsWith(QDir::separator() + QString("..")))
            {
                qDebug() << "The path is relative to the generator. Cannot specify where it is from." << path;
            }
            qDebug() << "The file in playlist does not exist!" << path;
        }
    }

    return info_list;
}

QFileInfoList PlaylistView::parseAudioBook(const QString &absoluteFilePath)
{
    m_progress = new QProgressDialog(this);
    m_progress->setWindowTitle(tr("Converting Audio Book"));
    m_progress->setLabelText(tr("Converting..."));
    m_progress->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    m_progress->setModal(true);
    m_progress->setFixedWidth(360);
    m_progress->setRange(0, 100);
    m_isAborted = false;

    m_abortButton = new QPushButton(m_progress);
    m_abortButton->setText(tr("Abort"));

    // setting cancel button connects the button to the dialog to hide the dialog on button click
    m_progress->setCancelButton(m_abortButton);
    // need to disconnect the connections since clicking on the button immediately hides the dialog
    m_progress->disconnect(m_abortButton);

    // then define custom connections
    connect(m_abortButton, &QPushButton::clicked, this, [this] () {
        m_progress->setLabelText(tr("Aborting..."));
        m_abortButton->setDisabled(true);
        m_isAborted = true;
        m_audioBookConverter.abort();
    });

    m_progress->show();
    m_progress->setValue(0);

    connect(&m_audioBookConverter, &AudioBookConverter::processUpdated, this, [this] (int percentage) {
        m_progress->setValue(percentage);
    });

    connect(&m_audioBookConverter, &AudioBookConverter::failed, this, [this] (const QString &errorString) {
        errorOccurred("AudioBookConverter\n" + errorString);
    });
    QFileInfoList file_list;
    connect(&m_audioBookConverter, &AudioBookConverter::finished, this, [&file_list] (QFileInfoList files) {
        file_list = files;
    });

    QEventLoop loop;
    QTimer::singleShot(0,[&]{m_audioBookConverter.convert(absoluteFilePath);});
    connect(&m_audioBookConverter, &AudioBookConverter::finished, &loop, &QEventLoop::quit);
    loop.exec();

    m_progress->close();
    m_progress->deleteLater();
    m_abortButton->deleteLater();
    return file_list;
}

void PlaylistView::onCellChanged(int row, int col)
{
    if (col == METADATA_TITLE_COLUMN_INDEX)
    {
        if (item(row, ID_COLUMN_INDEX))
        {
            m_data[text(row, ID_COLUMN_INDEX).toInt()].metadata.title = text(row, col);
        }
    }
}

void PlaylistView::dragEnterEvent(QDragEnterEvent *event)
{
    stopPlayer();

    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
    QTableWidget::dragEnterEvent(event);
}

void PlaylistView::dragLeaveEvent(QDragLeaveEvent *event)
{
    if (m_prevIndicatorRowIndex != -1)
    {
        removeRow(m_prevIndicatorRowIndex);
        m_prevIndicatorRowIndex = -1;
    }
    QTableWidget::dragLeaveEvent(event);
}

void PlaylistView::dragMoveEvent(QDragMoveEvent *event)
{
    int indicator_row = -1;

    auto y_pos = event->pos().y();

    QModelIndex index;
    // rootIndex() (i.e. the viewport) might be a valid index
    if (viewport()->rect().contains(event->pos()))
    {
        index = this->indexAt(event->pos());
        if (!index.isValid() || !this->visualRect(index).contains(event->pos()))
        {
            index = rootIndex();
        }
    }

    if (index.row() == 0 && y_pos < DEFAULT_ROW_HEIGHT / 2)
    {
        indicator_row = 0;
    }
    else {
        indicator_row = index.row();
    }

    if (m_prevIndicatorRowIndex != indicator_row)
    {
        if (m_prevIndicatorRowIndex != -1)
        {
            removeRow(m_prevIndicatorRowIndex);
        }
        insertRow(indicator_row);
        setRowBackground(m_indicatorColor, this->model(), indicator_row);
    }

    m_prevIndicatorRowIndex = indicator_row;

    currentPlaylistIsUntouched( false );
    QTableWidget::dragMoveEvent(event);
}

void PlaylistView::dropEvent(QDropEvent * event)
{
    int dropped_row = m_prevIndicatorRowIndex;
    if (m_prevIndicatorRowIndex != -1)
    {
        removeRow(m_prevIndicatorRowIndex);
        m_prevIndicatorRowIndex = -1;
    }

    if (event->source() == this && (event->dropAction() == Qt::MoveAction || dragDropMode() == QAbstractItemView::InternalMove))
    {// internal move
        QModelIndex topIndex;
        int row = -1;
        int col = -1;
        if (this->dropOn(event, &row, &col, &topIndex))
        {
            std::set<int> selRows = getSelectedRowsFast(selectionModel()->selection());

            int top = *(selRows.begin());
            int dropRow = row;

            // in case user wants to drop at very top of list

            /*if (dropRow == -1)
            {
                dropRow = rowCount();
            }*/

            dropRow = dropped_row;
            int offset = dropRow - top;
            //qDebug() << "SelRows:" << *selRows.begin() << "DropRow:" << dropRow << " TOp:" << top << "Offset:" << offset;

            // insert new rows
            std::set<int>::const_iterator it = selRows.begin();
            int r = *it + offset ;
            for( ; it != selRows.end(); ++it)
            {
                r = qMin(r, rowCount());
                r = qMax(r, 0);
                insertRow(r);
                //qDebug() << "Inserting row at " << r;
                r++;
            }

            // copy data

            // note: row numbers may have changed after inserts, hence refresh selRows
            selRows = getSelectedRowsFast(selectionModel()->selection());
            //qDebug() << "SelRows:" << *selRows.begin();
            top = *(selRows.begin());
            offset = dropRow - top;

            it = selRows.begin();
            r = *it + offset;

            for(; it != selRows.end(); ++it)
            {
                r = qMin(r, rowCount());
                r = qMax(r, 0);

                for(int c = 0; c < columnCount(); ++c)
                {
                    if (c == 0)
                    {
                        if (cellWidget(*it, SCISSORS_COLUMN_INDEX))
                        {
                            QWidget *scissors_widget = static_cast<QWidget*>(cellWidget(*it, SCISSORS_COLUMN_INDEX));
                            setCellWidget(r, c, scissors_widget);
                        }
                    }

                    if (c == 1)
                    {
                        if (cellWidget(*it, SPLIT_COLUMN_INDEX))
                        {
                            TripleCheckBox *check_box = static_cast<TripleCheckBox*>(cellWidget(*it, SPLIT_COLUMN_INDEX));
                            setCellWidget(r, c, check_box);
                        }

                        continue;
                    }

                    if (c == 3)
                    {
                        if (cellWidget(*it, PLAYBUTTON_COLUMN_INDEX))
                        {
                            PlaySymbolButton *play_button = static_cast<PlaySymbolButton*>(cellWidget(*it, PLAYBUTTON_COLUMN_INDEX));
                            setCellWidget(r, c, play_button);
                        }

                        continue;
                    }

                    QTableWidgetItem *source = takeItem(*it, c);
                    setItem(r, c, source);
                }
                setRowHeight(r, DEFAULT_ROW_HEIGHT);
                r++;
            }

            // delete selected rows.
            std::set<int>::const_reverse_iterator rit;
            for(rit = selRows.rbegin(); rit != selRows.rend(); ++rit)
            {
                removeRow(*rit);
                //qDebug() << "Removing row at " << *rit;
            }

            currentPlaylistIsUntouched( false );
            event->accept();
        }
    }
    else
    { // drag & drop from outside (i.e. Finder or explorer)
        QModelIndex topIndex;
        int row = -1;
        int col = -1;
        if (this->dropOn(event, &row, &col, &topIndex))
        {

            QStringList supported_format_list;

            for (const auto& filter : QString(SUPPORTED_FILES).split(" "))
            {
                supported_format_list << filter.section("*.", 1);
            }

            QFileInfoList file_info_list;
            for (const QUrl &url : event->mimeData()->urls())
            {
                QString fileName = url.toLocalFile();
                QFileInfo info(fileName);
                if (info.isDir()) {
                    QDir dir(fileName);
                    if (!dir.exists())

                    dir.setFilter(QDir::Files | QDir::NoSymLinks);
                    dir.setNameFilters(QString(SUPPORTED_FILES).split(" "));
                    dir.setSorting(QDir::Name);

                    file_info_list << dir.entryInfoList();
                }
                else if (info.isFile()) {
                    if (supported_format_list.contains(info.suffix().section(".", -1, -1).toLower()))
                        file_info_list << info;
                    else {
                        qDebug() << supported_format_list;
                        qDebug() << "Unsupported file format" << info.fileName();
                    }
                } else {
                    qDebug() << "Unknown file type:" << info;
                }
            }

            if (file_info_list.count() > 0)
                event->acceptProposedAction();

            readEntries(file_info_list, dropped_row);
        }

        currentPlaylistIsUntouched( false );
        QTableView::dropEvent(event);
    }
}

void PlaylistView::paintEvent(QPaintEvent *e)
{
/*
    if(!m_backgroundPix->isNull())
    {
        QPainter painter(viewport());
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        auto w = m_backgroundPix->size().width();
        auto h = m_backgroundPix->size().height();
        painter.drawPixmap((viewport()->width() - w) / 2, (viewport()->height() - h) / 2, w, h, *m_backgroundPix);
        painter.end();
    }
*/
    QTableWidget::paintEvent(e);
}

void PlaylistView::resizeEvent(QResizeEvent *e)
{
    QTableView::resizeEvent(e);
}

void PlaylistView::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Space)
    {
        PlaySymbolButton *button = static_cast<PlaySymbolButton*>(cellWidget(this->currentRow(), PLAYBUTTON_COLUMN_INDEX));
        if (button != nullptr)
        {
            updatePlayButton(text(this->currentRow(), ID_COLUMN_INDEX).toULongLong(), !button->isPlaying());
        }
    }
    else if (e->key() == Qt::Key_Return)
    {
        QTableWidgetItem *edit_item = this->item(this->currentRow(), METADATA_TITLE_COLUMN_INDEX);
        if (edit_item != nullptr)
        {
            this->editItem(edit_item);
        }
    }
    else
        QTableWidget::keyPressEvent(e);
}

void PlaylistView::contextMenuEvent(QContextMenuEvent *e)
{
    Q_UNUSED(e)
    // don't show the parent's context menu here, because we might need a different context menu for our playlist rows.
    // m_contextMenu->exec(e->globalPos());
}

std::set<int> PlaylistView::getSelectedRowsFast(const QItemSelection &selection)
{
    std::set<int> selRows;

    for (QItemSelectionRange const& range : selection)
    {
        if (range.isValid() && range.model())
        {
            for (int row = range.top(); row <= range.bottom(); ++row)
            {
                for (int column = range.left(); column <= range.right(); ++column)
                {
                    QModelIndex index = range.model()->index(row, column, range.parent());
                    Qt::ItemFlags flags = range.model()->flags(index);
                    if ((flags & Qt::ItemIsSelectable) && (flags & Qt::ItemIsEnabled))
                    {
                        if (selRows.insert(index.row()).second == false)
                        {
                            break; // insert failed, we assume we've already got the entry
                        }
                    }
                }
            }
        }
    }
    return selRows;
}

bool PlaylistView::droppingOnItself(QDropEvent *event, const QModelIndex &index)
{
    Qt::DropAction dropAction = event->dropAction();
    if (this->dragDropMode() == QAbstractItemView::InternalMove)
    {
        dropAction = Qt::MoveAction;
    }
    if (event->source() == this
            && event->possibleActions() & Qt::MoveAction
            && dropAction == Qt::MoveAction)
    {
        QModelIndexList selectedIndexes = this->selectedIndexes();
        QModelIndex child = index;
        while (child.isValid() && child != rootIndex())
        {
            if (selectedIndexes.contains(child))
                return true;
            child = child.parent();
        }
    }
    return false;
}

bool PlaylistView::dropOn(QDropEvent *event, int *dropRow, int *dropCol, QModelIndex *dropIndex)
{
    if (event->isAccepted())
        return false;

    QModelIndex index;
    // rootIndex() (i.e. the viewport) might be a valid index
    if (viewport()->rect().contains(event->pos()))
    {
        index = this->indexAt(event->pos());
        if (!index.isValid() || !this->visualRect(index).contains(event->pos()))
        {
            index = rootIndex();
        }
    }

    // if we are allowed to do the drop
    if (model()->supportedDropActions() & event->dropAction())
    {
        int row = -1;
        int col = -1;
        QAbstractItemView::DropIndicatorPosition dropIndicatorPosition;
        if (index != rootIndex())
        {
            dropIndicatorPosition = position(event->pos(), this->visualRect(index), index);
            switch (dropIndicatorPosition)
            {
            case QAbstractItemView::AboveItem:
                row = index.row();
                col = index.column();
                index = index.parent();
                break;
            case QAbstractItemView::BelowItem:
                row = index.row() + 1;
                col = index.column();
                index = index.parent();
                break;
            case QAbstractItemView::OnItem:
            case QAbstractItemView::OnViewport:
                row = index.row() + 1;
                col = index.column();
                break;
            }
        } else
        {
            dropIndicatorPosition = QAbstractItemView::OnViewport;
        }

        *dropIndex = index;
        *dropRow = row;
        *dropCol = col;
        if (!droppingOnItself(event, index))
            return true;
    }
    return false;
}

QAbstractItemView::DropIndicatorPosition PlaylistView::position(const QPoint &pos, const QRect &rect, const QModelIndex &index) const
{
    QAbstractItemView::DropIndicatorPosition r = QAbstractItemView::OnViewport;

    const int margin = 2;
    if (pos.y() - rect.top() < margin)
    {
        r = QAbstractItemView::AboveItem;
    }
    else if (rect.bottom() - pos.y() < margin)
    {
        r = QAbstractItemView::BelowItem;
    }
    else if (rect.contains(pos, true))
    {
        r = QAbstractItemView::OnItem;
    }


    if (r == QAbstractItemView::OnItem && (!(model()->flags(index) & Qt::ItemIsDropEnabled)))
    {
        r = pos.y() < rect.center().y() ? QAbstractItemView::AboveItem : QAbstractItemView::BelowItem;
    }

    return r;
}
