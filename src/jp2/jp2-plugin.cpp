#include "jp2-handler.h"
#include <QImageIOPlugin>

class Jp2Plugin : public QImageIOPlugin
{
public:
    QStringList      keys() const;
    Capabilities     capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler* create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

Q_EXPORT_PLUGIN2(qjp2, Jp2Plugin)

QStringList
Jp2Plugin:: keys() const
{
     return QStringList({"jp2"});
}

QImageIOPlugin::Capabilities
Jp2Plugin:: capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "jp2") {
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
Jp2Plugin:: create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new Jp2Handler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

