#include "webp-handler.h"
#include <QImageIOPlugin>


class WebpPlugin : public QImageIOPlugin
{
public:
    QStringList      keys() const;
    Capabilities     capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler* create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

Q_EXPORT_PLUGIN2(qwebp, WebpPlugin)

QStringList
WebpPlugin:: keys() const
{
    return QStringList({"webp"});
}

QImageIOPlugin::Capabilities
WebpPlugin:: capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "webp") {
        return Capabilities(CanRead | CanWrite);
    }
    Capabilities cap;
    if (!format.isEmpty() or !device->isOpen())
        return cap;

    if (device->isReadable() && canReadImage(device))
        cap |= CanRead;
    if (device->isWritable())
        cap |= CanWrite;
    return cap;
}

QImageIOHandler *
WebpPlugin:: create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new WebpHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

