/*  This file is a part of qt4-imageformat-plugins project, and is GNU LGPLv2.1 licensed
    Copyright (C) 2020 Arindam Chaudhuri <ksharindam@gmail.com>
*/
#include "webp-handler.h"
#include <webp/decode.h>
#include <webp/encode.h>
#include <QDebug>

bool
WebpHandler:: canRead() const
{
    return canReadImage(device());
}

bool
WebpHandler:: read(QImage *image)
{
    QImage decoded = readImage(device());
    if (decoded.isNull())
        return false;
    *image = decoded;
    return true;
}

bool
WebpHandler:: write(const QImage &image)
{
    return writeImage(image, device());
}



bool isWebp(QIODevice *device)
{
    if (device->size() < 12)
        return false;
    QByteArray bytes = device->peek(12);
    if (bytes.startsWith("RIFF") and bytes.endsWith("WEBP"))
        return true;
    return false;
}

bool canReadImage(QIODevice *device)
{
    if (!device)
        return false;
    return isWebp(device);
}

bool isBigEndian()
{
    int i=1; return ! *((char *)&i);
}

QImage readImage(QIODevice *device)
{
    QImage image;
    QByteArray bArr = device->readAll();
    const uchar *data = (uchar*) bArr.constData();
    size_t size = bArr.size();
    // get image info (width, height, has_alpha)
    WebPBitstreamFeatures info;
    VP8StatusCode status = WebPGetFeatures(data, size, &info);
    if (status != VP8_STATUS_OK)
        return image;
    // decode image with same byte order as QImage
    int w=0, h=0;
    uint8_t *rgb_data = 0;
    if (isBigEndian())
        rgb_data = WebPDecodeARGB(data, size, &w, &h);
    else
        rgb_data = WebPDecodeBGRA(data, size, &w, &h);
    if (!rgb_data)
        return image;
    QImage::Format format = info.has_alpha ? QImage::Format_ARGB32 : QImage::Format_RGB32;
    image = QImage(w, h, format);
    memcpy(image.bits(), rgb_data, w*h*4);
    // free data
    WebPFree(rgb_data);
    return image;
}

void switchByteOrder(QImage &image)
{
    int w = image.width();
    int h = image.height();
    for (int y=0; y<h; y++) {
        QRgb *row = (QRgb*) image.scanLine(y);
        for (int x=0; x<w; x++) { // Red<->Green and Blue<->Alpha swapped
            row[x] = qRgba(qGreen(row[x]), qRed(row[x]), qAlpha(row[x]), qBlue(row[x]));
        }
    }
}

bool writeImage(QImage image, QIODevice *device)
{
    if (image.isNull())
        return false;
    float quality = 75;
    int w = image.width();
    int h = image.height();
    uchar *output=0;
    size_t size=0;// output file size
    // encode image
    if (not image.hasAlphaChannel()) {
        image = image.convertToFormat(QImage::Format_RGB888);
        size = WebPEncodeRGB(image.constBits(), w, h, image.bytesPerLine(), quality, &output);
    }
    else {
        if (image.format() != QImage::Format_ARGB32)
            image = image.convertToFormat(QImage::Format_ARGB32);

        if (isBigEndian())
            switchByteOrder(image);
        size = WebPEncodeBGRA(image.constBits(), w, h, image.bytesPerLine(), quality, &output);
    }
    if (size==0)
        return false;
    qint64 file_size = device->write((char*)output, size);
    // free data
    WebPFree(output);
    if (file_size != size)
        return false;
    return true;
}
