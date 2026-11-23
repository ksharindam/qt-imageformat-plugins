#pragma once
#include <QImageIOPlugin>
#include <QImageIOHandler>
#include <QImage>


class AvifPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "avif.json")
public:
    Capabilities     capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler* create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};


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
