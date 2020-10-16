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

#ifndef PLAYSYMBOLBUTTON_H
#define PLAYSYMBOLBUTTON_H

#include <QWidget>

class QHBoxLayout;
class QLabel;

/**
 * @brief Custom play/stop button widget
 */
class PlaySymbolButton : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief PlaySymbolButton Constructor
     * @param parent parent widget
     * @param id entry identifier
     */
    explicit PlaySymbolButton(QWidget *parent = nullptr, int id = -1);

    /**
     * @brief updates button state to playing
     */
    void play();

    /**
     * @brief updates button state to stopped
     */
    void stop();

    /**
     * @brief set size of actual button inside widget
     * @param width
     * @param height
     */
    void setButtonSize(int width, int height);

    /**
     * @brief isPlaying represents if button state is playing
     * @return true if button state is playing, otherwise false
     */
    bool isPlaying();

signals:

    /**
     * @brief this signal is emitted when the widget is clicked by mouse
     */
    void clicked();

    /**
     * @brief this signal is emitted when button state changes
     * @param id entry id
     * @param isPlaying true if button state is changed to isPlaying, otherwise false
     */
    void stateChanged(int id, bool isPlaying);

public slots:

private:

    /**
     * @brief update button image according to the state
     */
    void updateState();

    void mousePressEvent(QMouseEvent *e);

    void mouseReleaseEvent(QMouseEvent *e);

    bool m_isPressed;
    bool m_isPlaying;
    int m_ID;

    QHBoxLayout *m_layout;
    QLabel *m_pic;
};

#endif // PLAYSYMBOLBUTTON_H
