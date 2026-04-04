#pragma once
#include <QImageIOHandler>
#include <QImage>

class SvgHandler : public QImageIOHandler
{
public:
    bool canRead() const;
    bool read(QImage *image);
    //bool write(const QImage &image);
};

QImage readImage(QIODevice *device);
//bool writeImage(QImage image, QIODevice *device);

bool canReadImage(QIODevice *device);
