#include "gdkpixbuf-plugin.h"
#include <QByteArray>
#include <QIODevice>
#include <QStringList>
#include <QImage>

// GLib defines a "signals" macro, which conflicts with Qt's keyword macro.
#undef signals
#include <gdk-pixbuf/gdk-pixbuf.h>
#define signals Q_SIGNALS

bool supportsFormat(const QByteArray &format);


QImageIOPlugin::Capabilities
GdkPixbufPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (!device || !device->isOpen() || !device->isReadable())
        return Capabilities();

    if (!format.isEmpty())
        return supportsFormat(format) ? Capabilities(CanRead) : Capabilities();

    return canReadImage(device) ? Capabilities(CanRead) : Capabilities();
}

QImageIOHandler *
GdkPixbufPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new GdkPixbufHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

// --------- handler -------------

bool GdkPixbufHandler::canRead() const
{
    return canReadImage(device());
}

bool GdkPixbufHandler::read(QImage *image)
{
    QImage decoded = readImage(device());
    if (decoded.isNull())
        return false;
    *image = decoded;
    return true;
}


QImage readImage(QIODevice *device)
{
    if (!device)
        return QImage();

    const QByteArray bytes = device->readAll();
    if (bytes.isEmpty())
        return QImage();

    GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
    GError *error = nullptr;
    gboolean ok = gdk_pixbuf_loader_write(
        loader,
        reinterpret_cast<const guchar *>(bytes.constData()),
        static_cast<gsize>(bytes.size()),
        &error);
    if (ok)
        ok = gdk_pixbuf_loader_close(loader, &error);

    GdkPixbuf *pixbuf = ok ? gdk_pixbuf_loader_get_pixbuf(loader) : nullptr;
    if (pixbuf)
        g_object_ref(pixbuf);

    if (error) {
        qWarning() << "gdk-pixbuf failed to decode image:" << error->message;
        g_error_free(error);
    }
    g_object_unref(loader);

    if (!pixbuf)
        return QImage();

    const int width = gdk_pixbuf_get_width(pixbuf);
    const int height = gdk_pixbuf_get_height(pixbuf);
    const int channels = gdk_pixbuf_get_n_channels(pixbuf);
    const int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    const bool hasAlpha = gdk_pixbuf_get_has_alpha(pixbuf);
    const guchar *pixels = gdk_pixbuf_read_pixels(pixbuf);

    QImage image(width, height, hasAlpha ? QImage::Format_RGBA8888
                                        : QImage::Format_RGB888);
    if (image.isNull() || (channels != 3 && channels != 4)) {
        g_object_unref(pixbuf);
        return QImage();
    }

    const size_t rowBytes = static_cast<size_t>(width) * channels;
    for (int y = 0; y < height; ++y)
        std::memcpy(image.scanLine(y), pixels + y * rowstride, rowBytes);

    g_object_unref(pixbuf);
    return image;
}


bool canReadImage(QIODevice *device)
{
    if (!device || !device->isOpen() || !device->isReadable())
        return false;

    const QByteArray prefix = device->peek(4096);
    if (prefix.isEmpty())
        return false;

    GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
    GError *error = NULL;
    const gboolean accepted = gdk_pixbuf_loader_write(
        loader,
        reinterpret_cast<const guchar *>(prefix.constData()),
        static_cast<gsize>(prefix.size()),
        &error);
    GdkPixbufFormat *format = accepted ? gdk_pixbuf_loader_get_format(loader) : NULL;

    if (error)
        g_error_free(error);
    g_object_unref(loader);
    return format != NULL;
}


namespace {

QStringList supportedFormats()
{
    QStringList extensions;
    GSList *formats = gdk_pixbuf_get_formats();

    for (GSList *item = formats; item; item = item->next) {
        GdkPixbufFormat *format = static_cast<GdkPixbufFormat *>(item->data);
        if (gdk_pixbuf_format_is_disabled(format))
            continue;

        gchar **formatExtensions = gdk_pixbuf_format_get_extensions(format);
        for (gchar **extension = formatExtensions; extension && *extension; ++extension) {
            const QString key = QString::fromLatin1(*extension).toLower();
            if (!extensions.contains(key))
                extensions.append(key);
        }
        g_strfreev(formatExtensions);
    }

    extensions.sort();
    g_slist_free(formats);
    return extensions;
}

} // namespace

bool supportsFormat(const QByteArray &format)
{
    if (format.isEmpty())
        return false;

    const QString key = QString::fromLatin1(format).toLower();
    return supportedFormats().contains(key);
}


