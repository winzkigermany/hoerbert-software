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
#include "functions.h"
extern QString HOERBERT_TEMP_PATH;

CardPage::CardPage(QWidget *parent)
    : QWidget(parent)
{
    m_isFormatting = false;
    m_mainWindow = dynamic_cast<MainWindow*>(parent);

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
    QStringListModel* cbModel = new QStringListModel();
    m_driveList->setModel(cbModel);
    m_driveList->setObjectName("DriveComboBox");
    m_driveList->setEditable(false);
    m_driveList->setToolTip(tr("Shows available memory cards or drives of this computer"));

    m_selectDriveButton = new QPushButton(this);
    m_selectDriveButton->setText(tr("Read Card"));
//    m_selectDriveButton->setToolTip(tr("Read the selected card to edit it")+" ("+QString(tr("Ctrl+r"))+")" );     // not ideal, not adjusted for different OSs
//    m_selectDriveButton->setShortcut(QKeySequence(tr("Ctrl+r")) );                                                // not ideal, not adjusted for different OSs

    m_ejectDriveButton = new PieButton(this);
    m_ejectDriveButton->setFixedSize(32, 32);
    m_ejectDriveButton->setOverlayPixmap(QPixmap(":/images/pie_overlay.png"));
    m_ejectDriveButton->setMainPixmap(QPixmap(":/images/eject.png"));
    m_ejectDriveButton->setShadowEnabled(false);
    m_ejectDriveButton->setToolTip( tr("Save all changes and eject card") );
//    m_ejectDriveButton->setShortcut( QKeySequence(tr("Ctrl+j")) );        // not ideal, not adjusted for different OSs
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

    for( int i=0; i<MAX_PLAYLIST_COUNT; i++){
        m_dirs[i] = new PieButton(m_gridWidget, i);
        m_recordingSelectors[i] = new RecordingSelector(m_gridWidget, i);
        m_recordingSelectors[i]->setEnabled(false);
    }

    m_dirs[0]->setBackgroundColor(CL_DIR0);
    m_dirs[1]->setBackgroundColor(CL_DIR1);
    m_dirs[2]->setBackgroundColor(CL_DIR2);
    m_dirs[3]->setBackgroundColor(CL_DIR3);
    m_dirs[4]->setBackgroundColor(CL_DIR4);
    m_dirs[5]->setBackgroundColor(CL_DIR5);
    m_dirs[6]->setBackgroundColor(CL_DIR6);
    m_dirs[7]->setBackgroundColor(CL_DIR7);
    m_dirs[8]->setBackgroundColor(CL_DIR8);

// not ideal, not adjusted for different OSs
/*    for (const auto& dir_button : m_dirs)
    {
        dir_button->setToolTip(tr("Edit playlist") + QString(" %1 (Ctrl+%1)").arg(dir_button->ID() + 1));
        QShortcut *shortcut = new QShortcut(QKeySequence(QString("Ctrl+%1").arg(dir_button->ID() + 1)), this);
        connect(shortcut, &QShortcut::activated, this, [this, dir_button] () {
            if (dir_button->percentage() > 0)
                return;
            if (!getSelectedDrive().isEmpty())
                this->onPlaylistButtonClicked(dir_button->ID());
        });
    }
*/

    m_horizontalGridSpacer1 = new QSpacerItem(GRID_SPACING, GRID_SPACING, QSizePolicy::Fixed, QSizePolicy::Minimum);
    m_horizontalGridSpacer2 = new QSpacerItem(GRID_SPACING, GRID_SPACING, QSizePolicy::Fixed, QSizePolicy::Minimum);
    m_leftGridSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_rightGridSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_verticalGridSpacer1 = new QSpacerItem(GRID_SPACING, GRID_SPACING, QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_verticalGridSpacer2 = new QSpacerItem(GRID_SPACING, GRID_SPACING, QSizePolicy::Minimum, QSizePolicy::Fixed);

    for( int i=0; i<MAX_PLAYLIST_COUNT; i++){
        m_buttonArea[i] = new QWidget();
        m_hbl[i] = new QHBoxLayout();
        m_hbl[i]->setSpacing(0);
        m_hbl[i]->addWidget(m_dirs[i]);
        m_hbl[i]->addWidget(m_recordingSelectors[i]);
        m_buttonArea[i]->setLayout( m_hbl[i] );
    }

    m_gridLayout->addItem(m_leftGridSpacer, 0, 0, 1, 1);

    m_gridLayout->addWidget( m_buttonArea[0], 0, 1, 1, 1 );
    m_gridLayout->addItem(m_horizontalGridSpacer1, 0, 2, 1, 1);
    m_gridLayout->addWidget( m_buttonArea[1], 0, 3, 1, 1 );
    m_gridLayout->addItem(m_horizontalGridSpacer1, 0, 4, 1, 1);
    m_gridLayout->addWidget( m_buttonArea[2], 0, 5, 1, 1 );
    m_gridLayout->addItem(m_rightGridSpacer, 0, 6, 1, 1);

    m_gridLayout->addItem(m_verticalGridSpacer1, 1, 1, 1, 1);

    m_gridLayout->addWidget( m_buttonArea[3], 2, 1, 1, 1 );
    m_gridLayout->addWidget( m_buttonArea[4], 2, 3, 1, 1 );
    m_gridLayout->addWidget( m_buttonArea[5], 2, 5, 1, 1 );

    m_gridLayout->addItem(m_verticalGridSpacer2, 3, 1, 1, 1);

    m_gridLayout->addWidget( m_buttonArea[6], 4, 1, 1, 1 );
    m_gridLayout->addWidget( m_buttonArea[7], 4, 3, 1, 1 );
    m_gridLayout->addWidget( m_buttonArea[8], 4, 5, 1, 1 );

    for( int i=0; i<MAX_PLAYLIST_COUNT; i++){
        connect( (MainWindow*)parent, &MainWindow::isLatestHoerbert, m_recordingSelectors[i], &RecordingSelector::setVisible);
    }

    m_diagWidget = new QWidget(this);

    m_diagLayout = new QVBoxLayout(m_diagWidget);
    m_diagLayout->setAlignment(Qt::AlignCenter);

    m_diagModeHint = new QLabel(m_diagWidget);
    m_diagModeHint->setObjectName("diagModeHintLabel");
    m_diagModeHint->setStyleSheet("#diagModeHintLabel {color:#353535;}");   // always: dark text on wooden background
    m_diagModeHint->setText(tr("This memory card is in diagnostics mode."));

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

    for( int i=0; i<MAX_PLAYLIST_COUNT; i++){
        connect(m_dirs[i], &QPushButton::clicked, this, [this, i](){
            if (m_dirs[i]->percentage() > 0){
                return;
            }
            if (!getSelectedDrive().isEmpty()){
                onPlaylistButtonClicked(i);
            }
        });
        connect( m_recordingSelectors[i], &RecordingSelector::selectedBluetooth, this, &CardPage::setBluetoothRecordingPlaylist );
        connect( m_recordingSelectors[i], &RecordingSelector::selectedMicrophone, this, &CardPage::setMicrophoneRecordingPermission );
        connect( m_recordingSelectors[i], &RecordingSelector::selectedWifi, this, &CardPage::setWifiRecordingPermission );
        connect( m_recordingSelectors[i], &RecordingSelector::valuesHaveChanged, this, &CardPage::generateIndexM3u );
    }

    connect(m_selectDriveButton, &QPushButton::clicked, this, [this]() {
        selectDrive(m_driveList->currentText());
        if( qApp->property("hoerbertModel")!=2011 && !isDiagnosticsModeEnabled() ){
            convertAllToMp3();

            if( QFile::exists(m_mainWindow->getCurrentDrivePath() + FIRMWARE_VERSION_FILE) ){
                m_mainWindow->checkForFirmwareUpdates(true);
            }
        }
    });
    connect(m_ejectDriveButton, &QPushButton::clicked, this, &CardPage::ejectDrive);

    connect(m_return2Normal, &QPushButton::clicked, this, [this] () {
        emit toggleDiagnosticsMode();
    });

    m_windowsDriveListener = new RemovableDriveListener();
    connect(m_windowsDriveListener, &RemovableDriveListener::drivesHaveChanged, this, &CardPage::updateDriveList, Qt::QueuedConnection);
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
}

void CardPage::updateDriveList()
{
    QString currentlySelectedDriveBeforeUpdatingList = getSelectedDrive();
    m_driveList->blockSignals(true);

    ListString newDriveList = m_deviceManager->getDeviceList();

    for (int i=m_driveList->count(); i>=0; i-- )
    {
        QString currentNewDriveString = "";
        QString comboBoxString = m_driveList->itemText(i);

        bool existsInNewDriveList = false;
        for (const QString& newDrive : newDriveList)
        {
            if (comboBoxString == newDrive)
            {
                existsInNewDriveList = true;
                currentNewDriveString = newDrive;
                break;
            }
        }

        if( !existsInNewDriveList )
        {
            m_driveList->removeItem(i);     // the drive does not exist in the new list
        }
        else
        {
            newDriveList.removeOne( currentNewDriveString );     // the drive already exists in our old list
        }
    }

    // here, the newDriveList only contains drives that are not in our old list.
    if( newDriveList.count()>0 )
    {
        m_driveList->addItems( newDriveList );
    }

    if( !currentlySelectedDriveBeforeUpdatingList.isEmpty() && -1==m_driveList->findText(currentlySelectedDriveBeforeUpdatingList) )
    {
        // the currently selected drive has vanished.
        deselectDrive();

        if( !m_isFormatting )
        {
            QMessageBox::critical( this, tr("The memory card has disappeared"), tr("The memory card has suddenly disappeared.")+"\n\n"+tr("This is only OK if you have ejected it properly with the operating system's eject function.")+"\n\n"+tr("Always eject memory cards using the eject button of this app or the operating system.")+"\n"+tr("Never pull the card from the computer without ejecting it first. Doing so may cause data loss."), QMessageBox::Ok, QMessageBox::Ok );
        }

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
        QMessageBox::warning(this, tr("Format"), tr("You have selected a custom path to work on.\nThis app does not format such paths for safety reasons.\nPlease format your memory card manually."));
        return;
    }

    QString selectedDrive = m_driveList->currentText();
    if( selectedDrive.isEmpty() ){
        return;
    }

    QString nameProposal = selectedDrive.split(" ").at(0);    // get only the first word up to the first whitespace
    nameProposal = nameProposal.replace(QRegExp("[^A-Z^a-z^0-9^_]{1,11}"), "").toUpper();

    if (!retry)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Format"));
        msgBox.setText(tr("THIS OPERATION WILL ERASE ALL DATA!")+"\n"+tr("Are you sure you want to format this drive?\n[%1]").arg(selectedDrive));
        msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setDefaultButton(QMessageBox::No);

        if(msgBox.exec() == QMessageBox::No)
            return;
    }

    QString passwd = "";
#ifdef Q_OS_LINUX
    bool ok = false;
    auto label = tr("If you do not want to enter your root password here,\nplease format the memory card in terminal with the following command:")+"\n\numount [device]\nmkfs.vfat [device]\n";
    passwd = QInputDialog::getText(this, tr("Permission required"), label, QLineEdit::Password, QString(), &ok);
    if (!ok || passwd.isEmpty())
        return;
#endif

    QInputDialog dialog(this);
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setWindowTitle(tr("Format"));
    dialog.setLabelText(tr("Please enter the new name for your card"));
    dialog.setTextValue(nameProposal);

    QLineEdit *lineEdit = dialog.findChild<QLineEdit *>();
    lineEdit->setMaxLength(11);
    lineEdit->setValidator(new QRegExpValidator(QRegExp("[A-Za-z0-9_ ]{1,11}"), lineEdit));
    connect(lineEdit, &QLineEdit::textChanged, this, [=]( QString currentText ){
        lineEdit->setText(currentText.replace(" ", "_").toUpper());       // intentionally replace spaces by underscores while the user is typing.
    });

    if (dialog.exec() == QDialog::Accepted)
    {
        m_isFormatting = true;      // set this to false again when the user selects the next card. NOT earlier than that!
        m_deviceManager->formatDrive(this, selectedDrive, dialog.textValue(), passwd);
    }

    deselectDrive();
    updateDriveList();
}

bool CardPage::ejectDrive()
{
    bool success = true;
    if (m_isProcessing)
    {
        auto selected = QMessageBox::question(this, tr("Eject"), QString(tr("Current drive [%1] is being processed.")+"\n\n"+tr("Are you sure you want to eject the drive?")).arg(getSelectedDrive()), QMessageBox::Yes|QMessageBox::No, QMessageBox::No );

        if (selected == QMessageBox::No)
            return false;
    }

    QString currentDevice = m_driveList->currentText();
    if (currentDevice.isEmpty())
    {
        return false;
    }

    m_pleaseWaitDialog = new PleaseWaitDialog();
    connect( m_pleaseWaitDialog, &QDialog::finished, m_pleaseWaitDialog, &QObject::deleteLater);
    m_pleaseWaitDialog->setParent( this );
    m_pleaseWaitDialog->setWindowFlags(Qt::Window | Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    m_pleaseWaitDialog->setWindowModality(Qt::ApplicationModal);

    QSettings settings;
    settings.beginGroup("Global");
    bool regenerateHoerbertXml = settings.value("regenerateHoerbertXml").toBool();
    settings.endGroup();
    if( (qApp->property("hoerbertModel")==2011) && isHoerbertXMLDirty() && regenerateHoerbertXml ){
        m_pleaseWaitDialog->setWindowTitle(tr("Generating hoerbert.xml"));
        m_pleaseWaitDialog->setWaitMessage(tr("Making this card compatible with the old hoerbert app V1.x"));
        m_pleaseWaitDialog->setProgressRange( 0, 100 );
        connect( this, &CardPage::sendProgressPercent, m_pleaseWaitDialog, &PleaseWaitDialog::setValue );
        m_pleaseWaitDialog->show();
        recreateXml();
    }

    m_pleaseWaitDialog->setProgressRange( 0, 0 );
    m_pleaseWaitDialog->setWindowTitle(tr("Ejecting card"));
    m_pleaseWaitDialog->setWaitMessage(tr("Waiting for all write operations to finish."));
    m_pleaseWaitDialog->show();

    qApp->processEvents();

    // no need to sync here, because the operating system will do that before ejecting the drive.
    // syncing here may even break the ejection process.
    auto result = m_deviceManager->ejectDrive(currentDevice);

    if (result == SUCCESS)
    {
        m_pleaseWaitDialog->setWindowTitle(tr("Finished"));
        deselectDrive();

        bool doGenerateHoerbertXml = false;
        {
            QSettings settings;
            settings.beginGroup("Global");
            doGenerateHoerbertXml = settings.value("regenerateHoerbertXml").toBool();
            settings.endGroup();
        }

        if(doGenerateHoerbertXml)
        {
            if( qApp->property("hoerbertModel")=="2011" ){
                m_pleaseWaitDialog->setResultString(tr("[%1] has been ejected.").arg(currentDevice)+"\n"+tr("It is now safe to remove it from your computer.")+"\n\n"+tr("If you do not need hoerbert.xml for the old hoerbert app 1.x,\nskip this step by ticking the check box below."));
                connect( m_pleaseWaitDialog, &PleaseWaitDialog::checkboxIsClicked, this, [=](bool onOff){
                    QSettings settings;
                    settings.beginGroup("Global");
                    settings.setValue("regenerateHoerbertXml", !onOff);
                    settings.endGroup();
                });
                m_pleaseWaitDialog->setCheckBoxLabel(tr("I only will use my memory cards with this new software from now on."));
            } else {
                m_pleaseWaitDialog->setResultString(tr("[%1] has been ejected.").arg(currentDevice)+"\n"+tr("It is now safe to remove it from your computer."));
            }
        }
        else
        {
            m_pleaseWaitDialog->setResultString(tr("[%1] has been ejected.").arg(currentDevice)+"\n"+tr("It is now safe to remove it from your computer."));
        }
        switchDiagnosticsMode( false );
        success = true;
    }
    else
    {
        m_pleaseWaitDialog->setResultString( tr("Failed to eject the memory card [%1].").arg(currentDevice)+"\n"+tr("Please try again or try to remove it with your operating system") );
        success = false;
    }

    updateDriveList();
    m_hasFat32WarningShown = false;

    return success;
}

void CardPage::recreateXml()
{
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
        if( qApp->property("hoerbertModel")==2011 ){
            dir.setNameFilters(QStringList() << "*" + DESTINATION_FORMAT_WAV);
        } else {
            dir.setNameFilters(QStringList() << "*" + DESTINATION_FORMAT_MP3);
        }
        dir.setSorting(QDir::Name);

        file_info_list.append(dir.entryInfoList());
    }

    AudioInfoThread *thread = new AudioInfoThread(file_info_list);
    connect(thread, &AudioInfoThread::processUpdated, this, &CardPage::sendProgressPercent);

    connect(thread, &AudioInfoThread::taskCompleted, this, [=] (const AudioList &result) {
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
            qDebug() << "CardPage: Failed creating hoerbert.xml file!";
        }
    });

    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

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
        m_playlistSize[i] = 0;
        m_playlistEstimatedSize[i] = 0;
        m_usedSpaceOffset = 0;
    }
}


void CardPage::clearRecordingSettings(){
    m_bluetoothRecordingPlaylist = 255;
    for( int i=0; i<MAX_PLAYLIST_COUNT; i++ ){
        m_microphoneRecordingPermissions[i] = false;
        m_wifiRecordingPermissions[i] = false;
    }
    updateRecordingSettings();
}


void CardPage::deselectDrive()
{
    m_deviceManager->setCurrentDrive("");
    if( isWorkingOnCustomDirectory() )  // we need to throw out our manually selected directory
    {
        updateDriveList();
    }

    clearRecordingSettings();
    for( int i=0; i<MAX_PLAYLIST_COUNT; i++){
        m_recordingSelectors[i]->setEnabled(false);
    }

    initializePlaylists();

    setHoerbertXMLDirty( false );

    m_ejectDriveButton->hide();
    m_ejectButtonLabel->hide();

    m_driveList->setEnabled(true);
    m_selectDriveButton->show();

    emit driveCapacityUpdated(0, 0);
    emit driveSelected("");
}



/**
* @brief CardPage::hasAudioFiles
* look for all audio files EXCEPT mp3 files. (We don't need to re-encode mp3 files)
*/
bool CardPage::hasAudioFiles( QString rootPath){
    QStringList nameFilter;
    nameFilter << "*" + DESTINATION_FORMAT_WAV << "*" + DESTINATION_FORMAT_FLAC << "*" + DESTINATION_FORMAT_AAC << "*" + DESTINATION_FORMAT_M4A << "*" + DESTINATION_FORMAT_MP4 << "*" + DESTINATION_FORMAT_OGG;

    bool foundOne = false;
    for( int i=0; i<MAX_PLAYLIST_COUNT; i++){
        qDebug() << "Searching for audio files in " << rootPath+QString::number(i);
        QDir directory(rootPath+QString::number(i));
        QStringList txtFilesAndDirectories = directory.entryList(nameFilter, QDir::AllEntries|QDir::NoDotAndDotDot);
        for ( const auto& entry : txtFilesAndDirectories  )
        {
            qDebug() << entry;
        }
        if( txtFilesAndDirectories.length()>0 ){
            foundOne = true;
            break;
        }
    }

    return foundOne;
}



void CardPage::convertAllAudioFilesToMp3( QString rootPath){
    QStringList nameFilter;
    nameFilter << "*" + DESTINATION_FORMAT_WAV << "*" + DESTINATION_FORMAT_FLAC << "*" + DESTINATION_FORMAT_AAC << "*" + DESTINATION_FORMAT_M4A << "*" + DESTINATION_FORMAT_MP4 << "*" + DESTINATION_FORMAT_OGG;

    for( int i=0; i<MAX_PLAYLIST_COUNT; i++){
        qDebug() << "Searching for audio files in " << rootPath+QString::number(i);
        QDir directory(rootPath+QString::number(i));
        QStringList txtFilesAndDirectories = directory.entryList(nameFilter, QDir::AllEntries|QDir::NoDotAndDotDot);
        int indexCount = 0;
        for ( const auto& iterator : txtFilesAndDirectories  )
        {
            HoerbertProcessor *processor = new HoerbertProcessor(rootPath+QString::number(i)+"/"+iterator, -1);
            QFileInfo audioFile( rootPath+QString::number(i) +"/"+ iterator);
            qDebug() << "Processing file " << audioFile.absoluteFilePath();
            QString mp3FileName;

            QDir checkDir(rootPath+QString::number(i));
            QStringList sameIndexFilter;
            sameIndexFilter << audioFile.baseName()+".*";
            QStringList numberOfSameIndex = checkDir.entryList(sameIndexFilter, QDir::Files | QDir::NoSymLinks);
            qDebug() << "file info list: " << numberOfSameIndex;

            if( numberOfSameIndex.length()>1 ){     // more files with the same index number exist -> append this file to the end of the playlist.
                uint newNumber = getHighestNumberInDirectory(rootPath+QString::number(i)) + 1;
                mp3FileName = audioFile.path() + "/" + QString().number(newNumber) + ".mp3";
            } else {
                mp3FileName = audioFile.path() + "/" + audioFile.baseName() + ".mp3";
            }

            emit convertingCurrentFile(audioFile.absoluteFilePath());

            processor->convertToMp3( audioFile.absoluteFilePath(), mp3FileName, true);
            indexCount++;
        }
    }
}


/**
 * @brief CardPage::cleanupDoubleFiles
 * @param driveName
 * clean up. Important: Don't leave url files and other files with the same index.
 */
void CardPage::cleanupDoubleFiles(const QString &rootPath){
    QStringList nameFilter;
    nameFilter << "*.*";

    for( int i=0; i<MAX_PLAYLIST_COUNT; i++){
        qDebug() << "Searching for double files in " << rootPath+QString::number(i);
        QDir directory(rootPath+QString::number(i));
        directory.setSorting(QDir::Name);

        QFileInfoList allFileNames = directory.entryInfoList(nameFilter, QDir::Files | QDir::NoSymLinks);
        for ( const auto& iterator : allFileNames  )
        {
            directory.refresh();
            QStringList sameIndexFilter;
            sameIndexFilter << iterator.baseName()+".*";

            QStringList filesOfSameIndex = directory.entryList(sameIndexFilter, QDir::Files | QDir::NoSymLinks);

            if( filesOfSameIndex.length()>1 ){
                for( int j=0; j<allFileNames.length(); j++){
                    if( allFileNames[j].baseName() == iterator.baseName()){
                        uint num = getHighestNumberInDirectory(iterator.absolutePath()) + 1;
                        moveFile( iterator.absoluteFilePath(), iterator.absolutePath() + "/" + QString().number(num) + "." + iterator.suffix() );
                    }
                }
            }
        }
    }
}


bool CardPage::cardContainsMp3FilesAlready( QString &drive_path ){

    for( int i=0; i<MAX_PLAYLIST_COUNT; i++){
        QDir dir(drive_path+QString().number(i));
        dir.setFilter(QDir::Files | QDir::NoSymLinks);
        dir.setNameFilters(QStringList() << "*" + DESTINATION_FORMAT_MP3);

        QFileInfoList list = dir.entryInfoList();
        qDebug() << "fileList: " << list;
        if( list.length()>0 ){
            return true;
        }
    }

    return false;
}


void CardPage::convertAllToMp3(){

    QString drive_path = currentDrivePath();

    if( hasAudioFiles(drive_path) ){
        bool backupFirst = false;
        QString backupString = "";
        QString conversionString = "";
        if( !cardContainsMp3FilesAlready(drive_path) ){
            backupFirst = true;
            backupString = "\n\n"+tr("In the next step, you will need to select a folder on your computer for a backup of this card.");
            conversionString = tr("After converting files, this card will NOT WORK in the older hörbert model 2011")+"\n\n";
        }

        auto selected = QMessageBox::question(this, tr("Convert files"), tr("Audio files need to be converted for this hörbert. This gets you more space on the card.")+"\n\n"+conversionString+tr("Do you want this?")+backupString, QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes );
        if (selected == QMessageBox::Yes)
        {

            if( backupFirst ){
                qDebug()<< "Force a backup";
                m_mainWindow->backupCard();
            }

            m_pleaseWaitDialog = new PleaseWaitDialog();
            connect( m_pleaseWaitDialog, &QDialog::finished, m_pleaseWaitDialog, &PleaseWaitDialog::close);
            m_pleaseWaitDialog->setParent( this );
            m_pleaseWaitDialog->setWindowFlags(Qt::Window | Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
            m_pleaseWaitDialog->setWindowModality(Qt::ApplicationModal);
            m_pleaseWaitDialog->show();

            connect( this, &CardPage::convertingCurrentFile, m_pleaseWaitDialog, &PleaseWaitDialog::setWaitMessage);

            cleanupDoubleFiles(drive_path);
            for( int i=0; i<MAX_PLAYLIST_COUNT; i++){
                renumberDirectory(drive_path+QString().number(i)+"/");
            }

            convertAllAudioFilesToMp3(drive_path);

            m_pleaseWaitDialog->setResultString(tr("Finished converting all files."));

            // re-read the card and fill the playlist counters
            update();
        } else {
            selectDrive("");
            updateDriveList();
            return;
        }
    }}


void CardPage::selectDrive(const QString &driveName, bool doUpdateCapacityBar)
{
    if (driveName.isEmpty())
    {
        deselectDrive();
        return;
    }

    clearRecordingSettings();

    m_deviceManager->refresh(driveName);    // refresh the storageInfo object, or else cached info will persist between drive (e.g. memory card) changes
/*
    if ( m_deviceManager->isWriteProtected(driveName))
    {
        QMessageBox::information(this, tr("Select drive"), tr("The selected device is write-protected. Please remove the write protection if you want to modify any playlists on it."));
        deselectDrive();
        return;
    }
*/
    m_isFormatting = false;

    m_ejectDriveButton->show();
    m_ejectButtonLabel->show();

    m_driveList->setEnabled(false);
    m_selectDriveButton->hide();

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

    qint64 size_in_bytes = m_deviceManager->getVolumeSize(driveName);
    if ( size_in_bytes > (qint64(VOLUME_SIZE_LIMIT)*qint64(1073741824)+1024) )
    {
        auto selected = QMessageBox::question(this, tr("Select drive"), tr("Volume size is bigger than 64GB. This may not be a memory card at all.")+"\n"+tr("Are you sure you want to work on this drive? [%1]").arg(driveName), QMessageBox::Yes|QMessageBox::No, QMessageBox::No );
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

            QDir inner(sub_dir);
            if (!inner.exists()) {
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
                QDir innerDir(drive_path);

                if (innerDir.exists(QString::number(i)))
                    continue;

                if (!innerDir.mkdir(QString::number(i)))
                {
                    qDebug() << "Failed creating sub-directory" << innerDir.absolutePath() << i;
                    perror("Creating directory");

                    QString proposedSolution = "";
                    if( m_deviceManager->isWorkingOnCustomDirectory() ){
                        proposedSolution = tr("Please select a different destination folder.");
                    } else {
                        proposedSolution = tr("Please select a different destination drive.");
                    }

                    deselectDrive();
                    QMessageBox::information(this, tr("Can't write"), tr("This app can't write files to the selected destination.")+"\n"+proposedSolution, QMessageBox::Ok );
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

            QDir inner(sub_dir);
            if (!inner.exists()) {
                qDebug() << "Sub-directory does not exist - " << i;
                deselectDrive();
                return;
            }
            inner.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
            if( qApp->property("hoerbertModel")==2011 ){
                inner.setNameFilters(QStringList() << "*" + DESTINATION_FORMAT_WAV);
            } else {
                inner.setNameFilters(QStringList() << "*" + DESTINATION_FORMAT_MP3 << "*" + DESTINATION_FORMAT_URL);
            }
            inner.setSorting(QDir::Name);

            QFileInfoList list = inner.entryInfoList();

            std::sort(list.begin(), list.end(), sortByNumber);
            // check plausibility at this point.
            auto index = 0;
            for (QFileInfo currentFile : list) {
                if( qApp->property("hoerbertModel")==2011 ){
                    if ( currentFile.fileName().toLower().remove(DESTINATION_FORMAT_WAV.toLower()).toInt() != index
                         || currentFile.size()<45 ) {    // the header of a wav file is at least 44 bytes long
                        isPlausible = false;
                        qDebug() << list;
                        plausibilityFixList.push_back(i);
                        break;
                    }
                } else {
                    //@TODO: Plausibility is a difficult concept for multi-file, multi-suffix scenarios.
                    //in the future, we might be able to check plausibility against an m3u file again.
                    //For now, the chances for false positives are simply too high for a sensible plausibility check.

                    //Probably, we could at least make sure that there is only one file suffix per file index. Except m3u files, of course...

                    ;
                }

                index++;
            }

            m_dirs[i]->setText("0");
            m_dirs[i]->setCount(list.size());
            m_dirs[i]->enable();

            m_recordingSelectors[i]->setEnabled(true);
        }

        readIndexM3u();

        //migrate old cards to the new format if there is no hoerbert.bak file on the card.
        if( qApp->property("hoerbertModel")==2011 ){
            if (!m_migrationSuggested && QFile::exists(drive_path + HOERBERT_XML)) {
                if (!QFile::exists(drive_path + HOERBERT_XML_BACKUP) && QFile::exists(drive_path + HOERBERT_XML)  )
                {
                    emit migrationNeeded(drive_path);
                    m_migrationSuggested = true;
                }
            }
        }
    }

    if (isPlausible)
        qDebug() << "This hoerbert card is plausible.";
    else {
        qDebug() << "This hoerbert card is not plausible.";
        emit plausibilityFixNeeded(plausibilityFixList);
    }

    if( doUpdateCapacityBar )
    {
        initUsedSpace();
    }
    emit driveSelected(driveName);
}

void CardPage::selectDriveByPath(const QString &path)
{
    m_deviceManager->addCustomPath( path );
    m_deviceManager->getDeviceList();

    QString driveName = m_deviceManager->getDriveName(path);
    selectDrive(driveName, false);
    if( qApp->property("hoerbertModel")!=2011 ){
        convertAllToMp3();
    }

    m_driveList->addItems(m_deviceManager->getDeviceList());
    m_driveList->setCurrentText(driveName);

    initUsedSpace();
    setCardManageButtonsEnabled(true);
}

void CardPage::update()
{
    selectDrive(m_driveList->currentText(), false);
}

void CardPage::updateButtons()
{
    QSettings settings;
    settings.beginGroup("Global");
    int buttonCount = settings.value("buttons").toInt();
    settings.endGroup();

    for( int i=0; i<MAX_PLAYLIST_COUNT; i++){
        switch( buttonCount ){
            case 3:
                m_dirs[i]->setOverlaySize(128, 128);
                m_dirs[i]->setFixedSize(128, 128);
                if ( i!=1 && i!=6 && i!=8 ){
                    m_buttonArea[i]->hide();
                } else {
                    m_buttonArea[i]->show();
                }

                m_horizontalGridSpacer1->changeSize(0,80);
            break;
            case 1:
                m_dirs[i]->setOverlaySize(128, 128);
                m_dirs[i]->setFixedSize(128, 128);
                if ( i != 4 ){
                    m_buttonArea[i]->hide();
                } else {
                    m_buttonArea[i]->show();
                }

            break;
            default:
                m_dirs[i]->setOverlaySize(96, 96);
                m_dirs[i]->setFixedSize(96, 96);
                m_buttonArea[i]->show();
            }
    }
}

void CardPage::initUsedSpace()
{
    QString selectedDrive = getSelectedDrive();
    m_deviceManager->refresh(selectedDrive);
    m_total_bytes = m_deviceManager->getVolumeSize(selectedDrive);

    quint64 availableBytes = m_deviceManager->getAvailableSize(selectedDrive);
    quint64 used_bytes = 0;
    if( availableBytes>m_total_bytes )
    {
        used_bytes=0;
    }
    else
    {
        used_bytes=m_total_bytes-availableBytes;
    }
    QString playlistFolder = "";

    quint64 playlists_used_bytes = 0;
    for( int i=0; i<9; i++ )
    {
        playlistFolder = currentDrivePath()+QString::number(i);     // yes, currentDrivePath() ends with a slash "/"

        m_playlistEstimatedSize[i] = 0;
        m_playlistSize[i] = m_deviceManager->getPlaylistSize(playlistFolder);

        playlists_used_bytes += m_playlistSize[i];
    }

    m_usedSpaceOffset = used_bytes - playlists_used_bytes;

    emit driveCapacityUpdated(used_bytes, m_total_bytes);
}


void CardPage::commitUsedSpace( int playlistIndex )
{
    if( playlistIndex<0 || playlistIndex>8 )
    {
        return;
    }

    QString playlistFolder = currentDrivePath()+"/"+QString::number(playlistIndex);

    // commit the used space
    m_playlistSize[playlistIndex] = m_deviceManager->getPlaylistSize(playlistFolder);
    m_playlistEstimatedSize[playlistIndex] = 0;

    sendDriveCapacity();
}


void CardPage::sendDriveCapacity()
{
    quint64 playlists_used_bytes = 0;

    for( int i=0; i<9; i++ )
    {
        playlists_used_bytes += m_playlistSize[i]+m_playlistEstimatedSize[i];
    }


    emit driveCapacityUpdated(playlists_used_bytes+m_usedSpaceOffset, m_total_bytes);
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

void CardPage::sendPercent(int buttonIndex, int percentage)
{
    m_dirs[buttonIndex]->setPercentage(percentage);
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
    return tailPath(m_deviceManager->getDrivePath(getSelectedDrive()));
}

QString CardPage::currentDriveName()
{
    return getSelectedDrive();  // @TODO: only return the drive name??
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
    if (getSelectedDrive().isEmpty())
        return;

    enableButtons(false);
    enableEditMenuItems( true );

    QString drive_path = currentDrivePath();
    QString sub_dir = QString();

    sub_dir = drive_path + QString::number(dir_num) + QDir::separator();

    QDir dir(sub_dir);

    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    if( qApp->property("hoerbertModel")==2011 ){
        dir.setNameFilters(QStringList() << "*" + DESTINATION_FORMAT_WAV);
    } else {
        dir.setNameFilters(QStringList() << "*" + DESTINATION_FORMAT_MP3 << "*" + DESTINATION_FORMAT_URL);
    }
    dir.setSorting(QDir::Name);

    QFileInfoList list = dir.entryInfoList();
    if (list.isEmpty())
    {
        AudioList empty_list;
        emit playlistChanged(dir_num, m_deviceManager->getDrivePath(getSelectedDrive()), empty_list);
        return;
    }
    std::sort(list.begin(), list.end(), sortByNumber);

    AudioInfoThread *thread = new AudioInfoThread(list);
    connect(thread, &AudioInfoThread::processUpdated, this, [this, dir_num] (int percent) {
        m_dirs[dir_num]->setPercentage(percent);
    });
    connect(thread, &AudioInfoThread::taskCompleted, this, [this, dir_num] (const AudioList &result) {
        m_dirs[dir_num]->setPercentage(100);
        emit playlistChanged(dir_num, m_deviceManager->getDrivePath(getSelectedDrive()), result);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    thread->start();
}


void CardPage::enableButtons( bool onOff )
{
    //go through list of buttons an enable/disable them
    for (const auto& dir_button : m_dirs)
    {
        dir_button->setEnabled(onOff);
    }
}

int CardPage::getDriveListLength()
{
    return m_driveList->count();
}

void CardPage::updateEstimatedDuration( int playlistIndex, quint64 seconds )
{
    if( playlistIndex>-1 && playlistIndex<9 )
    {
        m_playlistEstimatedSize[playlistIndex] += secondsToBytes(seconds);
    }

    sendDriveCapacity();
}

int CardPage::numberOfTracks()
{
    int trackCount = 0;

    for( int i=0; i<9; i++ )
    {
       trackCount += m_dirs[i]->getCount();
    }

    return trackCount;
}

bool CardPage::isWorkingOnCustomDirectory()
{
    return m_deviceManager->isWorkingOnCustomDirectory();
}

QString CardPage::getSelectedDrive()    // this IS the drive that the user wants to use and is currently working on.
{
    if( !m_driveList->isEnabled() )
    {
        QString driveName = m_driveList->currentText().section(" ",0).trimmed();
        return driveName;
    }

    return "";
}

QString CardPage::getDisplayedDrive()
{
    return m_driveList->currentText();  // this is NOT neccessarily the drive that the user wants to use!! (see getSelectedDrive() for that)
}




quint8 CardPage::getBluetoothRecordingPlaylist(){
    return m_bluetoothRecordingPlaylist;
}

bool CardPage::isWifiRecordingAllowedInPlaylist( quint8 playlistIndex ){
    if( playlistIndex<MAX_PLAYLIST_COUNT ){
        return m_wifiRecordingPermissions[playlistIndex];
    }
    return false;
}

bool CardPage::isMicrophoneRecordingAllowedInPlaylist( quint8 playlistIndex ){
    if( playlistIndex<MAX_PLAYLIST_COUNT ){
        return m_microphoneRecordingPermissions[playlistIndex];
    }
    return false;
}

void CardPage::setBluetoothRecordingPlaylist( quint8 playlistIndex, bool onOff ){
    qDebug() << "Setting bluetooth recording playlist: " << playlistIndex << ", value: " << onOff;

    if( onOff ){
        m_bluetoothRecordingPlaylist = playlistIndex;
    } else {
        m_bluetoothRecordingPlaylist = 255;
    }
    updateRecordingSettings();
}

void CardPage::setMicrophoneRecordingPermission( quint8 playlistIndex, bool onOff ){
    qDebug() << "Setting microphone recording permission: " << playlistIndex << ", value: " << onOff;

    if( playlistIndex<MAX_PLAYLIST_COUNT ){
        m_microphoneRecordingPermissions[playlistIndex] = onOff;
    }
    updateRecordingSettings();
}

void CardPage::setWifiRecordingPermission( quint8 playlistIndex, bool onOff ){
    qDebug() << "Setting wifi recording permission: " << playlistIndex << ", value: " << onOff;

    if( playlistIndex<MAX_PLAYLIST_COUNT ){
        m_wifiRecordingPermissions[playlistIndex] = onOff;
    }
    updateRecordingSettings();
}

void CardPage::updateRecordingSettings(){

    for( int i=0; i<MAX_PLAYLIST_COUNT; i++ ){
        if( i==m_bluetoothRecordingPlaylist ){
            m_recordingSelectors[i]->setBluetooth(true);
        } else {
            m_recordingSelectors[i]->setBluetooth(false);
        }
    }

    for( int i=0; i<MAX_PLAYLIST_COUNT; i++ ){
        m_recordingSelectors[i]->setMicrophone( m_microphoneRecordingPermissions[i] );
    }

    for( int i=0; i<MAX_PLAYLIST_COUNT; i++ ){
        m_recordingSelectors[i]->setWifi( m_wifiRecordingPermissions[i] );
    }
}


/**
 * @brief CardPage::readIndexM3U read the index.m3u file
 * @param rootPath
 */
void CardPage::readIndexM3u(){

    QString rootPath = m_mainWindow->getCurrentDrivePath();
    qDebug() << "root path for index.m3u: " << rootPath + INDEX_M3U_FILE;

    // reset all values
    m_bluetoothRecordingPlaylist = 255;
    for( int i=0; i<MAX_PLAYLIST_COUNT; i++){
        m_microphoneRecordingPermissions[i] = false;
        m_wifiRecordingPermissions[i] = false;
    }

    // Open file to read contents
    QFile file(rootPath + INDEX_M3U_FILE);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);

        while (!stream.atEnd())
        {
            QString newLine = stream.readLine();
            QString dataString = "";
            QString searchString;

            searchString = "#hoerbert:set_bluetooth_recordings_playlist";

            if( newLine.toLower().startsWith(searchString) ){
                dataString = newLine.toLower().right( newLine.length()-searchString.length() ).trimmed();

                if( dataString.trimmed()=="0.0"){
                    m_bluetoothRecordingPlaylist = 0;
                }

                if( dataString.trimmed()=="0.1"){
                    m_bluetoothRecordingPlaylist = 1;
                }

                if( dataString.trimmed()=="0.2"){
                    m_bluetoothRecordingPlaylist = 2;
                }

                if( dataString.trimmed()=="0.3"){
                    m_bluetoothRecordingPlaylist = 3;
                }

                if( dataString.trimmed()=="0.4"){
                    m_bluetoothRecordingPlaylist = 4;
                }

                if( dataString.trimmed()=="0.5"){
                    m_bluetoothRecordingPlaylist = 5;
                }

                if( dataString.trimmed()=="0.6"){
                    m_bluetoothRecordingPlaylist = 6;
                }

                if( dataString.trimmed()=="0.7"){
                    m_bluetoothRecordingPlaylist = 7;
                }

                if( dataString.trimmed()=="0.8"){
                    m_bluetoothRecordingPlaylist = 8;
                }
            }

            searchString = "#hoerbert:allow_microphone_recordings_in_playlist";
            if( newLine.toLower().startsWith(searchString) ){
                dataString = newLine.toLower().right( newLine.length()-searchString.length() ).trimmed();
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 2)
                QStringList list = dataString.split(" ", QString::SkipEmptyParts);
#else
                QStringList list = dataString.split(" ", Qt::SkipEmptyParts);
#endif
                for ( const auto& i : list  )
                {
                    if( i.trimmed()=="0.0"){
                        m_microphoneRecordingPermissions[0] = true;
                    }

                    if( i.trimmed()=="0.1"){
                        m_microphoneRecordingPermissions[1] = true;
                    }

                    if( i.trimmed()=="0.2"){
                        m_microphoneRecordingPermissions[2] = true;
                    }

                    if( i.trimmed()=="0.3"){
                        m_microphoneRecordingPermissions[3] = true;
                    }

                    if( i.trimmed()=="0.4"){
                        m_microphoneRecordingPermissions[4] = true;
                    }

                    if( i.trimmed()=="0.5"){
                        m_microphoneRecordingPermissions[5] = true;
                    }

                    if( i.trimmed()=="0.6"){
                        m_microphoneRecordingPermissions[6] = true;
                    }

                    if( i.trimmed()=="0.7"){
                        m_microphoneRecordingPermissions[7] = true;
                    }

                    if( i.trimmed()=="0.8"){
                        m_microphoneRecordingPermissions[8] = true;
                    }
                }
            }

            searchString = "#hoerbert:allow_wifi_recordings_in_playlist";
            if( newLine.toLower().startsWith(searchString) ){
                dataString = newLine.toLower().right( newLine.length()-searchString.length() ).trimmed();
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 2)
                QStringList list = dataString.split(" ", QString::SkipEmptyParts);
#else
                QStringList list = dataString.split(" ", Qt::SkipEmptyParts);
#endif
                for ( const auto& i : list  )
                {
                    if( i.trimmed()=="0.0"){
                        m_wifiRecordingPermissions[0] = true;
                    }

                    if( i.trimmed()=="0.1"){
                        m_wifiRecordingPermissions[1] = true;
                    }

                    if( i.trimmed()=="0.2"){
                        m_wifiRecordingPermissions[2] = true;
                    }

                    if( i.trimmed()=="0.3"){
                        m_wifiRecordingPermissions[3] = true;
                    }

                    if( i.trimmed()=="0.4"){
                        m_wifiRecordingPermissions[4] = true;
                    }

                    if( i.trimmed()=="0.5"){
                        m_wifiRecordingPermissions[5] = true;
                    }

                    if( i.trimmed()=="0.6"){
                        m_wifiRecordingPermissions[6] = true;
                    }

                    if( i.trimmed()=="0.7"){
                        m_wifiRecordingPermissions[7] = true;
                    }

                    if( i.trimmed()=="0.8"){
                        m_wifiRecordingPermissions[8] = true;
                    }

                }
            }
        }

        file.close();
    }
    updateRecordingSettings();
}


/**
 * @brief CardPage::generateIndexM3u
 * @param rootPath
 */
void CardPage::generateIndexM3u(){

    QString rootPath = m_mainWindow->getCurrentDrivePath();
    qDebug() << "Generating index.m3u. Root path for index.m3u: " << rootPath + INDEX_M3U_FILE;

    // Open file to copy contents
    QFile file(rootPath + INDEX_M3U_FILE);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        // create the file to keep the rest working as if it existed.
        file.open(QIODevice::ReadWrite | QIODevice::Text);
    }
    file.close();

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        // Open new file to write
        uint32_t lineNumber = 0;
        QFile temp( tailPath(HOERBERT_TEMP_PATH) + INDEX_M3U_FILE_BAK );
        if( temp.open(QIODevice::ReadWrite | QIODevice::Text) )
        {
            QTextStream stream(&file);
            QTextStream out(&temp);
            bool foundOne = false;

            out << "#EXTM3U" << "\n";

            // there can be only one single bluetooth recording playlist
            for( int i=0; i<MAX_PLAYLIST_COUNT; i++ ){
                if( true==m_recordingSelectors[i]->getBluetooth() ){
                    out << "#hoerbert:set_bluetooth_recordings_playlist" << " 0." << QString::number(i) << "\n";
                    break;
                }
            }


            QString microphonePermissionsString = "";
            foundOne = false;
            for( int i=0; i<MAX_PLAYLIST_COUNT; i++ ){
                if( true==m_recordingSelectors[i]->getMicrophone() ){
                    microphonePermissionsString += " 0."+QString::number(i);
                    foundOne = true;
                }
            }
            if( foundOne ){
                out << "#hoerbert:allow_microphone_recordings_in_playlist" << microphonePermissionsString << "\n";
            }

            QString wifiPermissionsString = "";
            foundOne = false;
            for( int i=0; i<MAX_PLAYLIST_COUNT; i++ ){
                if( true==m_recordingSelectors[i]->getWifi()  ){
                    wifiPermissionsString += " 0."+QString::number(i);
                    foundOne = true;
                }
            }
            if( foundOne ){
                out << "#hoerbert:allow_wifi_recordings_in_playlist" << wifiPermissionsString << "\n";
            }

            while (!stream.atEnd())
            {
                QString newLine = stream.readLine();

                if( newLine.toLower().startsWith("#extm3u") ){
                    lineNumber++;
                    continue;
                }
                if( newLine.toLower().startsWith("#hoerbert:set_bluetooth_recordings_playlist") ){
                    lineNumber++;
                    continue;
                }
                if( newLine.toLower().startsWith("#hoerbert:allow_microphone_recordings_in_playlist") ){
                    lineNumber++;
                    continue;
                }
                if( newLine.toLower().startsWith("#hoerbert:allow_wifi_recordings_in_playlist") ){
                    lineNumber++;
                    continue;
                }

                out << newLine  << "\n";
                lineNumber++;
            }
            temp.close();
            file.close();

            file.remove();
            if( !temp.rename(rootPath + INDEX_M3U_FILE) ){
                m_mainWindow->processorErrorOccurred( tr("Unable to write index.m3u to the memory card. Reason: %1").arg(temp.errorString()) );
            }
         } else {
            m_mainWindow->processorErrorOccurred( tr("Unable to create index.m3u in a temporary directory. Reason: %1").arg(temp.errorString()) );
        }
    }
}


/**
 * @brief try to read ver_fw.txt from the memory card and extract the firmware version from it.
 * @return
 */
QString CardPage::getHoerbertFirmwareString(){

    QString extractedVersionString = "";

    if (!getSelectedDrive().isEmpty()){
        QString firmwareVersionFile = m_mainWindow->getCurrentDrivePath() + FIRMWARE_VERSION_FILE;

        if( QFile(firmwareVersionFile).exists() ){
            QFile inputFile(firmwareVersionFile);
            if (inputFile.open(QIODevice::ReadOnly))
            {
                QTextStream in(&inputFile);
                while (!in.atEnd())
                {
                    QString line = in.readLine();
                    if( line.startsWith("App version:") ){
                        extractedVersionString = line.mid(13).trimmed();
                    }
                }
                inputFile.close();
            }
        }
    }

    return extractedVersionString;
}


/**
 * @brief backup fw_ver.txt on the memory card if it is there.
 */
void CardPage::removeFirmwareInfoFile(){

    QString firmwareVersionFileName = m_mainWindow->getCurrentDrivePath()+FIRMWARE_VERSION_FILE;
    QString firmwareVersionBakFileName = firmwareVersionFileName+".bak";

    if (!getSelectedDrive().isEmpty() && QFile::exists(firmwareVersionFileName) ){

        if( QFile::exists( firmwareVersionBakFileName ) ){
            QFile::remove( firmwareVersionBakFileName );
        }
        QFile::rename( firmwareVersionFileName, firmwareVersionBakFileName );
    }
}
