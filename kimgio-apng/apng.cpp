/*
 *  This file is part of KIMToy, an input method frontend for KDE
 *  Copyright (C) 2011 Ni Hui <shuizhuyuanluo@126.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "apng.h"

#ifdef USE_INTERNAL_PNG
#include "libpng-apng/png.h"
#else // USE_INTERNAL_PNG
#include <png.h>
#endif // USE_INTERNAL_PNG

#include <QDebug>
#include <QImage>
#include <QVector>
#include <QVariant>

#include <qbytearray.h>
#include <qimageiohandler.h>
#include <qiodevice.h>
#include <qstringlist.h>
#include <qtextcodec.h>

class QAPngHandlerPrivate
{
public:
    QAPngHandler* const q;

    bool readDone;

    float gamma;
    QString description;

    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;
    png_bytepp row_pointers;

    bool isAPNG;
    int frameIndex;
    int frameCount;
    int playCount;
    int nextDelay;

    QAPngHandlerPrivate(QAPngHandler* qq);
    ~QAPngHandlerPrivate();

    bool readAPngHeader();
    QImage::Format readImageFormat();

    bool readImage(QImage* outImage);

    bool getNextImage(QImage* result);
    int currentImageNumber() const;
    int imageCount() const;
    int loopCount() const;
    int nextImageDelay() const;
};

static void iod_read_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    QAPngHandlerPrivate* d = (QAPngHandlerPrivate*)png_get_io_ptr(png_ptr);
    QIODevice* in = d->q->device();
    while (length) {
        int nread = in->read((char*)data, length);
        if (nread <= 0) {
            png_error(png_ptr, "Read Error");
            return;
        }
        length -= nread;
    }
}

static void setup_qt(QImage& image, png_structp png_ptr, png_infop info_ptr, float screen_gamma)
{
    if (screen_gamma != 0.0 && png_get_valid(png_ptr, info_ptr, PNG_INFO_gAMA)) {
        double file_gamma;
        png_get_gAMA(png_ptr, info_ptr, &file_gamma);
        png_set_gamma(png_ptr, screen_gamma, file_gamma);
    }

    png_uint_32 width;
    png_uint_32 height;
    int bit_depth;
    int color_type;
    png_bytep trans_alpha = 0;
    png_color_16p trans_color_p = 0;
    int num_trans;
    png_colorp palette = 0;
    int num_palette;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);
    png_set_interlace_handling(png_ptr);

    if (color_type == PNG_COLOR_TYPE_GRAY) {
        // Black & White or 8-bit grayscale
        if (bit_depth == 1 && png_get_channels(png_ptr, info_ptr) == 1) {
            png_set_invert_mono(png_ptr);
            png_read_update_info(png_ptr, info_ptr);
            if (image.size() != QSize(width, height) || image.format() != QImage::Format_Mono) {
                image = QImage(width, height, QImage::Format_Mono);
                if (image.isNull())
                    return;
            }
            image.setColorCount(2);
            image.setColor(1, qRgb(0,0,0));
            image.setColor(0, qRgb(255,255,255));
        } else if (bit_depth == 16 && png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
            png_set_expand(png_ptr);
            png_set_strip_16(png_ptr);
            png_set_gray_to_rgb(png_ptr);
            if (image.size() != QSize(width, height) || image.format() != QImage::Format_ARGB32) {
                image = QImage(width, height, QImage::Format_ARGB32);
                if (image.isNull())
                    return;
            }
            if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
                png_set_swap_alpha(png_ptr);

            png_read_update_info(png_ptr, info_ptr);
        } else {
            if (bit_depth == 16)
                png_set_strip_16(png_ptr);
            else if (bit_depth < 8)
                png_set_packing(png_ptr);
            int ncols = bit_depth < 8 ? 1 << bit_depth : 256;
            png_read_update_info(png_ptr, info_ptr);
            if (image.size() != QSize(width, height) || image.format() != QImage::Format_Indexed8) {
                image = QImage(width, height, QImage::Format_Indexed8);
                if (image.isNull())
                    return;
            }
            image.setColorCount(ncols);
            for (int i=0; i<ncols; i++) {
                int c = i*255/(ncols-1);
                image.setColor(i, qRgba(c,c,c,0xff));
            }
            if (png_get_tRNS(png_ptr, info_ptr, &trans_alpha, &num_trans, &trans_color_p) && trans_color_p) {
                const int g = trans_color_p->gray;
                if (g < ncols) {
                    image.setColor(g, 0);
                }
            }
        }
    } else if (color_type == PNG_COLOR_TYPE_PALETTE
               && png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette)
               && num_palette <= 256)
    {
        // 1-bit and 8-bit color
        if (bit_depth != 1)
            png_set_packing(png_ptr);
        png_read_update_info(png_ptr, info_ptr);
        png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);
        QImage::Format format = bit_depth == 1 ? QImage::Format_Mono : QImage::Format_Indexed8;
        if (image.size() != QSize(width, height) || image.format() != format) {
            image = QImage(width, height, format);
            if (image.isNull())
                return;
        }
        png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
        image.setColorCount(num_palette);
        int i = 0;
        if (png_get_tRNS(png_ptr, info_ptr, &trans_alpha, &num_trans, &trans_color_p) && trans_alpha) {
            while (i < num_trans) {
                image.setColor(i, qRgba(
                    palette[i].red,
                    palette[i].green,
                    palette[i].blue,
                    trans_alpha[i]
                   )
               );
                i++;
            }
        }
        while (i < num_palette) {
            image.setColor(i, qRgba(
                palette[i].red,
                palette[i].green,
                palette[i].blue,
                0xff
               )
           );
            i++;
        }
    } else {
        // 32-bit
        if (bit_depth == 16)
            png_set_strip_16(png_ptr);

        png_set_expand(png_ptr);

        if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png_ptr);

        QImage::Format format = QImage::Format_ARGB32;
        // Only add filler if no alpha, or we can get 5 channel data.
        if (!(color_type & PNG_COLOR_MASK_ALPHA)
            && !png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
            png_set_filler(png_ptr, 0xff, QSysInfo::ByteOrder == QSysInfo::BigEndian ?
                           PNG_FILLER_BEFORE : PNG_FILLER_AFTER);
            // We want 4 bytes, but it isn't an alpha channel
            format = QImage::Format_RGB32;
        }
        if (image.size() != QSize(width, height) || image.format() != format) {
            image = QImage(width, height, format);
            if (image.isNull())
                return;
        }

        if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
            png_set_swap_alpha(png_ptr);

        png_read_update_info(png_ptr, info_ptr);
    }

    // Qt==ARGB==Big(ARGB)==Little(BGRA)
    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        png_set_bgr(png_ptr);
    }
}

QAPngHandlerPrivate::QAPngHandlerPrivate(QAPngHandler* qq)
: q(qq), readDone(false), gamma(0.0),
png_ptr(0), info_ptr(0), end_info(0), row_pointers(0),
isAPNG(false), frameIndex(0), frameCount(0), playCount(0), nextDelay(0)
{
}

QAPngHandlerPrivate::~QAPngHandlerPrivate()
{
}

bool QAPngHandlerPrivate::readAPngHeader()
{
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!png_ptr) {
        return false;
    }

//     png_set_error_fn(png_ptr, 0, 0, qt_png_warning);

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, 0, 0);
        png_ptr = 0;
        return false;
    }

    end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        png_ptr = 0;
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        png_ptr = 0;
        return false;
    }

    png_set_read_fn(png_ptr, this, iod_read_fn);

    png_read_info(png_ptr, info_ptr);

#ifndef QT_NO_IMAGE_TEXT
    png_textp text_ptr;
    int num_text = 0;
    png_get_text(png_ptr,info_ptr, &text_ptr, &num_text);

    while (num_text--) {
        QString key, value;
#if defined(PNG_iTXt_SUPPORTED) && !defined(QT_NO_TEXTCODEC)
        if (text_ptr->lang) {
            QTextCodec *codec = QTextCodec::codecForName(text_ptr->lang);
            if (codec) {
                key = codec->toUnicode(text_ptr->lang_key);
                value = codec->toUnicode(QByteArray(text_ptr->text, text_ptr->itxt_length));
            } else {
                key = QString::fromLatin1(text_ptr->key);
                value = QString::fromLatin1(QByteArray(text_ptr->text, int(text_ptr->text_length)));
            }
        } else
#endif
        {
            key = QString::fromLatin1(text_ptr->key);
            value = QString::fromLatin1(QByteArray(text_ptr->text, int(text_ptr->text_length)));
        }
        if (!description.isEmpty())
            description += QLatin1String("\n\n");
        description += key + QLatin1String(": ") + value.simplified();
        text_ptr++;
    }
#endif

    isAPNG = png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL);
    if (isAPNG) {
        frameCount = png_get_num_frames(png_ptr, info_ptr);
        playCount = png_get_num_plays(png_ptr, info_ptr);
    }
    else {
        frameCount = 1;
        playCount = 0;
    }

    row_pointers = 0;

    return true;
}

bool QAPngHandlerPrivate::readImage(QImage* outImage)
{
    if (!readDone) {
        bool ret = readAPngHeader();
        if (!ret)
            return false;
        readDone = true;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        delete[] row_pointers;
        png_ptr = 0;
        return false;
    }

    setup_qt(*outImage, png_ptr, info_ptr, gamma);

    if (outImage->isNull()) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        delete [] row_pointers;
        png_ptr = 0;
        return false;
    }

    png_uint_32 width;
    png_uint_32 height;
    int bit_depth;
    int color_type;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);

    uchar* data = outImage->bits();
    int bpl = outImage->bytesPerLine();
    row_pointers = new png_bytep[ height ];

    for (uint i = 0; i < height; ++i) {
        row_pointers[ i ] = data + i * bpl;
    }

    if (isAPNG) {
        png_uint_16 next_frame_delay_num;
        png_uint_16 next_frame_delay_den;

        png_read_frame_head(png_ptr, info_ptr);

        next_frame_delay_num = png_get_next_frame_delay_num(png_ptr, info_ptr);
        next_frame_delay_den = png_get_next_frame_delay_den(png_ptr, info_ptr);
        nextDelay = next_frame_delay_num * 1000 / next_frame_delay_den;

        png_read_image(png_ptr, row_pointers);

        outImage->setDotsPerMeterX(png_get_x_pixels_per_meter(png_ptr, info_ptr));
        outImage->setDotsPerMeterY(png_get_y_pixels_per_meter(png_ptr, info_ptr));

        frameIndex++;
        if (frameIndex == frameCount) {
            frameIndex = 0;

            png_read_end(png_ptr, info_ptr);

            png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

            readDone = false;

            /// rewind
            q->device()->seek(0);
        }

    }
    else {
        png_read_image(png_ptr, row_pointers);

        outImage->setDotsPerMeterX(png_get_x_pixels_per_meter(png_ptr, info_ptr));
        outImage->setDotsPerMeterY(png_get_y_pixels_per_meter(png_ptr, info_ptr));

        png_read_end(png_ptr, info_ptr);

        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

        readDone = false;
    }

    delete[] row_pointers;
    row_pointers = 0;

    return true;
}

QImage::Format QAPngHandlerPrivate::readImageFormat()
{
    QImage::Format format = QImage::Format_Invalid;
    png_uint_32 width, height;
    int bit_depth, color_type;
    png_colorp palette;
    int num_palette;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);
    if (color_type == PNG_COLOR_TYPE_GRAY) {
        // Black & White or 8-bit grayscale
        if (bit_depth == 1 && png_get_channels(png_ptr, info_ptr) == 1) {
            format = QImage::Format_Mono;
        }
        else if (bit_depth == 16 && png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
            format = QImage::Format_ARGB32;
        }
        else {
            format = QImage::Format_Indexed8;
        }
    }
    else if (color_type == PNG_COLOR_TYPE_PALETTE
                && png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette)
                && num_palette <= 256) {
        // 1-bit and 8-bit color
        if (bit_depth != 1)
            png_set_packing(png_ptr);
        png_read_update_info(png_ptr, info_ptr);
        png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);
        format = bit_depth == 1 ? QImage::Format_Mono : QImage::Format_Indexed8;
    }
    else {
        // 32-bit
        if (bit_depth == 16)
            png_set_strip_16(png_ptr);

        format = QImage::Format_ARGB32;
        // Only add filler if no alpha, or we can get 5 channel data.
        if (!(color_type & PNG_COLOR_MASK_ALPHA)
            && !png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
            // We want 4 bytes, but it isn't an alpha channel
            format = QImage::Format_RGB32;
        }
    }

    return format;
}

int QAPngHandlerPrivate::currentImageNumber() const
{
    return frameIndex;
}

int QAPngHandlerPrivate::imageCount() const
{
    return frameCount;
}

int QAPngHandlerPrivate::loopCount() const
{
    return playCount;
}

int QAPngHandlerPrivate::nextImageDelay() const
{
    return nextDelay;
}

QAPngHandler::QAPngHandler()
: d(new QAPngHandlerPrivate(this))
{
}

QAPngHandler::~QAPngHandler()
{
    delete d;
}

bool QAPngHandler::canRead() const
{
//     qWarning() << "QAPngHandler::canRead";
    if (d->readDone && d->frameIndex < d->frameCount)
        return true;
    return canRead(device());
}

QByteArray QAPngHandler::name() const
{
    return "apng";
}

bool QAPngHandler::read(QImage* image)
{
//     qWarning() << "QAPngHandler::read";
    if (!canRead())
        return false;
    return d->readImage(image);
}

int QAPngHandler::currentImageNumber() const
{
    return d->currentImageNumber();
}

int QAPngHandler::imageCount() const
{
    return d->imageCount();
}

int QAPngHandler::loopCount() const
{
    return d->loopCount();
}

int QAPngHandler::nextImageDelay() const
{
    return d->nextImageDelay();
}

bool QAPngHandler::canRead(QIODevice* device)
{
    if (!device) {
        qWarning("QAPngHandler::canRead() called with no device");
        return false;
    }

    return device->peek(8) == "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A";
}

QVariant QAPngHandler::option(ImageOption option) const
{
//     qWarning() << "option";
    if (option == Animation)
        return true;
    if (option == Gamma)
        return d->gamma;
    if (option == Description)
        return d->description;
    if (option == Size)
        return QSize(png_get_image_width(d->png_ptr, d->info_ptr),
                     png_get_image_height(d->png_ptr, d->info_ptr));
    if (option == ImageFormat)
        return d->readImageFormat();
    return QVariant();
}

void QAPngHandler::setOption(ImageOption option, const QVariant& value)
{
    if (option == Gamma)
        d->gamma = value.toFloat();
    else if (option == Description)
        d->description = value.toString();
}

bool QAPngHandler::supportsOption(ImageOption option) const
{
//     qWarning() << "supportsOption";
    return option == Animation
        || option == Gamma
        || option == Description
        || option == ImageFormat
        || option == Size;
}

class QAPngPlugin : public QImageIOPlugin
{
public:
    virtual QStringList keys() const;
    virtual Capabilities capabilities(QIODevice* device, const QByteArray& format) const;
    virtual QImageIOHandler* create(QIODevice* device, const QByteArray& format = QByteArray()) const;
};

QStringList QAPngPlugin::keys() const
{
    return QStringList() << "apng";
}

QImageIOPlugin::Capabilities QAPngPlugin::capabilities(QIODevice* device, const QByteArray& format) const
{
    if (!device)
        return 0;
    if (!device->isOpen() || !device->isReadable())
        return 0;

    if (format == "apng" && QAPngHandler::canRead(device))
        return CanRead;

    return 0;
}

QImageIOHandler* QAPngPlugin::create(QIODevice* device, const QByteArray& format) const
{
    QAPngHandler* handler = new QAPngHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

Q_EXPORT_STATIC_PLUGIN(QAPngPlugin)
Q_EXPORT_PLUGIN2(qapng, QAPngPlugin)
