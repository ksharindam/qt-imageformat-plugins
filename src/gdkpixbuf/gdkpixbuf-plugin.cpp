#include "gdkpixbuf-plugin.h"

#include <QDebug>
#include <cstring>

// GLib defines a "signals" macro that conflicts with Qt's keyword macro.
#undef signals
#include <gdk-pixbuf/gdk-pixbuf.h>
#define signals Q_SIGNALS

namespace {

bool canDecode(QIODevice *device)
{
    if (!device || !device->isOpen() || !device->isReadable())
        return false;

    // peek() preserves the device position, which is important because Qt calls
    // capabilities() before handing the device to the handler.
    const QByteArray header = device->peek(64 * 1024);
    if (header.isEmpty())
        return false;

    GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
    GError *error = nullptr;
    const gboolean accepted = gdk_pixbuf_loader_write(
        loader,
        reinterpret_cast<const guchar *>(header.constData()),
        static_cast gsize(header.size()),
        &error);
    const bool recognized = accepted && gdk_pixbuf_loader_get_format(loader);

    if (error)
        g_error_free(error);
    g_object_unref(loader);
    return recognized;
}

QImage decode(QIODevice *device)
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
        static_cast gsize(bytes.size()),
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

} // namespace

QImageIOPlugin::Capabilities
GdkPixbufPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (!format.isEmpty()) {
        // Metadata keys declare common extensions; let gdk-pixbuf validate the
        // actual content rather than trusting the filename alone.
        if (format == "bmp" || format == "gif" || format == "ico" ||
            format == "jpeg" || format == "jpg" || format == "png" ||
            format == "tif" || format == "tiff" || format == "xbm" ||
            format == "xpm" || format == "webp" || format == "avif")
            return device && device->isReadable() ? Capabilities(CanRead) : Capabilities();
        return Capabilities();
    }

    return canDecode(device) ? Capabilities(CanRead) : Capabilities();
}

QImageIOHandler *GdkPixbufPlugin::create(QIODevice *device,
                                         const QByteArray &format) const
{
    QImageIOHandler *handler = new GdkPixbufHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

bool GdkPixbufHandler::canRead() const
{
    return canDecode(device());
}

bool GdkPixbufHandler::read(QImage *image)
{
    if (!image)
        return false;

    const QImage decoded = decode(device());
    if (decoded.isNull())
        return false;
    *image = decoded;
    return true;
}
