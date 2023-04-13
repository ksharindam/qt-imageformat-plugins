#include "avif-handler.h"
#include "avif-plugin.h"

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
Q_EXPORT_PLUGIN2(qavif, AvifPlugin)
#endif

QStringList
AvifPlugin:: keys() const
{
    return QStringList({"avif"});
}

QImageIOPlugin::Capabilities
AvifPlugin:: capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "avif") {
        return Capabilities(CanRead/* | CanWrite*/);
    }
    Capabilities cap;
    if (!format.isEmpty() or !device->isOpen())
        return cap;

    if (device->isReadable() && canReadImage(device))
        cap |= CanRead;
    /*if (device->isWritable())
        cap |= CanWrite;*/
    return cap;
}

QImageIOHandler *
AvifPlugin:: create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new AvifHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

