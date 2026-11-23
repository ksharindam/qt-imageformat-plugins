#pragma once

#include <QImageIOHandler>
#include <QImageIOPlugin>

class GdkPixbufPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface"
                      FILE "gdkpixbuf.json")
public:
    Capabilities capabilities(QIODevice *device,
                              const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device,
                            const QByteArray &format = QByteArray()) const override;
};

class GdkPixbufHandler : public QImageIOHandler
{
public:
    bool canRead() const override;
    bool read(QImage *image) override;
};
