/*  This file is a part of qt4-imageformat-plugins project, and is GNU LGPLv2.1 licensed
    Copyright (C) 2020-2023 Arindam Chaudhuri <ksharindam@gmail.com>
*/
#include "jp2-handler.h"
#include "color.h"
#include <QDebug>


bool
Jp2Handler:: canRead() const
{
    return canReadImage(device());
}

bool
Jp2Handler:: read(QImage *image)
{
    QImage decoded = readImage(device());
    if (decoded.isNull())
        return false;
    *image = decoded;
    return true;
}

bool
Jp2Handler:: write(const QImage &image)
{
    return writeImage(image, device());
}


#define J2K_MAGIC "\xff\x4f\xff\x51"
#define JP2_MAGIC "\x0d\x0a\x87\x0a"
#define JP2_RFC3745_MAGIC "\x00\x00\x00\x0c\x6a\x50\x20\x20\x0d\x0a\x87\x0a"

bool isJp2(QIODevice *device)
{
    if (device->size() < 12)
        return false;
    QByteArray bytes = device->peek(12);
    // header for JP2 format
    if (memcmp(bytes.constData(), JP2_RFC3745_MAGIC, 12)==0 or
            memcmp(bytes.constData(), JP2_MAGIC, 4)==0)
        return true;
    return false;
}

bool isJ2k(QIODevice *device)
{
    if (device->size() < 12)
        return false;
    QByteArray bytes = device->peek(4);
    // header for J2K format
    if (memcmp(bytes.constData(), J2K_MAGIC, 4)==0)
        return true;
    return false;
}

bool canReadImage(QIODevice *device)
{
    if (!device)
        return false;
    return isJp2(device) or isJ2k(device);
}


OPJ_SIZE_T jp2_read_buffer(void *dest, OPJ_SIZE_T length, void *user_data)
{
    QIODevice *device = (QIODevice*) user_data;
    int len = device->read((char*)dest, length);
    if (len==0)
        return -1;//return 0 causes OpenJPEG to create infinte loop for reading
    return len;
}

OPJ_BOOL jp2_seek_buffer(OPJ_OFF_T offset, void *user_data)
{
    QIODevice *device = (QIODevice*) user_data;
    return device->seek(offset);
}

OPJ_OFF_T jp2_skip_buffer(OPJ_OFF_T offset, void *user_data)
{
    QIODevice *device = (QIODevice*) user_data;
    bool ok = device->seek(device->pos()+offset);
    if (not ok)
        return -1;
    return offset;
}

OPJ_SIZE_T jp2_write_buffer(void *src, OPJ_SIZE_T length, void *user_data)
{
    QIODevice *device = (QIODevice*) user_data;
    int len = device->write((char*)src, length);
    if (len == -1)
        return 0;
    return len;
}



QImage readImage(QIODevice *device)
{
    QImage image;
    int w, h, depth, channels, colorspace;
    // colorspace list according to OPJ_COLOR_SPACE enum
    QStringList clrspc_str = {"Unspecified", "sRGB", "Gray", "YCbCr", "xvYCC", "CMYK"};

    OPJ_CODEC_FORMAT format = isJ2k(device) ? OPJ_CODEC_J2K : OPJ_CODEC_JP2;
    opj_image_t *jp2_image = NULL;
    opj_codec_t *codec = NULL;

    opj_stream_t *stream = opj_stream_default_create (OPJ_TRUE);
    if (! stream)
        goto end;

    opj_stream_set_read_function(stream, jp2_read_buffer);
    opj_stream_set_seek_function(stream, jp2_seek_buffer);
    opj_stream_set_user_data(stream, device, NULL);
    opj_stream_set_user_data_length(stream, device->size());

    codec = opj_create_decompress (format);

    opj_dparameters_t  parameters;
    opj_set_default_decoder_parameters (&parameters);

    if (opj_setup_decoder (codec, &parameters) != OPJ_TRUE)
        goto end;

    if (opj_read_header (stream, codec, &jp2_image) != OPJ_TRUE)
    {
        qDebug("JP2 : Couldn't read header");
        goto end;
    }

    if (opj_decode (codec, stream, jp2_image) != OPJ_TRUE)
    {
        qDebug("JP2 : Couldn't decode image");
        goto end;
    }

    if (opj_end_decompress (codec, stream) != OPJ_TRUE)
    {
        qDebug("JP2 : Couldn't decompress image");
        goto end;
    }
    w = jp2_image->comps[0].w;
    h = jp2_image->comps[0].h;
    depth = jp2_image->comps[0].prec;
    channels = jp2_image->numcomps;
    colorspace = jp2_image->color_space;

    qDebug()<< "res ="<< w<< "x"<< h<< ", channels ="<< channels<< ", depth ="<< depth;
    if (colorspace!=-1)
        qDebug()<< "colorspace :"<< clrspc_str[colorspace];

    if (colorspace == OPJ_CLRSPC_SYCC) {
        if (! color_sycc_to_rgb (jp2_image)) {
            printf("JP2 : sYCC to sRGB conversion failed\n");
            goto end;
        }
    }

    if (channels>4)
        goto end;
    if (channels==1 or channels==3)
        image = QImage(w, h, QImage::Format_RGB32);
    else
        image = QImage(w, h, QImage::Format_ARGB32);

    if (channels >= 3) { // RGB or RGBA
        for (int y=0; y<h; y++) {
            QRgb *row = (QRgb*) image.scanLine(y);
            for (int x=0; x<w; x++) {
                int r = jp2_image->comps[0].data[y*w + x];
                int g = jp2_image->comps[1].data[y*w + x];
                int b = jp2_image->comps[2].data[y*w + x];
                row[x] = qRgb(r,g,b);
            }
        }
    }
    else { // Gray or Gray with alpha
        for (int y=0; y<h; y++) {
            QRgb *row = (QRgb*) image.scanLine(y);
            for (int x=0; x<w; x++) {
                int gray = jp2_image->comps[0].data[y*w + x];
                row[x] = qRgb(gray,gray,gray);
            }
        }
    }
    // Put Alpha Channel
    if (channels==2 or channels==4) {
        for (int y=0; y<h; y++) {
            QRgb *row = (QRgb*) image.scanLine(y);
            for (int x=0; x<w; x++) { //last channel is alpha
                int alpha = jp2_image->comps[channels-1].data[y*w + x];
                row[x] = (row[x] & 0x00ffffff) | (alpha << 24);
            }
        }
    }

end:
    if (jp2_image)
        opj_image_destroy (jp2_image);
    if (codec)
        opj_destroy_codec (codec);
    if (stream)
        opj_stream_destroy (stream);
    return image;
}


// ************** Write Image *******************

bool writeImage(QImage image, QIODevice *device)
{
    int w = image.width();
    int h = image.height();
    bool success = false;

    opj_image_t *jp2_image = NULL;
    opj_codec_t *codec = NULL;
    opj_stream_t *stream = NULL;

    opj_cparameters_t  parameters;
    opj_set_default_encoder_parameters(&parameters);
    parameters.tcp_numlayers = 1;
    // use these options to set fixed quality
    parameters.tcp_distoratio[0] = 40;
    parameters.cp_fixed_quality = OPJ_TRUE;
    // setting this 1 forces RGB->YCC conversion and higher compression
    parameters.tcp_mct = 1; // Multiple Component Tranform
    // Use these options to set fixed file size, (size = pixels_count * 3 / rate)
    //parameters.tcp_rates[0] = 32; // rate 32 = 32 times compression, 1 = lossless
    //parameters.cp_disto_alloc = OPJ_TRUE; // allocation by rate/distortion


    opj_image_cmptparm_t comp_info[3] = {};
    for (int i=0; i<3; i++) {
        comp_info[i].w = w;
        comp_info[i].h = h;
        comp_info[i].dx = comp_info[i].dy = 1;
        comp_info[i].prec = comp_info[i].bpp = 8;
    }
    jp2_image = opj_image_create(3, comp_info, OPJ_CLRSPC_SRGB);
    if (!jp2_image) {
        qDebug("JP2 : Could not create jp2_image");
        goto end;
    }
    jp2_image->x1 = w;
    jp2_image->y1 = h;
    // image format must be 32 bit format for copying
    if (image.format() != QImage::Format_RGB32 and image.format() != QImage::Format_ARGB32)
        image = image.convertToFormat(QImage::Format_RGB32);

    for (int y=0; y<h; y++) {
        QRgb *row = (QRgb*) image.constScanLine(y);
        for (int x=0; x<w; x++) {
            jp2_image->comps[0].data[y*w + x] = qRed(row[x]);
            jp2_image->comps[1].data[y*w + x] = qGreen(row[x]);
            jp2_image->comps[2].data[y*w + x] = qBlue(row[x]);
        }
    }

    stream = opj_stream_default_create (OPJ_FALSE);
    if (! stream)
        goto end;

    opj_stream_set_seek_function(stream, jp2_seek_buffer);
    opj_stream_set_skip_function(stream, jp2_skip_buffer);
    opj_stream_set_write_function(stream, jp2_write_buffer);
    opj_stream_set_user_data(stream, device, NULL);

    codec = opj_create_compress(OPJ_CODEC_JP2);
    if (opj_setup_encoder (codec, &parameters, jp2_image) != OPJ_TRUE)
    {
        qDebug("JP2 : Couldn't set parameters on encoder");
        goto end;
    }

    if ( opj_start_compress(codec, jp2_image, stream) != OPJ_TRUE  ||
        opj_encode(codec, stream) != OPJ_TRUE  ||
        opj_end_compress(codec, stream) != OPJ_TRUE )
    {
        qDebug("JP2 : encoding failed");
        goto end;
    }
    success = true;
end:
    if (jp2_image)
        opj_image_destroy (jp2_image);
    if (codec)
        opj_destroy_codec (codec);
    if (stream)
        opj_stream_destroy (stream);
    return success;
}

