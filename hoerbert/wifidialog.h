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

#ifndef WIFIDIALOG_H
#define WIFIDIALOG_H

#include <QDialog>

#include <QLabel>
#include <QLineEdit>
#include <QLayout>
#include <QPushButton>
#include "mainwindow.h"

class MainWindow;

/**
 * @brief A dialog to set WiFi access data
 */
class WifiDialog : public QDialog
{
    Q_OBJECT

public:
    WifiDialog(QWidget* parent = Q_NULLPTR);

signals:

private:
    MainWindow* m_mainWindow;

    QLabel* m_instructionLabel;

    QLineEdit *m_wifiSsid0;
    QLineEdit *m_wifiKey0;
    QLineEdit *m_wifiSsid1;
    QLineEdit *m_wifiKey1;
    QLineEdit *m_wifiSsid2;
    QLineEdit *m_wifiKey2;
    QLineEdit *m_wifiSsid3;
    QLineEdit *m_wifiKey3;
    QLineEdit *m_wifiSsid4;
    QLineEdit *m_wifiKey4;

    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;

    void readWifiSettings();
    void saveWifiSettings();
    void showEvent(QShowEvent * event);
};

#endif // WIFIDIALOG_H
