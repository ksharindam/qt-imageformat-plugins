#pragma once

#include <QImageIOPlugin>
#include <QImageIOHandler>

class GdkPixbufPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "gdkpixbuf.json")
public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};


class GdkPixbufHandler : public QImageIOHandler
{
public:
    bool canRead() const;
    bool read(QImage *image);
};

bool canReadImage(QIODevice *device);
QImage readImage(QIODevice *device);
