/*  This file is a part of qt-imageformat-plugins project, and is GNU LGPLv2.1 licensed
    Copyright (C) 2023 Arindam Chaudhuri <ksharindam@gmail.com>
*/
#include "avif-handler.h"
#include <avif/avif.h>
#include <QDebug>

bool
AvifHandler:: canRead() const
{
    return canReadImage(device());
}

bool
AvifHandler:: read(QImage *image)
{
    QImage decoded = readImage(device());
    if (decoded.isNull())
        return false;
    *image = decoded;
    return true;
}

/*bool
AvifHandler:: write(const QImage &image)
{
    return writeImage(image, device());
}*/



bool isAvif(QIODevice *device)
{
    if (device->size() < 20)
        return false;
    device->seek(8);
    QByteArray bytes = device->peek(12);
    if (bytes.startsWith("avif") and bytes.endsWith("avif"))
        return true;
    return false;
}

bool canReadImage(QIODevice *device)
{
    return (device && isAvif(device));
}

bool isBigEndian()
{
    int i=1; return ! *((char *)&i);
}

QImage readImage(QIODevice *device)
{
    QByteArray bArr = device->readAll();
    const uchar *data = (uchar*) bArr.constData();
    size_t size = bArr.size();

    QImage image;
    QImage::Format format = QImage::Format_RGB32;

    avifRGBImage rgb;
    memset(&rgb, 0, sizeof(rgb));

    avifDecoder *decoder = avifDecoderCreate();

    avifResult result = avifDecoderSetIOMemory(decoder, data, size);
    if (result != AVIF_RESULT_OK) {
        qDebug() << "Cannot set IO on avifDecoder";
        goto cleanup;
    }

    if (avifDecoderParse(decoder) != AVIF_RESULT_OK) {
        qDebug() << "Failed to decode image";
        goto cleanup;
    }

    if (avifDecoderNextImage(decoder) != AVIF_RESULT_OK){
        goto cleanup;
    }

    avifRGBImageSetDefaults(&rgb, decoder->image);
    rgb.depth = 8;
    if (isBigEndian())
        rgb.format = AVIF_RGB_FORMAT_ARGB;
    else
        rgb.format = AVIF_RGB_FORMAT_BGRA;

    if (decoder->image->alphaPlane) {
        format = QImage::Format_ARGB32;
    }

    image = QImage(decoder->image->width, decoder->image->height, format);
    rgb.pixels = image.bits();
    rgb.rowBytes = 4*decoder->image->width;

    result = avifImageYUVToRGB(decoder->image, &rgb);
    if (result != AVIF_RESULT_OK) {
        qDebug() << "Conversion from YUV failed: " << avifResultToString(result);
        goto cleanup;
    }

cleanup:
    avifDecoderDestroy(decoder);
    return image;
}

