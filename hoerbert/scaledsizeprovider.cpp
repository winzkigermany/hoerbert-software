#include "scaledsizeprovider.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>

QSize ScaledSizeProvider::getScaledSize(const QSize &size)
{
    return {static_cast<int>(size.width() * getXScaleFactor()), static_cast<int>(size.height() * getYScaleFactor())};
}

QSize ScaledSizeProvider::getScaledSize(const int width, const int height)
{
    return getScaledSize( QSize( width, height ) );
}


qreal ScaledSizeProvider::getXScaleFactor()
{
    auto desktopWidget = QApplication::desktop();
    qDebug() << "X" << desktopWidget->logicalDpiX() << "/" << getReferenceDpiValue();
    return desktopWidget->logicalDpiX() / getReferenceDpiValue();
}

qreal ScaledSizeProvider::getYScaleFactor()
{
    auto desktopWidget = QApplication::desktop();
    qDebug() << "Y" << desktopWidget->logicalDpiX() << "/" << getReferenceDpiValue();
    return desktopWidget->logicalDpiY() / getReferenceDpiValue();
}

qreal ScaledSizeProvider::getReferenceDpiValue()
{
#ifdef WIN32
    return 96.0;
#else
    return 72.0;
#endif
}

qreal ScaledSizeProvider::getFontSizeFactor()
{
    auto desktopWidget = QApplication::desktop();
    qDebug() << "Font" << getReferenceDpiValue() << "/" << desktopWidget->logicalDpiX() ;
    return  getReferenceDpiValue() / desktopWidget->logicalDpiX();
}
