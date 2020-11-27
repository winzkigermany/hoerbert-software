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

#include "cardpage.h"


CardPage::CardPage(QWidget *parent)
    : QWidget(parent)
{
    m_deviceManager = nullptr;
    m_deviceManager = std::make_shared<DeviceManager> ();

    m_isProcessing = false;
    m_migrationSuggested = false;

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(30, 10, 30, 30);
    m_mainLayout->setSpacing(10);

    m_cardMngContainer = new QWidget(this);
    m_cardMngContainer->setFixedHeight(42);

    m_cardMngLayout = new QHBoxLayout(m_cardMngContainer);
    m_cardMngLayout->setMargin(0);
    m_cardMngLayout->setSpacing(10);

    m_driveList = new QComboBox(this);
    m_driveList->setObjectName("DriveComboBox");
    m_driveList->setEditable(false);
    m_driveList->setToolTip(tr("Shows available memory cards or drives of this computer"));
//    m_driveList->setMinimumSize(100, 42);   // minimum height - this looks better because of the eject button making the whole row jump when it appears otherwise.

    m_selectDriveButton = new QPushButton(this);
    m_selectDriveButton->setText(tr("Read Card"));
    m_selectDriveButton->setToolTip(tr("Read the selected card to edit it")+" ("+QString(tr("Ctrl+r"))+")" );
    m_selectDriveButton->setShortcut(QKeySequence(tr("Ctrl+r")) );

    m_ejectDriveButton = new PieButton(this);
    m_ejectDriveButton->setFixedSize(32, 32);
    m_ejectDriveButton->setOverlayPixmap(QPixmap(":/images/pie_overlay.png"));
    m_ejectDriveButton->setMainPixmap(QPixmap(":/images/eject.png"));
    m_ejectDriveButton->setShadowEnabled(false);
    m_ejectDriveButton->setToolTip( tr("Save all changes and eject card")+" ("+QString(tr("Ctrl+j"))+")" );
    m_ejectDriveButton->setShortcut( QKeySequence(tr("Ctrl+j")) );
    m_ejectDriveButton->hide();

    m_ejectButtonLabel = new QLabel(this);
    m_ejectButtonLabel->setFixedHeight(32);
    m_ejectButtonLabel->setAlignment(Qt::AlignCenter);
    m_ejectButtonLabel->setText(tr("Eject card"));
    m_ejectButtonLabel->setObjectName("ejectLabel");
    m_ejectButtonLabel->setStyleSheet("#ejectLabel {color:#353535;}");
    m_ejectButtonLabel->hide();

    m_cardMngLayout->addWidget(m_driveList, 5);
    m_cardMngLayout->addWidget(m_selectDriveButton);
    m_cardMngLayout->addWidget(m_ejectButtonLabel);
    m_cardMngLayout->addWidget(m_ejectDriveButton);

    m_mainLayout->addWidget(m_cardMngContainer);

    m_gridWidget = new QWidget(this);

    m_gridLayout = new QGridLayout(m_gridWidget);
    m_gridLayout->setSpacing(30);
    m_gridLayout->setHorizontalSpacing(30);
    m_gridLayout->setVerticalSpacing(30);

    m_dir0 = new PieButton(m_gridWidget, 0);
    m_dir1 = new PieButton(m_gridWidget, 1);
    m_dir2 = new PieButton(m_gridWidget, 2);
    m_dir3 = new PieButton(m_gridWidget, 3);
    m_dir4 = new PieButton(m_gridWidget, 4);
    m_dir5 = new PieButton(m_gridWidget, 5);
    m_dir6 = new PieButton(m_gridWidget, 6);
    m_dir7 = new PieButton(m_gridWidget, 7);
    m_dir8 = new PieButton(m_gridWidget, 8);

    m_dir0->setBackgroundColor(CL_DIR0);
    m_dir1->setBackgroundColor(CL_DIR1);
    m_dir2->setBackgroundColor(CL_DIR2);
    m_dir3->setBackgroundColor(CL_DIR3);
    m_dir4->setBackgroundColor(CL_DIR4);
    m_dir5->setBackgroundColor(CL_DIR5);
    m_dir6->setBackgroundColor(CL_DIR6);
    m_dir7->setBackgroundColor(CL_DIR7);
    m_dir8->setBackgroundColor(CL_DIR8);

    m_dirs[DIR0] = m_dir0;
    m_dirs[DIR1] = m_dir1;
    m_dirs[DIR2] = m_dir2;
    m_dirs[DIR3] = m_dir3;
    m_dirs[DIR4] = m_dir4;
    m_dirs[DIR5] = m_dir5;
    m_dirs[DIR6] = m_dir6;
    m_dirs[DIR7] = m_dir7;
    m_dirs[DIR8] = m_dir8;

    for (const auto& dir_button : m_dirs)
    {
        dir_button->setToolTip(tr("Edit playlist") + QString(" %1 (Ctrl+%1)").arg(dir_button->ID() + 1));
        QShortcut *shortcut = new QShortcut(QKeySequence(QString("Ctrl+%1").arg(dir_button->ID() + 1)), this);
        connect(shortcut, &QShortcut::activated, this, [this, dir_button] () {
            if (dir_button->percentage() > 0)
                return;
            if (!m_deviceManager->selectedDrive().isEmpty())
                this->onPlaylistButtonClicked(dir_button->ID());
        });
    }

    m_horizontalGridSpacer1 = new QSpacerItem(GRID_SPACING, GRID_SPACING, QSizePolicy::Fixed, QSizePolicy::Minimum);
    m_horizontalGridSpacer2 = new QSpacerItem(GRID_SPACING, GRID_SPACING, QSizePolicy::Fixed, QSizePolicy::Minimum);
    m_leftGridSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_rightGridSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_verticalGridSpacer1 = new QSpacerItem(GRID_SPACING, GRID_SPACING, QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_verticalGridSpacer2 = new QSpacerItem(GRID_SPACING, GRID_SPACING, QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_gridLayout->addItem(m_leftGridSpacer, 0, 0, 1, 1);
    m_gridLayout->addWidget(m_dir0, 0, 1, 1, 1);
    m_gridLayout->addItem(m_horizontalGridSpacer1, 0, 2, 1, 1);
    m_gridLayout->addWidget(m_dir1, 0, 3, 1, 1);
    m_gridLayout->addItem(m_horizontalGridSpacer1, 0, 4, 1, 1);
    m_gridLayout->addWidget(m_dir2, 0, 5, 1, 1);
    m_gridLayout->addItem(m_rightGridSpacer, 0, 6, 1, 1);
    m_gridLayout->addItem(m_verticalGridSpacer1, 1, 1, 1, 1);
    m_gridLayout->addWidget(m_dir3, 2, 1, 1, 1);
    m_gridLayout->addWidget(m_dir4, 2, 3, 1, 1);
    m_gridLayout->addWidget(m_dir5, 2, 5, 1, 1);
    m_gridLayout->addItem(m_verticalGridSpacer2, 3, 1, 1, 1);
    m_gridLayout->addWidget(m_dir6, 4, 1, 1, 1);
    m_gridLayout->addWidget(m_dir7, 4, 3, 1, 1);
    m_gridLayout->addWidget(m_dir8, 4, 5, 1, 1);

    m_diagWidget = new QWidget(this);

    m_diagLayout = new QVBoxLayout(m_diagWidget);
    m_diagLayout->setAlignment(Qt::AlignCenter);

    m_diagModeHint = new QLabel(m_diagWidget);
    m_diagModeHint->setObjectName("diagModeHintLabel");
    m_diagModeHint->setStyleSheet("#diagModeHintLabel {color:#353535;}");   // always: dark text on wooden background
    m_diagModeHint->setText(tr("This memory card is in diagnostics mode."));
    m_diagModeHint->setFont(QFont("Console", 12, 50));

    m_return2Normal = new QPushButton(m_diagWidget);
    m_return2Normal->setText(tr("Return to normal mode"));
    m_return2Normal->setToolTip(tr("Return to normal mode from diagnostics mode"));
    m_return2Normal->setFixedHeight(50);
    m_return2Normal->setObjectName("BigButton");

    m_diagLayout->addWidget(m_diagModeHint);
    m_diagLayout->addWidget(m_return2Normal);

    m_stackWidget = new QStackedWidget(this);
    m_stackWidget->addWidget(m_gridWidget);
    m_stackWidget->addWidget(m_diagWidget);

    m_mainLayout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
    m_mainLayout->addWidget(m_stackWidget);
    m_mainLayout->addSpacerItem(new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding));

    m_driveList->addItems(m_deviceManager->getDeviceList());

    updateButtons();

    for (const auto& key : m_dirs.keys())
    {
        connect(m_dirs.value(key), &QPushButton::clicked, [this, key] (){
            if (m_dirs[key]->percentage() > 0)
                return;
            if (!m_deviceManager->selectedDrive().isEmpty())
                this->onPlaylistButtonClicked(static_cast<qint8>(key));
        });
    }

    connect(m_selectDriveButton, &QPushButton::clicked, [this] () {
        this->selectDrive(this->m_driveList->currentText());
    });
    connect(m_ejectDriveButton, &QPushButton::clicked, this, &CardPage::ejectDrive);

    connect(m_return2Normal, &QPushButton::clicked, this, [this] () {
        emit this->diagnosticsModeSwitched();
    });

#if defined (Q_OS_WIN)
    m_deviceWatcher = new QDeviceWatcher;
    m_deviceWatcher->appendEventReceiver(this);

    connect(m_deviceWatcher, SIGNAL(deviceAdded(const QString&)), this, SLOT(onDeviceAdded(const QString&)), Qt::DirectConnection);
    connect(m_deviceWatcher, SIGNAL(deviceRemoved(const QString&)), this, SLOT(onDeviceRemoved(const QString&)), Qt::DirectConnection);
    m_deviceWatcher->start();
#elif defined (Q_OS_MACOS)
    m_watcher.addPath("/Volumes");
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, [this] (const QString &path) {
        Q_UNUSED(path)
        qDebug() << "Volume change detected!";
        QTimer::singleShot(MOUNT_VOLUME_DELAY * 1000, this, &CardPage::updateDriveList);
    });
#elif defined (Q_OS_LINUX)
    QString user_name = qgetenv("USER");
    if (!user_name.isEmpty())
    {
        m_watcher.addPath(QString("/media/%1").arg(user_name));
        connect(&m_watcher, &QFileSystemWatcher::directoryChanged, [this] (const QString &path) {
            Q_UNUSED(path)
            qDebug() << "Volume change detected!";
            QTimer::singleShot(MOUNT_VOLUME_DELAY * 1000, this, &CardPage::updateDriveList);
        });
    }
#endif
}

CardPage::~CardPage()
{
    if (m_gridLayout) {
        m_gridLayout->removeItem(m_horizontalGridSpacer1);
        m_gridLayout->removeItem(m_horizontalGridSpacer2);
        m_gridLayout->removeItem(m_leftGridSpacer);
        m_gridLayout->removeItem(m_rightGridSpacer);
        m_gridLayout->removeItem(m_verticalGridSpacer1);
        m_gridLayout->removeItem(m_verticalGridSpacer2);
        delete m_horizontalGridSpacer1;
        delete m_horizontalGridSpacer2;
        delete m_leftGridSpacer;
        delete m_rightGridSpacer;
        delete m_verticalGridSpacer1;
        delete m_verticalGridSpacer2;
    }
#if defined (Q_OS_WIN)
    m_deviceWatcher->stop();
    m_deviceWatcher->deleteLater();
#endif
}

void CardPage::onDeviceAdded(const QString &dev)
{
    Q_UNUSED(dev)
#if defined (Q_OS_MACOS) || defined (Q_OS_LINUX)
        QTimer::singleShot(MOUNT_VOLUME_DELAY * 1000, [this, dev] () {
            m_deviceManager->getDeviceList();
            if (m_deviceManager->isValidDevice(dev))
                updateDriveList();
        });
#else
        updateDriveList();
#endif
}

void CardPage::onDeviceRemoved(const QString &dev)
{
    Q_UNUSED(dev)
    updateDriveList();
}

void CardPage::updateDriveList()
{
    m_driveList->clear();
    m_driveList->setCurrentText(QString());
    m_driveList->addItems(m_deviceManager->getDeviceList());

    bool selectedDriveExists = false;
    if (!m_deviceManager->selectedDriveName().isEmpty())
    {
        for (const auto& drive : m_deviceManager->getDeviceList()) {
            if (drive.compare(m_deviceManager->selectedDriveName()) == 0) {
                selectedDriveExists = true;
                m_driveList->setCurrentText(m_deviceManager->selectedDriveName());
            }
        }
    }

    if (!selectedDriveExists || m_deviceManager->selectedDrive().isEmpty())
    {
        initializePlaylists();
        //selectDrive(m_driveList->currentText());
    }

    if (m_driveList->count() > 0)
    {
        m_selectDriveButton->setEnabled(true);
        m_ejectDriveButton->setEnabled(true);
    }
    else
    {
        m_selectDriveButton->setEnabled(false);
        m_ejectDriveButton->setEnabled(false);
    }
}

void CardPage::formatSelectedDrive(bool retry)
{
    if( m_deviceManager->isWorkingOnCustomDirectory() ){
        QMessageBox::information(this, tr("Format"), tr("You have selected a custom path to work on. For safety purposes, this app does not format such paths. Please format your memory card manually yourself."));
        return;
    }

    QString selectedDrive = m_driveList->currentText();
    if( selectedDrive.isEmpty() ){
        return;
    }

    if (!retry)
    {
        auto selected = QMessageBox::question(this, tr("Format"), tr("Are you sure you want to format this drive [%1] ?").arg(selectedDrive), QMessageBox::Yes|QMessageBox::No, QMessageBox::No );

        if (selected == QMessageBox::No)
            return;
    }

    QString passwd = "";
    bool ok = false;
#ifdef Q_OS_LINUX
    auto label = tr("If you do not want to enter your root password here,\nplease format the memory card in terminal with the following command:")
            + "\n\numount [device]\nmkfs.vfat [device]\n";

    passwd = QInputDialog::getText(this, tr("Permission required"), label, QLineEdit::Password, QString(), &ok);
    if (!ok || passwd.isEmpty())
        return;
#endif

    auto driveLabel = QInputDialog::getText(this, tr("Format"), tr("Please enter the new name for your card"), QLineEdit::Normal, DEFAULT_DRIVE_LABEL, &ok);

    m_deviceManager->formatDrive(this, selectedDrive, driveLabel, passwd);

    updateDriveList();
    deselectDrive();
}

void CardPage::ejectDrive()
{
    if (m_isProcessing)
    {
        auto selected = QMessageBox::question(this, tr("Eject"), QString(tr("Current drive [%1] is being processed.")+"\n\n"+tr("Are you sure you want to eject the drive?")).arg(m_deviceManager->selectedDriveName()), QMessageBox::Yes|QMessageBox::No, QMessageBox::No );

        if (selected == QMessageBox::No)
            return;
    }

    QString currentDevice = m_driveList->currentText();
    if (currentDevice.isEmpty())
    {
        QMessageBox::information(this, tr("Eject"), tr("No memory card is selected.")+"\n"+tr("Please select the device you want to eject."));
        updateDriveList();
        deselectDrive();
        return;
    }

    QSettings settings;
    settings.beginGroup("Global");
    bool regenerateHoerbertXml = settings.value("regenerateHoerbertXml").toBool();
    settings.endGroup();
    if( this->isHoerbertXMLDirty() && regenerateHoerbertXml ){
        recreateXml();
    }

    auto result = m_deviceManager->ejectDrive(currentDevice);

    if (result == SUCCESS)
    {
        updateDriveList();
        deselectDrive();
        QMessageBox::information(this, tr("Eject"), tr("[%1] has been ejected.").arg(currentDevice)+"\n"+tr("It is now safe to remove it from your computer."));
        switchDiagnosticsMode( false );
    }
    else
    {
        QMessageBox::information(this, tr("Eject"), tr("Failed to eject the memory card [%1].").arg(currentDevice)+"\n"+tr("Please try again or try to remove it with your operating system"));
    }

    m_hasFat32WarningShown = false;
}

void CardPage::recreateXml()
{
    QProgressDialog *m_progress = new QProgressDialog(this);

    QFileInfoList file_info_list;
    for (int i = 0; i < 9; i++) {
        QString sub_dir = QString();

        sub_dir = currentDrivePath() + QString::number(i);

        QDir dir(sub_dir);
        if (!dir.exists()) {
            qDebug() << "Sub-directory does not exist - " << i;
            deselectDrive();
            return;
        }
        dir.setFilter(QDir::Files | QDir::NoSymLinks);
        dir.setNameFilters(QStringList() << "*" + DEFAULT_DESTINATION_FORMAT);
        dir.setSorting(QDir::Name);

        file_info_list.append(dir.entryInfoList());
    }

    AudioInfoThread *thread = new AudioInfoThread(file_info_list);
    connect(thread, &AudioInfoThread::processUpdated, this, [m_progress] (int percent) {
        m_progress->setValue(percent);
        m_progress->setLabelText(tr("Getting audio information..."));
        QCoreApplication::processEvents();
    });
    connect(thread, &AudioInfoThread::taskCompleted, this, [this, m_progress] (const AudioList &result) {
        XmlWriter writer(this->currentDrivePath() + HOERBERT_XML, result, QFile::exists(this->currentDrivePath() + DIAGMODE_FILE));
        bool success = writer.create();
        if (success){
            QString bakPath = QString(this->currentDrivePath() + HOERBERT_XML_BACKUP);
            if(QFile::exists( bakPath )) {
                QFile::remove( bakPath );
            }
            if(!QFile::copy( this->currentDrivePath()+HOERBERT_XML, bakPath)){
                qDebug() << "CardPage: Failed creating hoerbert xml backup file!";
            }
        } else {
            qDebug() << "CardPage: Failed creating hoerbert xml file!";
        }

        this->setCursor(Qt::ArrowCursor);
        m_progress->deleteLater();
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    m_progress->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    m_progress->setWindowTitle(tr("Recreating xml"));
    m_progress->setModal(true);
    m_progress->setLabelText(tr("Getting audio information..."));
    m_progress->setRange(0, 100);
    m_progress->setFixedWidth(360);

    QPushButton *abort_button = new QPushButton(m_progress);
    abort_button->setText(tr("Abort"));

    // setting cancel button connects the button to the dialog to hide the dialog on button click
    m_progress->setCancelButton(abort_button);
    // need to disconnect the connections since clicking on the button immediately hides the dialog
    m_progress->disconnect(abort_button);

    // then define custom connections
    connect(abort_button, &QPushButton::clicked, this, [m_progress, abort_button, thread] () {
        m_progress->setLabelText(tr("Aborting..."));
        m_progress->show();
        QCoreApplication::processEvents();
        abort_button->setDisabled(true);

        thread->quit();
        thread->wait();
        thread->deleteLater();
    });

    m_progress->show();
    QCoreApplication::processEvents();

    QEventLoop loop;
    connect(thread, &QThread::finished, &loop, &QEventLoop::quit);

    thread->start();

    loop.exec();

    m_hoerbertXMLIsDirty = false;
}

void CardPage::initializePlaylists()
{
    for (int i = 0; i < 9; i++)
    {
        m_dirs[i]->setCount(0);
        m_dirs[i]->disable();
    }
}

void CardPage::selectDrive()
{
    selectDrive(m_driveList->currentText());
}

void CardPage::deselectDrive()
{
    initializePlaylists();

    m_ejectDriveButton->hide();
    m_ejectButtonLabel->hide();

    m_driveList->setEnabled(true);
    m_selectDriveButton->show();

    emit driveCapacityUpdated(0, 0);
    emit driveSelected(QString());
}

void CardPage::selectDrive(const QString &driveName)
{
    if (driveName.isEmpty())
    {
        deselectDrive();
        return;
    }
    m_ejectDriveButton->show();
    m_ejectButtonLabel->show();

    m_driveList->setEnabled(false);
    m_selectDriveButton->hide();

    m_deviceManager->refresh(driveName);    // refresh the storageInfo object, or else cached info will persist between drive (e.g. memory card) changes

    this->repaint();        // make sure the GUI is repainted. If not, it just looks ugly.
    qApp->processEvents();

    if (m_deviceManager->isWriteProtected(driveName))
    {
        QMessageBox::information(this, tr("Select drive"), tr("The selected device is write-protected. Please eject the device and remove the write protection, if you want to modify any playlists on it."));
        return;
    }

    auto fs = m_deviceManager->getVolumeFileSystem(driveName);
    if (fs != "FAT32" && fs != "msdos" && fs != "vfat")
    {
        if( m_deviceManager->isWorkingOnCustomDirectory() ){
            if( !m_hasFat32WarningShown ){
                // when the user did select a destination path manually, we don't dare to format that drive. Better be safe than sorry.
                QMessageBox::information(this, tr("Select drive"), tr("The selected target path is not FAT32 formatted. It needs to be formatted correctly, or else playback will not work with hörbert.")+"\n\n"+tr("Please make sure that your destination has the correct FAT32 format for hörbert.") );
            }
            m_hasFat32WarningShown = true;
        }
        else
        {
            auto selected = QMessageBox::question(this, tr("Select drive"), tr("The selected memory card is not FAT32 formatted. It needs to be formatted correctly, which will erase everything that is on the card.")+"\n\n"+tr("Do you you want to format this drive [%1] now?").arg(driveName), QMessageBox::Yes|QMessageBox::No, QMessageBox::No );

            if (selected == QMessageBox::No)
            {
                updateDriveList();
                return;
            }

            formatSelectedDrive( true );
            selectDrive("");
            return;
        }
    }

    auto size_in_bytes = m_deviceManager->getVolumeSize(driveName);
    if ( static_cast<int>(size_in_bytes / pow(2, 30)) > VOLUME_SIZE_LIMIT)
    {
        auto selected = QMessageBox::question(this, tr("Select drive"), tr("Volume size is bigger than 32GB. This may not be a memory card at all.")+"\n"+tr("Are you sure you want to work on this drive? [%1]").arg(driveName), QMessageBox::Yes|QMessageBox::No, QMessageBox::No );
        if (selected == QMessageBox::No)
        {
            updateDriveList();
            return;
        }
    }

    bool isPlausible = true;
    std::list <int> plausibilityFixList;

    m_deviceManager->setCurrentDrive(driveName);

    QString drive_path = currentDrivePath();

    QDir dir(drive_path);
    if (dir.exists(DIAGMODE_FILE))
        m_stackWidget->setCurrentIndex(1);
    else
    {
        m_stackWidget->setCurrentIndex(0);

        bool subdir_not_exist = false;

        for (int i = 0; i < 9; i++)
        {
            QString sub_dir = QString();

            sub_dir = drive_path + QString::number(i);

            QDir dir(sub_dir);
            if (!dir.exists()) {
                subdir_not_exist = true;
                break;
            }
        }

        if (subdir_not_exist)
        {
            auto selected = QMessageBox::question(this, tr("Select drive"), tr("This card has never been used with hörbert.")+"\n"+tr("Do you want to make it ready for use with hörbert?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::No );

            if (selected == QMessageBox::No)
            {
                deselectDrive();
                return;
            }

            for (int i = 0; i < 9; i++)
            {
                QDir dir(drive_path);

                if (dir.exists(QString::number(i)))
                    continue;

                if (!dir.mkdir(QString::number(i)))
                {
                    qDebug() << "Failed creating sub-directory" << dir.absolutePath() << i;
                    perror("Creating directory");
                    deselectDrive();
                    return;
                }
            }
        }

        for (int i = 0; i < 9; i++)
        {
            if (m_dirs[i]->isOnProgress())
                continue;

            QString sub_dir = QString();

            sub_dir = drive_path + QString::number(i);

            QDir dir(sub_dir);
            if (!dir.exists()) {
                qDebug() << "Sub-directory does not exist - " << i;
                deselectDrive();
                return;
            }
            dir.setFilter(QDir::Files | QDir::NoSymLinks);
            dir.setNameFilters(QStringList() << "*" + DEFAULT_DESTINATION_FORMAT);
            dir.setSorting(QDir::Name);

            QFileInfoList list = dir.entryInfoList();

            std::sort(list.begin(), list.end(), sortByNumber);
            // check plausibility at this point.
            auto index = 0;
            for (auto item : list) {
                if (item.fileName().toLower().remove(DEFAULT_DESTINATION_FORMAT.toLower()).toInt() != index) {
                    isPlausible = false;
                    qDebug() << list;
                    plausibilityFixList.push_back(i);
                    break;
                }
                index++;
            }

            m_dirs[i]->setText("0");
            m_dirs[i]->setCount(list.size());
            m_dirs[i]->enable();
        }

        //migrate old cards to the new format if there is no hoerbert.bak file on the card.
        if (!m_migrationSuggested && QFile::exists(drive_path + HOERBERT_XML)) {
            if (!QFile::exists(drive_path + HOERBERT_XML_BACKUP) && QFile::exists(drive_path + HOERBERT_XML)  )
            {
                emit migrationNeeded(drive_path);
                m_migrationSuggested = true;
            }
        }
    }

    if (isPlausible)
        qDebug() << "This hoerbert card is plausible.";
    else {
        qDebug() << "This hoerbert card is not plausible.";
        emit plausibilityFixNeeded(plausibilityFixList);
    }

    updateUsedSpace();
    emit driveSelected(driveName);

    this->repaint();        // make sure the GUI is repainted. If not, it just looks ugly.
    qApp->processEvents();
}

void CardPage::selectDriveByPath(const QString &path)
{
    m_deviceManager->addCustomPath( path );
    m_deviceManager->getDeviceList();
    QString driveName = m_deviceManager->getDriveName(path);
    selectDrive(driveName);
    m_driveList->addItems(m_deviceManager->getDeviceList());
    m_driveList->setCurrentText(driveName);
}

void CardPage::update()
{
    selectDrive();
}

void CardPage::updateButtons()
{
    QSettings settings;
    settings.beginGroup("Global");
    int buttonCount = settings.value("buttons").toInt();
    settings.endGroup();

    for (const auto & btn : m_dirs) {
        auto id_= btn->ID();

        switch( buttonCount ){
            case 3:
                btn->setOverlaySize(128, 128);
                btn->setFixedSize(128, 128);
                if ( id_ != 1 && id_ != 6 && id_ != 8)
                    btn->hide();
                else
                    btn->show();

                m_horizontalGridSpacer1->changeSize(0,80);
            break;
            case 1:
                btn->setOverlaySize(128, 128);
                btn->setFixedSize(128, 128);
                if (id_ != 4)
                    btn->hide();
                else
                    btn->show();
            break;
            default:
                btn->setOverlaySize(96, 96);
                btn->setFixedSize(96, 96);
                btn->show();
        }

    }
}

void CardPage::updateUsedSpace()
{
    m_deviceManager->refresh();
    quint64 total_bytes = m_deviceManager->getVolumeSize(m_deviceManager->selectedDriveName());
    quint64 used_bytes = total_bytes - m_deviceManager->getAvailableSize(m_deviceManager->selectedDriveName());
    emit driveCapacityUpdated(used_bytes, total_bytes);
}

void CardPage::setCardManageButtonsEnabled(bool flag)
{
    m_selectDriveButton->setEnabled(flag);
    m_ejectDriveButton->setEnabled(flag);
}

void CardPage::switchDiagnosticsMode(bool enabled)
{
    if( enabled == isDiagnosticsModeEnabled() )
        return;

    m_stackWidget->setCurrentIndex(enabled ? 1 : 0);
    if (!enabled)
        selectDrive();
}

bool CardPage::isDiagnosticsModeEnabled()
{
    return m_stackWidget->currentIndex() == 1;
}

qint64 CardPage::getCurrentVolumeSize()
{
    if (m_driveList->currentText().isEmpty())
        return -1;
    else
        return m_deviceManager->getVolumeSize(m_driveList->currentText());
}

qint64 CardPage::getCurrentAvailableSpace()
{
    if (m_driveList->currentText().isEmpty())
        return -1;
    else
        return m_deviceManager->getAvailableSize(m_driveList->currentText());
}

QString CardPage::getCardFileSystemType()
{
    if (m_driveList->currentText().isEmpty())
        return "Unknown";
    else
        return m_deviceManager->getVolumeFileSystem(m_driveList->currentText());
}

void CardPage::setPercent(int buttonIndex, int percentage)
{
    m_dirs[buttonIndex]->setPercentage(percentage);
    updateUsedSpace();

    m_hoerbertXMLIsDirty = true;
}

void CardPage::setButtonEnabled(int buttonIndex, bool isEnabled)
{
    if (isEnabled)
        m_dirs[buttonIndex]->enable();
    else
        m_dirs[buttonIndex]->disable();
}

QString CardPage::currentDrivePath()
{
    return tailPath(m_deviceManager->getDrivePath());
}

QString CardPage::currentDriveName()
{
    return m_deviceManager->selectedDriveName();
}

void CardPage::setIsProcessing(bool flag)
{
    m_isProcessing = flag;
}

bool CardPage::isProcessing()
{
    for (const auto& dir_button : m_dirs)
    {
        if (dir_button->percentage() != 0)
        {
            return true;
        }
    }

    return false;
}

void CardPage::setHoerbertXMLDirty( bool yesNo ){
    m_hoerbertXMLIsDirty = yesNo;
}


bool CardPage::isHoerbertXMLDirty(){
    return m_hoerbertXMLIsDirty;
}

QColor CardPage::getPlaylistColor(quint8 id)
{
    return m_dirs[id]->backgroundColor();
}

void CardPage::onPlaylistButtonClicked(qint8 dir_num)
{
    if (m_deviceManager->selectedDrive().isEmpty())
        return;

    enableButtons(false);
    enableEditMenuItems( true );

    QString drive_path = currentDrivePath();
    QString sub_dir = QString();

    sub_dir = drive_path + QString::number(dir_num) + QDir::separator();

    QDir dir(sub_dir);

    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setNameFilters(QStringList() << "*" + DEFAULT_DESTINATION_FORMAT);
    dir.setSorting(QDir::Name);

    QFileInfoList list = dir.entryInfoList();
    if (list.isEmpty())
    {
        AudioList empty_list;
        emit playlistChanged(dir_num, m_deviceManager->getDrivePath(), empty_list);
        return;
    }
    std::sort(list.begin(), list.end(), sortByNumber);

    AudioInfoThread *thread = new AudioInfoThread(list);
    connect(thread, &AudioInfoThread::processUpdated, this, [this, dir_num] (int percent) {
        m_dirs[dir_num]->setPercentage(percent);
    });
    connect(thread, &AudioInfoThread::taskCompleted, this, [this, dir_num] (const AudioList &result) {
        m_dirs[dir_num]->setPercentage(100);
        emit playlistChanged(dir_num, m_deviceManager->getDrivePath(), result);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    thread->start();
}


bool CardPage::isDriveWritable( const QString *driveName )
{
    QString deviceName = m_driveList->currentText();
    if( driveName==NULL ){
        deviceName = QString(*driveName);
    }
    if (!deviceName.isEmpty()){
        return m_deviceManager->isWriteProtected(deviceName);
    }

    return false;
}

void CardPage::enableButtons( bool onOff )
{
    //go through list of buttons an enable/disable them
    for (const auto& dir_button : m_dirs)
    {
        dir_button->setEnabled(onOff);
    }
}
