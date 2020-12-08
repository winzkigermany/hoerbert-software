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

#ifndef PIEBUTTON_H
#define PIEBUTTON_H

#include <QPushButton>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QLabel>

#include "define.h"
#include "waitingspinnerwidget.h"


/**
 * @brief Custom styled button with pie progress
 */
class PieButton : public QPushButton
{
    Q_OBJECT
public:

    /**
     * @brief PieButton constructor
     * @param parent
     */
    explicit PieButton(QWidget *parent = Q_NULLPTR, int id = -1);

    /**
     * @brief setBackgroundColor
     */
    void setBackgroundColor(const QColor &);

    /**
     * @brief return the background color
     * @return the background color of this button
     */
    QColor backgroundColor();

    /**
     * @brief setFillColor
     */
    void setFillColor(const QColor &);

    /**
     * @brief set the button/directory/playlist id for this button
     */
    void setID(qint8 );

    qint8 ID();

    void setPercentage(int );

    /**
     * @brief get the percentage of the pie chart of this button
     * @return a percentage
     */
    int percentage();

    /**
     * @brief setProgress
     */
    void setProgress(int );

    /**
     * @brief setCount
     */
    void setCount(int );

    /**
     * @brief getCount
     * @return the number written on the button
     */
    int getCount();

    /**
     * @brief increasePercentage
     */
    void increasePercentage();

    /**
     * @brief set the graphical overlay pixmap for this button (glass effect)
     */
    void setOverlayPixmap(const QPixmap &);

    /**
     * @brief set the graphical overlay pixmap size for this button (glass effect)
     */
    void setOverlaySize(int, int);

    /**
     * @brief setMainPixmap
     */
    void setMainPixmap(const QPixmap &);

    /**
     * @brief setMainPixmapSize
     */
    void setMainPixmapSize(int, int);

    /**
     * @brief setFixedSize
     */
    void setFixedSize(int, int);

    /**
     * @brief setShadowBlurRadius
     */
    void setShadowBlurRadius(int );

    /**
     * @brief setShadowOffset
     */
    void setShadowOffset(int, int);

    /**
     * @brief setShadowColor
     */
    void setShadowColor(const QColor &);

    /**
     * @brief setShadowEnabled
     */
    void setShadowEnabled(bool );

    /**
     * @brief enable the button
     */
    void enable();

    /**
     * @brief disable the button
     */
    void disable();

    bool isOnProgress();

signals:
    /**
     * @brief triggered once the percentage of process reaches 100
     */
    void processCompleted();

public slots:

private:
    /*void enterEvent(QEvent *);
    void leaveEvent(QEvent *);*/
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);
    void updateStyleSheet();

    int m_id;
    int m_count;
    int m_percentage;
    bool m_enabled;
    bool m_shadowEnabled;

    QColor m_fillColor;
    QColor m_backColor;
    QColor m_color;
    QGraphicsDropShadowEffect *m_shadow;
    WaitingSpinnerWidget *m_spinner;

    QLabel *m_overlay;
    QLabel *m_mainPixmap;
};

#endif // PIEBUTTON_H
