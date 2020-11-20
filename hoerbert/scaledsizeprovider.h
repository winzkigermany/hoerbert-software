#ifndef SCALEDSIZEPROVIDER_H
#define SCALEDSIZEPROVIDER_H

#include <QSize>

class ScaledSizeProvider
{
public:
//    ScaledSizeProvider();

    static QSize getScaledSize(const QSize &size);
    static QSize getScaledSize(const int width, const int height);
    static qreal getXScaleFactor();
    static qreal getYScaleFactor();
    static qreal getReferenceDpiValue();
    static qreal getFontSizeFactor();
};

#endif // SCALEDSIZEPROVIDER_H
