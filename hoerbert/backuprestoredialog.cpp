#include "backuprestoredialog.h"

#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QDebug>

BackupRestoreDialog::BackupRestoreDialog(QDialog *parent) : QDialog(parent)
{
    setWindowTitle(tr("Restore backup"));
    setFixedSize(600, 396);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(25, 20, 25, 25);

    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    m_layout->addWidget(m_label);

    m_buttonGroup = new QGroupBox();
    m_buttonVLayout = new QVBoxLayout();
    m_buttonHLayout = new QHBoxLayout();
    m_buttonGroup->setLayout(m_buttonVLayout);

    m_buttonsLabel = new QLabel(this);
    m_buttonsLabel->setText(tr("Please choose to merge or replace the contents of the current card."));
    m_buttonsLabel->setAlignment(Qt::AlignCenter);
    m_buttonVLayout->addWidget(m_buttonsLabel);

    m_buttonVLayout->addLayout(m_buttonHLayout);
    m_layout->addWidget(m_buttonGroup);

    QSize buttonSize(200, 200);
    m_mergeButton = new QPushButton(this);  //@TODO: How on earth can an icon be added without being pixelated by Qt?
    QIcon mergeIcon;
    mergeIcon.addFile(":/images/merge_copy_400.png", QSize(400,400));
    mergeIcon.addFile(":/images/merge_copy_200.png", QSize(200,200));
    m_mergeButton->setFixedSize(buttonSize);
    m_mergeButton->setIconSize(buttonSize);
    m_mergeButton->setIcon(mergeIcon);
    m_mergeButton->setEnabled(false);
    m_mergeButton->setToolTip(tr("Copy the backup in each playlist behind the contents on the card"));
    m_buttonHLayout->addWidget(m_mergeButton);
    connect( m_mergeButton, &QAbstractButton::clicked, this, &BackupRestoreDialog::confirmMerge );

    m_overwriteButton = new QPushButton(this);  //@TODO: How on earth can an icon be added without being pixelated by Qt?
    QIcon overwriteIcon;
    overwriteIcon.addFile(":/images/overwrite_copy_400.png", QSize(400,400));
    overwriteIcon.addFile(":/images/overwrite_copy_200.png", QSize(200,200));
    m_overwriteButton->setIcon(overwriteIcon);
    m_overwriteButton->setFixedSize(buttonSize);
    m_overwriteButton->setIconSize(buttonSize);
    m_overwriteButton->setToolTip(tr("Replace all contents of the memory card by the contents of the backup"));
    m_overwriteButton->setEnabled(false);
    m_buttonHLayout->addWidget(m_overwriteButton);
    connect( m_overwriteButton, &QAbstractButton::clicked, this, &BackupRestoreDialog::confirmOverwrite );

    m_cancelButton = new QPushButton(this);
    m_cancelButton->setGeometry(QRect(250, 100, 112, 32));
    m_cancelButton->setText(tr("Cancel"));
    m_layout->addWidget(m_cancelButton, 0, Qt::AlignRight);
    connect( m_cancelButton, &QAbstractButton::clicked, this, &BackupRestoreDialog::close);

    QMetaObject::connectSlotsByName(this);

}

void BackupRestoreDialog::confirmMerge()
{
    close();
    emit mergeClicked( m_sourcePath, true );
}

void BackupRestoreDialog::confirmOverwrite()
{
    close();
    emit overwriteClicked( m_sourcePath, false );
}

void BackupRestoreDialog::setSourcePath(const QString &sourcePath)
{
    m_sourcePath = sourcePath;

    if( parseXml( sourcePath+BACKUP_INFO_FILE ) )
    {
        m_label->setText(tr("Backup of card")+": "+m_infoDriveName+"\n"+tr("Backup by")+": "+m_infoByWhom+" "+m_infoAppVersion+"\n"+tr("Backup date")+": "+m_infoLastWriteDate.toString( tr("yyyy-MM-dd hh:mm:ss")));
        m_mergeButton->setEnabled(true);
        m_overwriteButton->setEnabled(true);
    } else {
        m_label->setText(tr("Either the selected folder does not contain a backup made with this app,\nor the backup is incomplete. If this really is your backup folder,\nyou will have to restore that backup manually."));
    }
}


bool BackupRestoreDialog::parseXml(const QString &fileName)
{

    QFile* file = new QFile(fileName);

    if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "unable to open info.xml";
        return false;
    }

    QXmlStreamReader xml(file);
    QList< QMap<QString,QString> > infoData;

    while(!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();

        // If token is just StartDocument, we'll go to next.
        if(token == QXmlStreamReader::StartDocument)
        {
            continue;
        }

        // If token is StartElement, we'll see if we can read it.
        if(token == QXmlStreamReader::StartElement)
        {

            if(xml.name() == QString("hoerbert"))
            {
                continue;
            }

            if(xml.name() == QString("app_version"))
            {
                xml.readNext();
                m_infoAppVersion = xml.text().toString();
            }

            if(xml.name() == QString("last_write_date"))
            {
                xml.readNext();
                QString dateTimeString = xml.text().toString();
                if( !dateTimeString.isEmpty() )
                {
                    m_infoLastWriteDate = QDateTime::fromString( dateTimeString, "yyyyMMddhhmmss" );
                    qDebug() << "backup date: " << m_infoLastWriteDate.toString() << "with dateTimeString: " << dateTimeString;
                }
            }

            if(xml.name() == QString("drive_name"))
            {
                xml.readNext();
                m_infoDriveName = xml.text().toString();
            }

            if(xml.name() == QString("by_whom"))
            {
                xml.readNext();
                m_infoByWhom = xml.text().toString();
            }
        }
    }

    if(xml.hasError())
    {
        qDebug() << "xmlError occurred when reading info.xml";
        //return false; // this may not be fatal. Worst case: No information about the backup was found.
    }

    xml.clear();
    return true;
}
