#include "svg-handler.h"
#include "svg-plugin.h"

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
Q_EXPORT_PLUGIN2(qsvg, SvgPlugin)
#endif

QStringList
SvgPlugin::keys() const
{
    return QStringList({"svg"});
}

QImageIOPlugin::Capabilities
SvgPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "svg") {
        return Capabilities(CanRead);
    }
    Capabilities cap;
    if (!format.isEmpty() || !device->isOpen())
        return cap;

    if (device->isReadable() && canReadImage(device))
        cap |= CanRead;
    
    return cap;
}

QImageIOHandler *
SvgPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new SvgHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}