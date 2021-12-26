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

#include "aboutdialog.h"

#include <QLocale>
#include <QWidget>
#include <QApplication>

#include "version.h"

/**
 * Please keep the references to other projects updated.
 */
AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
{
    if (objectName().isEmpty())
        setObjectName(QString("AboutDialog"));

    resize(640, 396);
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setSizePolicy(sizePolicy);
    setWindowTitle(tr("About this software"));

    m_disclaimer = new QTextBrowser(this);
    m_disclaimer->setObjectName("disclaimer");
    m_disclaimer->setAlignment(Qt::AlignHCenter);
    m_disclaimer->resize( 600, 160 );
    m_disclaimer->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    m_disclaimer->setReadOnly(true);
    m_disclaimer->setStyleSheet("#disclaimer {background: white; color: black}");
    m_disclaimer->setOpenExternalLinks(true);
    m_disclaimer->setOpenLinks(true);
    m_disclaimer->setHtml("<html><body><p>" + tr("This program is free software: You can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.") + "</p><p>"
                          + tr("This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.") + "</p><p>"
                          + tr("You should have received a copy of the GNU General Public License along with this program. If not, see")
                          + " "+"<a href='http://www.gnu.org/licenses/'>http://www.gnu.org/licenses/</a></p><p>"
                          + tr("You will find the source code of this program here:")
                          + " "+"<a href='https://github.com/winzkigermany/hoerbert-software'>https://github.com/winzkigermany/hoerbert-software</a></p><p>"
                          + tr("This program makes use of other code from other good people, namely")
                          + "<br>ffmpeg <a href='https://www.ffmpeg.org'>https://www.ffmpeg.org</a><br>freac <a href='https://www.freac.org'>https://www.freac.org</a><br>sox <a href='http://sox.sourceforge.net'>http://sox.sourceforge.net</a><br>wavsilence <a href='https://github.com/DOSx86/wavsilence'>https://github.com/DOSx86/wavsilence</a><br>QtWaitingSpinner <a href='https://github.com/snowwlex/QtWaitingSpinner'>https://github.com/snowwlex/QtWaitingSpinner</a><br>df by Paul Sadowski<br>EjectMedia <a href='https://www.uwe-sieber.de'>https://www.uwe-sieber.de</a><br>7-Zip <a href='https://7-zip.org'>https://7-Zip.org</a><br>OpenClipart.org <a href='https://www.openclipart.org'>https://OpenClipart.org</a></p></body></html>");

    m_copyright = new QLabel(this);
    m_copyright->setObjectName(QString("copyright"));
    m_copyright->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    m_copyright->setAlignment(Qt::AlignCenter);
    m_copyright->setText(tr("Copyright (C) 2019 WINZKI GmbH & Co. KG"));

    QFont font;
    font.setBold(true);
    font.setWeight(QFont::Weight::Bold);

    m_company = new QLabel(this);
    m_company->setObjectName(QString("company"));
    m_company->setFont(font);
    m_company->setAlignment(Qt::AlignCenter);
    m_company->setText(tr("h\303\266rbert Software"));

    m_version = new QLabel(this);
    m_version->setObjectName(QString("app_version"));
    m_version->setFont(font);
    m_version->setAlignment(Qt::AlignCenter);
    m_version->setText(tr("Version")+" "+VER_PRODUCTVERSION_STR);

    m_companysite = new QLabel(this);
    m_companysite->setObjectName(QString("companysite"));
    m_companysite->setAlignment(Qt::AlignCenter);
    m_companysite->setText(tr("www.hoerbert.com"));

    m_checkForUpdateButton = new QPushButton(this);
    m_checkForUpdateButton->setFixedSize(250, 40);
    m_checkForUpdateButton->setObjectName(QString("check_for_new_version"));
    m_checkForUpdateButton->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
    m_checkForUpdateButton->setText(tr("Check for newer version"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(20);
    layout->addWidget(m_company);
    layout->addWidget(m_copyright);
    layout->addWidget(m_companysite);
    layout->addSpacing(10);
    layout->addWidget(m_version);
    layout->addWidget(m_checkForUpdateButton, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(m_disclaimer);

    m_cancelButton = new QPushButton(this);
    m_cancelButton->setGeometry(QRect(250, 100, 112, 32));
    m_cancelButton->setText(tr("Close"));
    layout->addWidget(m_cancelButton, 0, Qt::AlignRight);
    connect( m_cancelButton, &QAbstractButton::clicked, this, &AboutDialog::close);


    connect(m_checkForUpdateButton, &QPushButton::clicked, this, [this] () {
        emit checkForUpdateRequested();
    });
}
