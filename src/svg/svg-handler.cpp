/*  This file is a part of qt-imageformat-plugins project, and is GNU LGPLv2.1 licensed
    Copyright (C) 2020-2026 Arindam Chaudhuri <ksharindam@gmail.com>
*/
#include "svg-handler.h"
#include <QDebug>
#include <cstring>

// Undefine Qt's signals macro before including GLib to avoid conflicts
#undef signals
#include <librsvg/rsvg.h>
#include <cairo.h>
// Redefine Qt's signals macro
#define signals Q_SIGNALS


bool
SvgHandler::canRead() const
{
    return canReadImage(device());
}

bool
SvgHandler::read(QImage *image)
{
    QImage decoded = readImage(device());
    if (decoded.isNull())
        return false;
    *image = decoded;
    return true;
}


bool isSvg(QIODevice *device)
{
    if (device->size() < 5)
        return false;
    QByteArray bytes = device->peek(256);

    // Check for SVG magic bytes/patterns
    if (bytes.startsWith("<?xml") || bytes.startsWith("<svg") ||
        bytes.startsWith("<SVG") || bytes.contains("<svg ")) {
        return true;
    }
    return false;
}

bool canReadImage(QIODevice *device)
{
    if (!device)
        return false;
    return isSvg(device);
}

QImage readImage(QIODevice *device)
{
    QImage image;
    QByteArray svgData = device->readAll();

    if (svgData.isEmpty())
        return image;

    GError *error = nullptr;

    // Create RsvgHandle from data
    RsvgHandle *handle = rsvg_handle_new_from_data(
        (const guint8 *)svgData.constData(),
        svgData.size(),
        &error
    );

    if (!handle) {
        if (error) {
            qWarning() << "Failed to load SVG:" << error->message;
            g_error_free(error);
        }
        return image;
    }

    // Get SVG dimensions
    RsvgDimensionData dimensions;
    rsvg_handle_get_dimensions(handle, &dimensions);

    int width = dimensions.width ? dimensions.width : 256;
    int height = dimensions.height ? dimensions.height : 256;

    // Create cairo surface for rendering
    cairo_surface_t *surface = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32,
        width,
        height
    );

    if (!surface) {
        qWarning() << "Failed to create cairo surface";
        g_object_unref(handle);
        return image;
    }

    cairo_t *cr = cairo_create(surface);

    // Set background to white
    //cairo_set_source_rgb(cr, 1, 1, 1);
    //cairo_paint(cr);

    // Render SVG to cairo surface
    rsvg_handle_render_cairo(handle, cr);

    // Convert cairo surface to QImage
    int stride = cairo_image_surface_get_stride(surface);
    unsigned char *data = cairo_image_surface_get_data(surface);

    image = QImage(width, height, QImage::Format_ARGB32);

    for (int y = 0; y < height; ++y) {
        memcpy(
            image.scanLine(y),
            data + y * stride,
            width * 4
        );
    }

    // Cleanup
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_object_unref(handle);

    return image;
}
