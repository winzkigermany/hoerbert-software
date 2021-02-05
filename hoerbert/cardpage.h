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

#ifndef CARDPAGE_H
#define CARDPAGE_H


#include <list>
#include <QWidget>
#include <QMutex>
#include <QDir>
#include <QStackedWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLayout>
#include <QMessageBox>
#include <QLineEdit>
#include <QTimer>
#include <cmath>
#include <QFileSystemWatcher>
#include <QShortcut>
#include <QProgressDialog>
#include <QApplication>
#include <QSettings>
#include <QDebug>
#include <QInputDialog>
#include <QVBoxLayout>

#include "define.h"
#include "functions.h"
#include "piebutton.h"
#include "capacitybar.h"
#include "audioinfothread.h"
#include "xmlwriter.h"
#include "pleasewaitdialog.h"
#include "devicemanager.h"
#include "mainwindow.h"
#include "removabledrivelistener.h"

class MainWindow;

const int GRID_SPACING              = 16;
const int MOUNT_VOLUME_DELAY        = 2; // in seconds

typedef std::shared_ptr<DeviceManager> DeviceManager_ptr;

/**
 * @brief The CardPage class represents a page to deal with memory cards and to show directories in them
 */
class CardPage : public QWidget
{
    Q_OBJECT
public:

    /**
     * @brief CardPage constructor
     * @param parent
     */
    explicit CardPage(QWidget *parent = Q_NULLPTR);

    /**
     * @brief CardPage destructor
     */
    ~CardPage();

    enum DIRECTORIES {
        DIR0 = 0,
        DIR1,
        DIR2,
        DIR3,
        DIR4,
        DIR5,
        DIR6,
        DIR7,
        DIR8
    };

    /**
     * @brief format a memory card
     */
    void formatSelectedDrive(bool retry = false);

    /**
     * @brief eject a memory card
     */
    bool ejectDrive();

    /**
     * @brief recreate hoerbert.xml
     */
    void recreateXml();

    /**
     * @brief get the color for a given playlist number
     * @param id
     * @return color of the given playlist number
     */
    QColor getPlaylistColor(quint8 id);

    /**
     * @brief update
     */
    void update();

    /**
     * @brief updateButtons
     * @param count
     */
    void updateButtons();

    /**
     * @brief set the percent value for the pie chart (completion percent)
     * @param buttonIndex the button to set
     * @param percentage the percentage to set
     */
    void sendPercent(int buttonIndex, int percentage);

    /**
     * @brief set the enabled state of a button
     * @param buttonIndex the button to set
     * @param flag true=enabled, false=disabled
     */
    void setButtonEnabled(int buttonIndex, bool flag);

    /**
     * @brief absolute path of selected card
     */
    QString currentDrivePath();

    /**
     * @brief label of selected drive
     */
    QString currentDriveName();

    /**
     * @brief set whether the app is processing or not
     */
    void setIsProcessing(bool flag);

    /**
     * @brief isProcessing
     * @return true when there's commiting on progress, otherwise false
     */
    bool isProcessing();


    /**
     * @brief set whether hoerbertXML needs to be rewritten
     */
    void setHoerbertXMLDirty( bool yesNo );

    /**
     * @brief isHoerbertXMLDirty
     * @return true when there has been some write action of any kind on the playlists
     */
    bool isHoerbertXMLDirty();

    /**
     * @brief update the numbers: space used and space available
     */
    void initUsedSpace();

    /**
     * @brief enable/disable buttons for card management on the page
     * @param flag flag
     */
    void setCardManageButtonsEnabled(bool flag);

    /**
     * @brief switchDiagnosticsMode
     * @param enabled true when diagnostics mode is enabled
     */
    void switchDiagnosticsMode(bool enabled);

    /**
     * @brief isDiagnosticsModeEnabled
     * @return
     */
    bool isDiagnosticsModeEnabled();

    /**
     * @brief get volume size of current memory card
     * @return
     */
    qint64 getCurrentVolumeSize();

    /**
     * @brief get available space of current memory card
     * @return
     */
    qint64 getCurrentAvailableSpace();

    /**
     * @brief get filesystem type of memory card
     * @return
     */
    QString getCardFileSystemType();

    /**
     * @brief numberOfTracks
     * @return the overall number of tracks on this card
     */
    int numberOfTracks();

    /**
     * @brief select drive by its full path on file system
     * @param path absolute path to the drive storage
     */
    void selectDriveByPath(const QString &path);

    /**
     * @brief releaseButtonLock
     */
    void releaseButtonLock();

    /**
     * @brief enable or disable the playlist buttons
     * @param onOff if true, enable the playlist buttons
     */
    void enableButtons( bool onOff );

    /**
     * @brief getDriveListLength
     * @return the number of drives in the drop down list
     */
    int getDriveListLength();

    /**
     * @brief isWorkingOnCustomDirectory
     * @return true if we're working on a custom directory instead of a memory card.
     */
    bool isWorkingOnCustomDirectory();

    /**
     * @brief getDropDownText
     * @return the text that is currently visible in the dropdown.
     */
    QString getSelectedDrive();

    /**
     * @brief getDisplayedDrive
     * @return the drive that is currently visible in the comboBox.
     */
    QString getDisplayedDrive();

signals:

    /**
     * @brief this signal is emitted when the playlist is selected
     * @param id playlist identifier
     * @param drivePath absolute path to the directory on the drive
     * @param audioList list of audio info in the playlist
     */
    void playlistChanged(qint8 id, const QString &drivePath, const AudioList &audioList);

    /**
     * @brief this signal is emitted when there's change in drive space
     * @param used bytes
     * @param total bytes
     */
    void driveCapacityUpdated(quint64 used, quint64 total);

    /**
     * @brief this signal is emitted when a drive selected for further edit
     */
    void driveSelected(const QString &);

    /**
     * @brief this signal is emitted when user selects a memory card which is used with old software last time
     */
    void migrationNeeded(const QString &dirPath);

    /**
     * @brief this signal is emitted when user switches diagnostics mode
     * @param enabled
     */
    void toggleDiagnosticsMode();

    /**
     * @brief this signal is emitted when the memory card has plausibility issues
     * @param fixList
     */
    void plausibilityFixNeeded(std::list <int> fixList);

    /**
     * @brief enable or disable menu->edit buttons
     */
    void enableEditMenuItems( bool );

    /**
     * @brief signals the percentage of long running processes
     */
    void sendProgressPercent( int );

    /**
     * @brief signals the current number of drives in the drive list
     */
    void driveListChanged( int );

public slots:

    /**
     * @brief updateDriveList updates list of currently available devices
     */
    void updateDriveList();

    /**
     * @brief commitUsedSpace
     * @param playlistIndex
     * @param playlistFolder
     */
    void commitUsedSpace( int playlistIndex );


    void updateEstimatedDuration( int playlistIndex, quint64 seconds );

private slots:

    /**
     * @brief called when a playlist button is clicked
     * @param dirIndex index of playlist (equals to actual directory name)
     */
    void onPlaylistButtonClicked(qint8 dirIndex);

private:

    /**
     * @brief initialize playlist buttons
     */
    void initializePlaylists();

    /**
     * @brief read the drive and shows directories
     * @param driveName drive label + root path
     */
    void selectDrive(const QString &driveName, bool doUpdateCapacityBar=true );

    /**
     * @brief initialize playlist buttons and hide playlist details
     */
    void deselectDrive();

    /**
     * @brief hasFat32WarningShown helps to remember whether we've shown the warning once.
     */
    bool m_hasFat32WarningShown;

    /**
     * @brief sendDriveCapacity sends information about space used + max space available in bytes
     */
    void sendDriveCapacity();

    bool m_isProcessing;
    bool m_migrationSuggested;
    bool m_hoerbertXMLIsDirty;

    DeviceManager_ptr m_deviceManager;

    RemovableDriveListener* m_windowsDriveListener ;

    QWidget *m_cardMngContainer;
    QComboBox *m_driveList;
    QPushButton *m_selectDriveButton;
    PieButton *m_ejectDriveButton;
    QLabel *m_ejectButtonLabel;
    QHBoxLayout *m_cardMngLayout;
    QVBoxLayout *m_mainLayout;

    QStackedWidget *m_stackWidget;

    QWidget *m_gridWidget;
    QGridLayout *m_gridLayout;

    QWidget *m_diagWidget;
    QVBoxLayout *m_diagLayout;
    QLabel *m_diagModeHint;
    QPushButton *m_return2Normal;

    PieButton *m_dir0;
    PieButton *m_dir1;
    PieButton *m_dir2;
    PieButton *m_dir3;
    PieButton *m_dir4;
    PieButton *m_dir5;
    PieButton *m_dir6;
    PieButton *m_dir7;
    PieButton *m_dir8;
    QMap<int, PieButton*> m_dirs;

    QSpacerItem *m_horizontalGridSpacer1;
    QSpacerItem *m_horizontalGridSpacer2;
    QSpacerItem *m_verticalGridSpacer1;
    QSpacerItem *m_verticalGridSpacer2;
    QSpacerItem *m_leftGridSpacer;
    QSpacerItem *m_rightGridSpacer;

    QMutex m_audioInputThreadMutex;

    MainWindow* m_mainWindow;   // our parent
    PleaseWaitDialog* m_pleaseWaitDialog;

    quint64 m_total_bytes = 0;
    quint64 m_playlistSize[9];
    quint64 m_playlistEstimatedSize[9];
    quint64 m_usedSpaceOffset = 0;           // this is the used space on the card that's NOT in any playlist.
    bool m_isFormatting = false;                // this flag tells us if a formatting operation is underway and keeps us from being afraid of sudden loss of the memory card.
};

#endif // CARDPAGE_H
