/*
 *  This file is part of kde-thumbnailer-ssf
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

#include "ssfcreator.h"

#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QColor>
#include <QFile>
#include <QString>
#include <QTextStream>
#include <KZip>
#include <KZipFileEntry>
#include <kdemacros.h>

extern "C" {
    KDE_EXPORT ThumbCreator* new_creator() {
        return new SsfCreator;
    }
}

SsfCreator::SsfCreator()
{
}

SsfCreator::~SsfCreator()
{
}

bool SsfCreator::create( const QString& path, int width, int height, QImage& img )
{
    if ( !QFile::exists( path ) )
        return false;

    KZip zip( path );
    if ( !zip.open( QIODevice::ReadOnly ) )
        return false;

    const KArchiveEntry* entry = zip.directory()->entry( "skin.ini" );
    const KZipFileEntry* skinini = static_cast<const KZipFileEntry*>(entry);

    if ( !skinini )
        return false;

    QFont preEditFont;
    QFont candidateFont;
    QColor preEditColor;
    QColor labelColor;
    QColor candidateColor;

    int pt = 0, pb = 0, pl = 0, pr = 0;
    int zt = 0, zb = 0, zl = 0, zr = 0;

    QByteArray data = skinini->data();

    /// parse ini file content
    bool display = false;
    bool scheme_h1 = false;
    QPixmap skin;
    int hstm = 0;
    int sl = 0, sr = 0;
    int fontPixelSize = 12;
    QString font_ch, font_en;
    QString color_ch, color_en;

    QTextStream ss( data );
    QString line;
    QString key, value;
    do {
        line = ss.readLine();
        if ( line.isEmpty() )
            continue;

        if ( line.at( 0 ) == '[' ) {
            display = ( line == "[Display]" );
            scheme_h1 = ( line == "[Scheme_H1]" );
            continue;
        }

        key = line.split( '=' ).at( 0 );
        value = line.split( '=' ).at( 1 );

        if ( display ) {
            if ( key == "font_size" )
                fontPixelSize = value.trimmed().toInt();
            else if ( key == "font_ch" )
                font_ch = value;
            else if ( key == "font_en" )
                font_en = value;
            else if ( key == "pinyin_color" )
                color_en = value;
            else if ( key == "zhongwen_color" )
                color_ch = value;
        }
        else if ( scheme_h1 ) {
            if ( key == "pic" ) {
                const KArchiveEntry* e = zip.directory()->entry( value );
                const KZipFileEntry* pix = static_cast<const KZipFileEntry*>(e);
                if ( pix )
                    skin.loadFromData( pix->data() );
            }
            else if ( key == "layout_horizontal" ) {
                hstm = value.split( ',' ).at( 0 ).trimmed().toInt();
                sl = value.split( ',' ).at( 1 ).trimmed().toInt();
                sr = value.split( ',' ).at( 2 ).trimmed().toInt();
            }
            else if ( key == "pinyin_marge" ) {
                QStringList list = value.split( ',' );
                pt = list.at( 0 ).trimmed().toInt();
                pb = list.at( 1 ).trimmed().toInt();
                pl = list.at( 2 ).trimmed().toInt();
                pr = list.at( 3 ).trimmed().toInt();
            }
            else if ( key == "zhongwen_marge" ) {
                QStringList list = value.split( ',' );
                zt = list.at( 0 ).trimmed().toInt();
                zb = list.at( 1 ).trimmed().toInt();
                zl = list.at( 2 ).trimmed().toInt();
                zr = list.at( 3 ).trimmed().toInt();
            }
        }
    } while ( !line.isNull() );

    preEditFont.setFamily( font_en );
    preEditFont.setPixelSize( fontPixelSize );
    preEditFont.setBold( true );
    candidateFont.setFamily( font_ch );
    candidateFont.setPixelSize( fontPixelSize );
    candidateFont.setBold( true );

    color_en = color_en.leftJustified( 8, '0' ).replace( "0x", "#" );
    color_ch = color_ch.leftJustified( 8, '0' ).replace( "0x", "#" );
    preEditColor = QColor( color_en );
    candidateColor = QColor( color_ch );
    labelColor = candidateColor;

    int pinyinh = QFontMetrics( preEditFont ).height();
    int zhongwenh = QFontMetrics( candidateFont ).height();
    int pinyinw = QFontMetrics( preEditFont ).width( "ABC pinyin" );
    int zhongwenw = QFontMetrics( candidateFont ).width( "1candidate" );

    /// save target size
    int targetHeight = height;
    int targetWidth = width;

    height = skin.height();
    width = qMax( pl + pinyinw + pr, zl + zhongwenw + zr );
    width = qMax( width, targetWidth );
    width = qMax( width, skin.width() );

    QPixmap pixmap( width, height );
    pixmap.fill( Qt::transparent );

    QPainter p( &pixmap );

    /// left right
    p.drawPixmap( 0, 0, sl, height, skin, 0, 0, sl, skin.height() );
    p.drawPixmap( width - sr, 0, sr, height, skin, skin.width() - sr, 0, sr, skin.height() );

    /// middle
    QPixmap middlepix = skin.copy( sl, 0, skin.width() - sl - sr, skin.height() );
    if ( hstm == 0 )
        p.drawPixmap( sl, 0, middlepix.scaled( width - sl - sr, height ) );
    else
        p.drawTiledPixmap( sl, 0, width - sl - sr, height, middlepix );

    int y = 0;

    /// draw preedit / aux text
    p.save();
    p.setFont( preEditFont );
    p.setPen( preEditColor );
    y += pt;
    p.drawText( pl, pt, width - pl - pr, pinyinh, Qt::AlignLeft, "ABC pinyin" );
    int pixelsWide = QFontMetrics( preEditFont ).width( "ABC pinyin" );
    p.drawLine( pl + pixelsWide, pt, pl + pixelsWide, pt + pinyinh );
    y += pinyinh + pb;
    p.restore();

    /// draw lookup table
    p.save();
    int x = zl;
    y += zt;
    int w = 0;
    p.setFont( candidateFont );
    /// draw label
    p.setPen( labelColor );
    w = p.fontMetrics().width( "1" );
    p.drawText( x, y, w, zhongwenh, Qt::AlignCenter, "1" );
    x += w;
    /// draw candidate
    p.setPen( candidateColor );
    w = p.fontMetrics().width( "candidate" );
    p.drawText( x, y, w, zhongwenh, Qt::AlignCenter, "candidate" );
    x += w;
    p.restore();

    if ( targetWidth < width || targetHeight < height ) {
        pixmap = pixmap.scaled( targetWidth, targetHeight, Qt::KeepAspectRatio );
    }

    img = pixmap.toImage();

    return true;
}
