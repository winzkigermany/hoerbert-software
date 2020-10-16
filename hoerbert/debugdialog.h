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

#ifndef DEBUGDIALOG_H
#define DEBUGDIALOG_H

#include <QDialog>
#include <QMutex>

class QPlainTextEdit;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;

/**
 * @brief A dialog to show error messages to user for debug purpose
 */
class DebugDialog : public QDialog
{
    Q_OBJECT
public:

    /**
     * @brief DebugDialog constructor
     * @param parent
     */
    DebugDialog(QWidget *parent = Q_NULLPTR);

    /**
     * @brief add text to current log
     * @param text text to be added
     */
    void appendLog(const QString &text);

    /**
     * @brief set log with given text
     * @param text text to be set
     */
    void setLog(const QString &text);

    /**
     * @brief set the flag to determine whether to automatically
     *        show dialog when new text is appended or set
     * @param flag the flag
     */
    void setAutoVisable(bool flag);

protected:

    virtual void closeEvent(QCloseEvent *e) override;

private slots:

    /**
     * @brief send error message to backend
     */
    void sendErrorMessage();

private:
    QPlainTextEdit *m_logEdit; // text container
    QPushButton *m_sendButton; // send text to backend
    QPushButton *m_copyButton; // copy text to clipboard
    QPushButton *m_closeButton; // hides the dialog
    QVBoxLayout *m_layout;
    QHBoxLayout *m_buttonLayout;

    QMutex m_mutex;

    bool m_autoVisible;
    int m_index;
};

//extern DebugDialog *dbgDlg;

#endif // DEBUGDIALOG_H
