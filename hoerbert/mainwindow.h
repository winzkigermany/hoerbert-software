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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QMutex>

#include "define.h"

class QWidget;
class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QStackedWidget;
class QGraphicsDropShadowEffect;
class QAction;
class QMenu;
class QProgressDialog;
class QMutex;

class AboutDialog;
class AdvancedFeaturesDialog;
class CardPage;
class PlaylistPage;
class CapacityBar;
class HoerbertProcessor;
class BackupManager;
class DebugDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    /**
     * @brief MainWindow constructor
     * @param parent
     */
    MainWindow(QWidget *parent = Q_NULLPTR);

    /**
     * @brief MainWindow destructor
     */
    ~MainWindow();

    /**
     * @brief fix plausibility issues
     * @param fixList list of folder indices which need plausibility fix
     */
    void makePlausible(std::list <int> fixList);

signals:
    /**
     * @brief changeAlbumColumnVisibility is a signal that's sent when the user toggles visibility of the album column
     * @return
     */
    void changeAlbumColumnVisibility( bool onOff );

    /**
     * @brief changeAlbumColumnVisibility is a signal that's sent when the user toggles visibility of the comment column
     * @return
     */
    void changeCommentColumnVisibility( bool onOff );


private slots:
    void addTitle();
    void removeTitle();
    void moveToAnotherPlaylist(quint8 toDir, bool toBeginning);

    void printTableOfContent(const QString &outputPath = QString(), bool showOnBrowser = true);
    void backupCard();
    void restoreBackup();
    void formatCard();
    void advancedFeatures();
    void selectDestinationManually();

    void switchDiagnosticsMode();

    void about();
    void checkForUpdates();

    void processCommit(const QMap<ENTRY_LIST_TYPE, AudioList> &list, const quint8 dir_index);
    void processorErrorOccurred(const QString &errorString);
    void taskCompleted(int failCount, int totalCount);

    void migrate(const QString &dirPath);

    void printHtml(const AudioList &list, const QString &outputPath = QString(), bool showOnBrowser = true);

    void sync();

    void collectInformationForSupport();
    void showHideEditMenuEntries( bool );

private:

    void closeEvent(QCloseEvent *e);

    void showVersion(const QString &version);

    void remindBackup();

    void createActions();

    bool copyRecursively(const QString &sourceFolder, const QString &destFolder);

    bool m_isWritingToDisk;
    QString m_migrationPath;
    BackupManager *m_backupManager;
    QProgressDialog *m_progress;

    QStackedWidget *m_stackWidget;
    AboutDialog *m_aboutDlg;
    AdvancedFeaturesDialog *m_featuresDlg;
    DebugDialog *m_dbgDlg;
    CardPage *m_cardPage;
    PlaylistPage *m_playlistPage;

    QVBoxLayout *m_layout;
    QHBoxLayout *m_infoLayout;
    QWidget *m_centralWidget;

    QString m_selectedDriveLabel;

    CapacityBar *m_capBar;
    QGraphicsDropShadowEffect *m_shadow;

    QMenu *m_moveToPlaylistMenu;
    QMenu *m_subMenuBegin;
    QMenu *m_subMenuEnd;
    QMenu *m_extrasMenu;
    QMenu *m_viewMenu;
    QMenu *m_subMenuColumns;
    QMenu *m_helpMenu;
    QMenu *m_editMenu;
    QMenu *m_serviceToolsMenu;

    QAction *m_addTitleAction;
    QAction *m_removeTitleAction;
    QAction *m_printAction;
    QAction *m_backupAction;
    QAction *m_restoreAction;
    QAction *m_formatAction;
    QAction *m_advancedFeaturesAction;
    QAction *m_selectManually;
    QAction *m_showAlbumAction;
    QAction *m_showPathAction;
    QAction *m_darkModeAction;

    QAction *m_moveToB1;
    QAction *m_moveToB2;
    QAction *m_moveToB3;
    QAction *m_moveToB4;
    QAction *m_moveToB5;
    QAction *m_moveToB6;
    QAction *m_moveToB7;
    QAction *m_moveToB8;
    QAction *m_moveToB9;

    QAction *m_moveToE1;
    QAction *m_moveToE2;
    QAction *m_moveToE3;
    QAction *m_moveToE4;
    QAction *m_moveToE5;
    QAction *m_moveToE6;
    QAction *m_moveToE7;
    QAction *m_moveToE8;
    QAction *m_moveToE9;

    QAction *aboutAction;
    QAction *findBooksAction;
    QAction *checkUpdatesAction;

    QAction *m_visitServiceWebsiteAction;
    QAction *m_collectDataAction;
    QAction *m_toggleDiagnosticsModeAction;

    QMutex m_plausibilityCheckMutex;

    QMap<int, QString> m_errorLog;
};

#endif // MAINWINDOW_H
