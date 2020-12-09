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

#include "advancedfeaturesdialog.h"

#include <QSettings>
#include <QLabel>
#include <QRadioButton>
#include <QObject>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QButtonGroup>
#include <QDebug>
#include <QMessageBox>

#include "version.h"
#include "define.h"

AdvancedFeaturesDialog::AdvancedFeaturesDialog(QWidget *parent)
    : QDialog (parent)
{
    setWindowTitle(tr("Advanced features"));
    setFixedSize(600, 396);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(25, 20, 25, 25);
    m_layout->setAlignment(Qt::AlignHCenter);

    m_versionLabel = new QLabel(this);
    m_versionLabel->setAlignment(Qt::AlignCenter);
    m_versionLabel->setText(tr("Software version: ")+" "+VER_PRODUCTVERSION_STR);

    m_companyLabel = new QLabel(this);
    m_companyLabel->setAlignment(Qt::AlignCenter);
    m_companyLabel->setText(tr("(c) WINZKI GmbH & Co. KG"));

    m_siteUrlLabel = new QLabel(this);
    m_siteUrlLabel->setAlignment(Qt::AlignCenter);
    m_siteUrlLabel->setText(tr("<a href=\"https://en.hoerbert.com/\"><span style='color: #5599ee'>https://www.hoerbert.com</span></a>"));
    m_siteUrlLabel->setTextFormat(Qt::RichText);
    m_siteUrlLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_siteUrlLabel->setOpenExternalLinks(true);

    m_companyLayout = new QHBoxLayout();
    m_companyLayout->setContentsMargins(0, 0, 0, 0);
    m_companyLayout->addWidget(m_companyLabel, 0, Qt::AlignLeft);
    m_companyLayout->addWidget(m_siteUrlLabel, 0, Qt::AlignRight);

    m_maxVolumeLabel = new QLabel(this);
    m_maxVolumeLabel->setAlignment(Qt::AlignCenter);
    m_maxVolumeLabel->setText(tr("Maximum volume:"));

    m_showButtonsLabel = new QLabel(this);
    m_showButtonsLabel->setAlignment(Qt::AlignCenter);
    m_showButtonsLabel->setText(tr("Show buttons:"));

    m_showNineButtons = new QRadioButton(this);
    m_showNineButtons->setText(tr("9(default)"));

    m_showThreeButtons = new QRadioButton(this);
    m_showThreeButtons->setText("3");

    m_showOneButton = new QRadioButton(this);
    m_showOneButton->setText("1");

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->addButton(m_showNineButtons, 9);
    m_buttonGroup->addButton(m_showThreeButtons, 3);
    m_buttonGroup->addButton(m_showOneButton, 1);

    m_showButtonLayout = new QHBoxLayout();
    m_showButtonLayout->setContentsMargins(0, 0, 0, 0);
    m_showButtonLayout->setAlignment(Qt::AlignCenter);
    m_showButtonLayout->addWidget(m_showButtonsLabel);
    m_showButtonLayout->addWidget(m_showNineButtons);
    m_showButtonLayout->addWidget(m_showThreeButtons);
    m_showButtonLayout->addWidget(m_showOneButton);

    m_lowVolumeOption = new QRadioButton(this);
    m_lowVolumeOption->setText(tr("-6dB (very low)"));

    m_normalVolumeOption = new QRadioButton(this);
    m_normalVolumeOption->setText(tr("-1.5dB (normal)"));

    m_maxVolumeOption = new QRadioButton(this);
    m_maxVolumeOption->setText(tr("0dB (maximum)"));

    m_volumeGroup = new QButtonGroup(this);
    m_volumeGroup->addButton(m_lowVolumeOption);
    m_volumeGroup->addButton(m_normalVolumeOption);
    m_volumeGroup->addButton(m_maxVolumeOption);

    m_optionLayout = new QHBoxLayout();
    m_optionLayout->setContentsMargins(0, 0, 0, 0);
    m_optionLayout->setAlignment(Qt::AlignCenter);
    m_optionLayout->addWidget(m_maxVolumeLabel);
    m_optionLayout->addWidget(m_lowVolumeOption);
    m_optionLayout->addWidget(m_normalVolumeOption);
    m_optionLayout->addWidget(m_maxVolumeOption);

    m_reminderOption = new QCheckBox(this);
    m_reminderOption->setText(tr("Remind of backups regularly"));

    m_showLargeDriveCheck = new QCheckBox(this);
    m_showLargeDriveCheck->setText(tr("Show drives larger than %1GB").arg(VOLUME_SIZE_LIMIT));

    m_regenerateXmlCheck = new QCheckBox(this);
    m_regenerateXmlCheck->setText( tr("Always regenerate hoerbert.xml for the old hoerbert app versions 1.x") );

    m_checkLayout = new QVBoxLayout;
    m_checkLayout->setAlignment(Qt::AlignCenter);

    m_checkLayout->addWidget(m_reminderOption);
    m_checkLayout->addWidget(m_showLargeDriveCheck);
    m_checkLayout->addWidget(m_regenerateXmlCheck);

    m_closeButton = new QPushButton(this);
    m_closeButton->setText(tr("Close"));
    m_closeButton->setFixedWidth(100);

    m_layout->addWidget(m_versionLabel);
    m_layout->addLayout(m_companyLayout);
    m_layout->addLayout(m_optionLayout);
    m_layout->addLayout(m_checkLayout, 0);
    m_layout->addLayout(m_showButtonLayout);
    m_layout->addItem(new QSpacerItem(100, 15, QSizePolicy::Fixed, QSizePolicy::Maximum));
    m_layout->addWidget(m_closeButton, 0, Qt::AlignRight);

    connect(m_buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, [this](int count) {
        QSettings settings;
        settings.beginGroup("Global");
        settings.setValue("buttons", count);
        settings.endGroup();

        emit buttonSettingsChanged();
    });

    connect(m_lowVolumeOption, &QRadioButton::clicked, [this] () {
        writeVolumeSettings("-6");
    });

    connect(m_normalVolumeOption, &QRadioButton::clicked, [this] () {
        writeVolumeSettings("-1.5");
    });

    connect(m_maxVolumeOption, &QRadioButton::clicked, [this] () {
        writeVolumeSettings("0");
    });


    connect(m_reminderOption, &QCheckBox::clicked, this, [this] (int state) {
        Q_UNUSED(state)
        QSettings settings;
        settings.beginGroup("Global");
        settings.setValue("showBackupReminder", m_reminderOption->isChecked());
        settings.endGroup();
    });

    connect(m_showLargeDriveCheck, &QCheckBox::stateChanged, this, [this] (int state) {
        Q_UNUSED(state)
        QSettings settings;
        settings.beginGroup("Global");
        settings.setValue("showLargeDrives", m_showLargeDriveCheck->isChecked());
        settings.endGroup();
    });

    connect(m_regenerateXmlCheck, &QCheckBox::stateChanged, this, [this] (int state) {
        Q_UNUSED(state)
        QSettings settings;
        settings.beginGroup("Global");
        settings.setValue("regenerateHoerbertXml", m_regenerateXmlCheck->isChecked());
        settings.endGroup();
    });

    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::close);

    readSettings();
}

void AdvancedFeaturesDialog::readSettings()
{
    QSettings settings;
    settings.beginGroup("Global");

    auto buttons = settings.value("buttons").toInt();
    switch (buttons) {
        case 1:
            m_showOneButton->setChecked(true);
            break;
        case 3:
            m_showThreeButtons->setChecked(true);
            break;
        case 9:
        default:
            m_showNineButtons->setChecked(true);
    }

    QString volume = settings.value("volume").toString();

    if (volume.compare("-6") == 0)
        m_lowVolumeOption->setChecked(true);
    else if (volume.compare("0") == 0)
        m_maxVolumeOption->setChecked(true);
    else
        m_normalVolumeOption->setChecked(true);

    bool showReminder = settings.value("showBackupReminder").toBool();
    m_reminderOption->setChecked(showReminder);

    bool showLarge = settings.value("showLargeDrives").toBool();
    m_showLargeDriveCheck->setChecked(showLarge);

    bool generateXml = settings.value("regenerateHoerbertXml").toBool();
    m_regenerateXmlCheck->setChecked(generateXml);

    settings.endGroup();
}

void AdvancedFeaturesDialog::writeVolumeSettings(const QString &volume)
{
    QSettings settings;
    settings.beginGroup("Global");
    settings.setValue("volume", volume);
    settings.endGroup();
}
