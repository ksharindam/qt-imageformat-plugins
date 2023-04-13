#pragma once
#include <QImageIOHandler>
#include <QImage>

class AvifHandler : public QImageIOHandler
{
public:
    bool canRead() const;
    bool read(QImage *image);
    //bool write(const QImage &image);
};

bool canReadImage(QIODevice *device);
QImage readImage(QIODevice *device);
//bool writeImage(QImage image, QIODevice *device);

