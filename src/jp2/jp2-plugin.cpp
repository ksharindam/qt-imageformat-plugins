#include "jp2-handler.h"
#include "jp2-plugin.h"

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
Q_EXPORT_PLUGIN2(qjp2, Jp2Plugin)
#endif

QStringList
Jp2Plugin:: keys() const
{
     return supported;
}

QImageIOPlugin::Capabilities
Jp2Plugin:: capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "jp2") {
        return Capabilities(CanRead | CanWrite);
    }
    if (supported.contains(format))
        return Capabilities(CanRead);

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

