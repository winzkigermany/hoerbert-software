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

#include "piebutton.h"


PieButton::PieButton(QWidget *parent, int id) : QPushButton(parent)
{
    setObjectName("PieButton");

#ifdef Q_OS_MACOS
    setFont(QFont(HOERBERT_FONTFAMILY, 21, QFont::DemiBold));
#elif defined (Q_OS_LINUX)
    setFont(QFont(HOERBERT_FONTFAMILY, 21, QFont::Normal));
#elif defined(Q_OS_WIN)
    setFont(QFont(HOERBERT_FONTFAMILY, 18, QFont::DemiBold));
#endif

    m_id = id;
    m_count = -1;
    m_percentage = 0;
    m_shadowEnabled = true;
    m_enabled = true;

    m_backColor = QColor(55, 157, 88, 225);
    m_color = QColor(0, 0, 0, 255);

    m_shadow = new QGraphicsDropShadowEffect();
    m_shadow->setBlurRadius(15);
    m_shadow->setOffset(3, 3);
    m_shadow->setColor(QColor(155, 155, 155));

    setGraphicsEffect(m_shadow);

    m_overlay = new QLabel(this);
    m_overlay->setPixmap(QPixmap(":/images/pie_overlay.png"));
    m_overlay->setScaledContents(true);

    m_mainPixmap = new QLabel(this);
    m_mainPixmap->setScaledContents(true);
    m_mainPixmap->setVisible(false);

    m_spinner = new WaitingSpinnerWidget( m_overlay, true, false );
    m_spinner->setRoundness(100.0);
    m_spinner->setMinimumTrailOpacity(0.0);
    m_spinner->setTrailFadePercentage(70.0);
    m_spinner->setNumberOfLines(12);
    m_spinner->setLineLength(5);
    m_spinner->setLineWidth(5);
    m_spinner->setInnerRadius(35);
    m_spinner->setRevolutionsPerSecond(0.25);
    m_spinner->setColor(QColor(255, 255, 255));
    m_spinner->stop();

    updateStyleSheet();
    setFixedSize(96, 96);
    setOverlaySize(96, 96);
}

void PieButton::setID(qint8 id)
{
    assert(id >= 0);
    m_id = id;
}

qint8 PieButton::ID()
{
    return m_id;
}

void PieButton::setPercentage(int value)
{
    if (value < 0 || value > 100)
        return;

    m_color = QColor(225, 225, 225, 255);
    updateStyleSheet();

    if (m_enabled)
    {
        m_enabled = false;
        m_spinner->start();
    }

    if (value == 0)
        setText("0%");

    if (m_percentage != value) {
        m_percentage = value;
        assert(m_percentage >=0 && m_percentage <= 100);
        setText(QString("%1%").arg(value));
        m_spinner->start();

        update();
        QApplication::processEvents();

        if (m_percentage == 100) {
            emit processCompleted();
            m_percentage = 0;
            m_spinner->stop();
        }
    }
}

int PieButton::percentage()
{
    return m_percentage;
}

void PieButton::setProgress(int count)
{
    setPercentage(count * 100 / m_count);
}

void PieButton::setCount(qint32 count)
{
    m_count = count;
    assert(m_count >= 0);
    setText(QString::number(m_count));
    m_color = QColor(0, 0, 0, 255);
    updateStyleSheet();
}

int PieButton::getCount()
{
    return m_count;
}

void PieButton::increasePercentage()
{
    setPercentage(static_cast<qint8>(m_percentage) + 1);
}

void PieButton::setBackgroundColor(const QColor &color)
{
    m_backColor = color;
    m_fillColor = QColor(m_backColor);
    m_fillColor.setAlpha(255);
    m_fillColor.setRed(m_fillColor.red() / 2);
    m_fillColor.setGreen(m_fillColor.green() / 2);
    m_fillColor.setBlue(m_fillColor.blue() / 2);
    updateStyleSheet();
}

QColor PieButton::backgroundColor()
{
    return m_backColor;
}

void PieButton::setFillColor(const QColor &color)
{
    m_fillColor = color;
    updateStyleSheet();
}

void PieButton::updateStyleSheet()
{
    QString background1 = QString("rgba(%1, %2, %3, %4)").arg(m_backColor.red())
            .arg(m_backColor.green()).arg(m_backColor.blue()).arg(225);
    QString background2 = QString("rgba(%1, %2, %3, %4)").arg(m_backColor.red())
            .arg(m_backColor.green()).arg(m_backColor.blue()).arg(155);
    QString background3 = QString("rgba(%1, %2, %3, %4)").arg(m_backColor.red())
            .arg(m_backColor.green()).arg(m_backColor.blue()).arg(255);
    QString foreground = QString("rgba(%1, %2, %3, %4)").arg(m_color.red())
            .arg(m_color.green()).arg(m_color.blue()).arg(255);
    setStyleSheet(QString(PIE_STYLE_TEMPLATE).arg(this->width() / 2).arg(background1).arg(background2).arg(background3).arg(foreground));
}

/*void PieButton::enterEvent(QEvent *e)
{
    m_backColor.setAlpha(155);
    updateStyleSheet();
}
void PieButton::leaveEvent(QEvent *e)
{
    m_backColor.setAlpha(225);
    m_shadow->setEnabled(true);
    updateStyleSheet();
}*/

void PieButton::setFixedSize(int w, int h)
{
    QPushButton::setFixedSize(w, h);

    updateStyleSheet();
}

void PieButton::setOverlayPixmap(const QPixmap &pix)
{
    m_overlay->setPixmap(pix);
    m_overlay->setFixedSize(width(), height());
}

void PieButton::setOverlaySize(int w, int h)
{
    m_overlay->setFixedSize(w, h);
}

void PieButton::setMainPixmap(const QPixmap &pix)
{
    m_mainPixmap->setPixmap(pix);
    m_mainPixmap->setFixedSize(width() * 3 / 5, height() * 3 / 5);
    m_mainPixmap->setGeometry((width() - m_mainPixmap->width()) / 2, (height() - m_mainPixmap->height()) / 2, m_mainPixmap->width(), m_mainPixmap->height());
    m_mainPixmap->setVisible(true);
    setText("");
}

void PieButton::setMainPixmapSize(int w, int h)
{
    m_mainPixmap->setFixedSize(w, h);
}

void PieButton::setShadowBlurRadius(int radius)
{
    m_shadow->setBlurRadius(radius);
}

void PieButton::setShadowOffset(int xoffset, int yoffset)
{
    m_shadow->setOffset(xoffset, yoffset);
}

void PieButton::setShadowColor(const QColor &color)
{
    m_shadow->setColor(color);
}

void PieButton::setShadowEnabled(bool flag)
{
    m_shadow->setEnabled(flag);
    m_shadowEnabled = flag;
}

void PieButton::enable()
{
    m_enabled = true;
    setText(QString::number(m_count));
}

void PieButton::disable()
{
    m_enabled = false;
    setText(QString());

    m_percentage = 0;
    m_count = -1;
}

bool PieButton::isOnProgress()
{
    return (m_percentage > 0 && m_percentage != 100);
}

void PieButton::mousePressEvent(QMouseEvent *e)
{
    if (!m_enabled)
        return;

    QPushButton::mousePressEvent(e);
    if (m_shadowEnabled)
        m_shadow->setEnabled(false);
}

void PieButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (!m_enabled)
        return;

    QPushButton::mouseReleaseEvent(e);
    if (m_shadowEnabled)
        m_shadow->setEnabled(true);
}

void PieButton::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    return;
}

void PieButton::paintEvent(QPaintEvent *e)
{
    if (m_percentage != 0)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        //painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);

        painter.setBrush(QBrush(m_fillColor));
        painter.setPen(Qt::NoPen);
        painter.drawPie(QRect(0, 0, width(), height()), 90 * 16, -static_cast<int>(360 * static_cast<qreal>(m_percentage) / static_cast<qreal>(100) * 16));
        m_spinner->setVisible(true);
    }
    else
    {
        m_spinner->stop();
    }

    QPushButton::paintEvent(e);
}
