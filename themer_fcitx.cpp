/*
 *  This file is part of KIMToy, an input method frontend for KDE
 *  Copyright (C) 2011 Ni Hui <shuizhuyuanluo@126.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your oh_ption) version 3 or any later version
 *  acceh_pted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a h_proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the imh_plied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "themer_fcitx.h"

#include <QBitmap>
#include <QColor>
#include <QFile>
#include <QFontMetrics>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QTextStream>

#include <KDebug>
#include <KIconLoader>
#include <KTar>
#include <Plasma/WindowEffects>

#include "preeditbar.h"
#include "statusbar.h"
#include "statusbarlayout.h"
#include "propertywidget.h"

#include "kimtoysettings.h"

static QColor value2color( const QString& value )
{
    QStringList list = value.split( ' ' );
    int r = list.at( 0 ).toInt();
    int g = list.at( 1 ).toInt();
    int b = list.at( 2 ).toInt();
    return QColor( r, g, b );
}

ThemerFcitx* ThemerFcitx::m_self = 0;

ThemerFcitx* ThemerFcitx::self()
{
    if ( !m_self )
        m_self = new ThemerFcitx;
    return m_self;
}

ThemerFcitx::ThemerFcitx()
: Themer()
{
}

ThemerFcitx::~ThemerFcitx()
{
}

bool ThemerFcitx::loadTheme()
{
    QString file = KIMToySettings::self()->importTheme().path();
    if ( !QFile::exists( file ) )
        return false;

    KTar tar( file );
    if ( !tar.open( QIODevice::ReadOnly ) )
        return false;

    QStringList entries = tar.directory()->entries();
    if ( entries.count() != 1 )
        return false;

    const KArchiveEntry* entry = tar.directory()->entry( entries.first() );
    if ( !entry->isDirectory() )
        return false;

    const KArchiveDirectory* subdir = static_cast<const KArchiveDirectory*>(entry);
    const KArchiveEntry* skinconf_entry = subdir->entry( "fcitx_skin.conf" );
    const KArchiveFile* skinconf = static_cast<const KArchiveFile*>(skinconf_entry);
    if ( !skinconf )
        return false;

    QByteArray data = skinconf->data();

    /// parse ini file content
    bool skinfont = false;
    bool skinmainbar = false;
    bool skininputbar = false;

    QPixmap preEditBarPixmap;
    QPixmap statusBarPixmap;
    QFont font;
    QString resizemode;

    ml = 0, mr = 0, mt = 0, mb = 0;
    sml = 0, smr = 0, smt = 0, smb = 0;
    yen = 0, ych = 0;
    xba = 0, yba = 0;
    xfa = 0, yfa = 0;

    QTextStream ss( data );
    QString line;
    QString key, value;
    do {
        line = ss.readLine();
        if ( line.isEmpty() || line.at( 0 ) == '#' )
            continue;

        if ( line.at( 0 ) == '[' ) {
            skinfont = ( line == "[SkinFont]" );
            skinmainbar = ( line == "[SkinMainBar]" );
            skininputbar = ( line == "[SkinInputBar]" );
            continue;
        }

        QString key = line.split( '=' ).at( 0 );
        QString value = line.split( '=' ).at( 1 );

        if ( value.isEmpty() )
            continue;

        if ( skinfont ) {
            if ( key == "FontSize" ) {
                font.setPixelSize( value.toInt() );
            }
            else if ( key == "InputColor" ) {
                m_preEditColor = value2color( value );
            }
            else if ( key == "IndexColor" ) {
                m_labelColor = value2color( value );
            }
            else if ( key == "FirstCandColor" ) {
                m_candidateColor = value2color( value );
            }
            else if ( key == "UserPhraseColor" ) {
                /// NOTE implement this
                QColor color_ch_user = value2color( value );
            }
            else if ( key == "OtherColor" ) {
                /// NOTE implement this
                QColor color_ch_other = value2color( value );
            }
        }
        else if ( skinmainbar ) {
            if ( key == "BackImg" ) {
                const KArchiveEntry* e = subdir->entry( value );
                const KArchiveFile* pix = static_cast<const KArchiveFile*>(e);
                if ( pix )
                    statusBarPixmap.loadFromData( pix->data() );
            }
            else if ( key == "MarginLeft" ) {
                sml = value.toInt();
            }
            else if ( key == "MarginRight" ) {
                smr = value.toInt();
            }
            else if ( key == "MarginTop" ) {
                smt = value.toInt();
            }
            else if ( key == "MarginBottom" ) {
                smb = value.toInt();
            }
        }
        else if ( skininputbar ) {
            if ( key == "BackImg" ) {
                const KArchiveEntry* e = subdir->entry( value );
                const KArchiveFile* pix = static_cast<const KArchiveFile*>(e);
                if ( pix )
                    preEditBarPixmap.loadFromData( pix->data() );
            }
            else if ( key == "Resize" ) {
                resizemode = value;
            }
            else if ( key == "MarginLeft" ) {
                ml = value.toInt();
            }
            else if ( key == "MarginRight" ) {
                mr = value.toInt();
            }
            else if ( key == "MarginTop" ) {
                mt = value.toInt();
            }
            else if ( key == "MarginBottom" ) {
                mb = value.toInt();
            }
            else if ( key == "InputPos" ) {
                yen = value.toInt();
            }
            else if ( key == "OutputPos" ) {
                ych = value.toInt();
            }
            else if ( key == "CursorColor" ) {
                color_cursor = value2color( value );
            }
            else if ( key == "BackArrow" ) {
                const KArchiveEntry* e = subdir->entry( value );
                const KArchiveFile* pix = static_cast<const KArchiveFile*>(e);
                if ( pix )
                    barrow.loadFromData( pix->data() );
            }
            else if ( key == "ForwardArrow" ) {
                const KArchiveEntry* e = subdir->entry( value );
                const KArchiveFile* pix = static_cast<const KArchiveFile*>(e);
                if ( pix )
                    farrow.loadFromData( pix->data() );
            }
            else if ( key == "BackArrowX" ) {
                xba = value.toInt();
            }
            else if ( key == "BackArrowY" ) {
                yba = value.toInt();
            }
            else if ( key == "ForwardArrowX" ) {
                xfa = value.toInt();
            }
            else if ( key == "ForwardArrowY" ) {
                yfa = value.toInt();
            }
        }
    } while ( !line.isNull() );

    preEditBarSkin = SkinPixmap( preEditBarPixmap, ml, preEditBarPixmap.width() - mr, mt, preEditBarPixmap.height() - mb, 0, 0 );
    statusBarSkin = SkinPixmap( statusBarPixmap, sml, statusBarPixmap.width() - smr, smt, statusBarPixmap.height() - smb, 0, 0 );

    font.setBold( true );

    m_preEditFont = font;
    m_candidateFont = font;
    m_labelFont = font;

    m_preEditFontHeight = QFontMetrics( m_preEditFont ).height();
    m_labelFontHeight = QFontMetrics( m_labelFont ).height();
    m_candidateFontHeight = QFontMetrics( m_candidateFont ).height();

    return true;
}

QSize ThemerFcitx::sizeHintPreEditBar( const PreEditBar* widget ) const
{
    int w = preEditBarSkin.skinw();
    int h = preEditBarSkin.skinh();

    /// preedit and aux
    int pinyinauxw = QFontMetrics( m_preEditFont ).width( widget->m_text + widget->m_auxText );
    w = qMax( pinyinauxw + ml + mr, w );

    int candidateh = mt + ych - m_candidateFontHeight + mb;
    /// lookuptable
    if ( KIMToySettings::self()->verticalPreeditBar() ) {
        int count = qMin( widget->m_labels.count(), widget->m_candidates.count() );
        for ( int i = 0; i < count; ++i ) {
            QString tmp = widget->m_labels.at( i ).trimmed() + widget->m_candidates.at( i ).trimmed();
            w = qMax( QFontMetrics( m_candidateFont ).width( tmp ) + ml + mr, w );
            candidateh += m_candidateFontHeight;
        }
    }
    else {
        QString tmp = widget->m_labels.join( QString() );
        int count = qMin( widget->m_labels.count(), widget->m_candidates.count() );
        for ( int i = 0; i < count; ++i ) {
            tmp += widget->m_labels.at( i ).trimmed() + widget->m_candidates.at( i ).trimmed();
        }
        int lookuptablew = QFontMetrics( m_candidateFont ).width( tmp );
        w = qMax( lookuptablew + ml + mr, w );
        candidateh += m_candidateFontHeight;
    }
    h = qMax( candidateh, h );

    if ( !KIMToySettings::self()->enablePreeditResizing() ) {
        /// align with skin width + 70 * x
        const int align = 70;
        w = ( ( w - 1 ) / align + 1 ) * align;
    }

    return QSize( w, h );
}

QSize ThemerFcitx::sizeHintStatusBar( const StatusBar* widget ) const
{
    int w = widget->m_layout->count() * 22;
    int h = 22;

    w += sml + smr;
    h += smt + smb;

    return QSize( w, h );
}

void ThemerFcitx::layoutStatusBar( StatusBarLayout* layout ) const
{
    int itemCount = layout->count();
    for ( int i = 0; i < itemCount; ++i ) {
        QLayoutItem* item = layout->m_items.at( i );
        item->setGeometry( QRect( QPoint( i * 22 + sml, smt ), item->maximumSize() ) );
    }
}

void ThemerFcitx::resizePreEditBar( const QSize& size )
{
    preEditBarSkin.resizePixmap( size );

    /// calculate mask if necessary
    if ( KIMToySettings::self()->enableWindowMask()
        || KIMToySettings::self()->enableBackgroundBlur()
        || KIMToySettings::self()->backgroundColorizing() ) {
        preEditBarSkin.resizeRegion( size );
    }
}

void ThemerFcitx::resizeStatusBar( const QSize& size )
{
    statusBarSkin.resizePixmap( size );

    /// calculate mask if necessary
    if ( KIMToySettings::self()->enableWindowMask()
        || KIMToySettings::self()->enableBackgroundBlur()
        || KIMToySettings::self()->backgroundColorizing() ) {
        statusBarSkin.resizeRegion( size );
    }
}

void ThemerFcitx::maskPreEditBar( PreEditBar* widget )
{
    widget->setMask( preEditBarSkin.currentRegion() );
}

void ThemerFcitx::maskStatusBar( StatusBar* widget )
{
    QRegion mask = statusBarSkin.currentRegion();
    foreach ( const QLayoutItem* item, widget->m_layout->m_items ) {
        mask |= item->geometry();
    }
    widget->setMask( mask );
}

void ThemerFcitx::blurPreEditBar( PreEditBar* widget )
{
    Plasma::WindowEffects::enableBlurBehind( widget->winId(), true, preEditBarSkin.currentRegion() );
}

void ThemerFcitx::blurStatusBar( StatusBar* widget )
{
    Plasma::WindowEffects::enableBlurBehind( widget->winId(), true, statusBarSkin.currentRegion() );
}

void ThemerFcitx::drawPreEditBar( PreEditBar* widget )
{
    QPainter p( widget );

    if ( KIMToySettings::self()->backgroundColorizing() ) {
        QPainterPath path;
        path.addRegion( preEditBarSkin.currentRegion() );
        p.fillPath( path, KIMToySettings::self()->preeditBarColorize() );
    }

    preEditBarSkin.drawPixmap( &p, widget->width(), widget->height() );

    int pinyiny = mt + yen - m_preEditFontHeight;
    int zhongweny = mt + ych - m_candidateFontHeight;

    if ( widget->preeditVisible || widget->auxVisible ) {
        /// draw preedit / aux text
        p.save();
        p.setFont( m_preEditFont );
        p.setPen( m_preEditColor );

        p.drawText( ml, pinyiny, widget->width() - ml - mr, m_preEditFontHeight, Qt::AlignLeft, widget->m_text + widget->m_auxText );
        if ( widget->preeditVisible ) {
            int pixelsWide = QFontMetrics( m_preEditFont ).width( widget->m_text.left( widget->m_cursorPos ) );
            p.drawLine( ml + pixelsWide, pinyiny, ml + pixelsWide, pinyiny + m_preEditFontHeight );
        }
        p.restore();
    }

    if ( widget->lookuptableVisible ) {
        /// draw lookup table
        int x = ml;
        int w = 0;
        int h = qMax( m_labelFontHeight, m_candidateFontHeight );

        /// draw labels and candidates
        int count = qMin( widget->m_labels.count(), widget->m_candidates.count() );

        if ( KIMToySettings::self()->verticalPreeditBar() ) {
            for ( int i = 0; i < count; ++i ) {
                /// draw label
                p.setFont( m_labelFont );
                p.setPen( m_labelColor );
                x = ml;
                w = p.fontMetrics().width( widget->m_labels.at( i ) );
                p.drawText( x, zhongweny, w, h, Qt::AlignCenter, widget->m_labels.at( i ) );
                x += w;
                /// draw candidate
                p.setFont( m_candidateFont );
                p.setPen( m_candidateColor );
                w = p.fontMetrics().width( widget->m_candidates.at( i ).trimmed() );
                p.drawText( x, zhongweny, w, h, Qt::AlignCenter, widget->m_candidates.at( i ).trimmed() );
                zhongweny += h;
            }
        }
        else {
            for ( int i = 0; i < count; ++i ) {
                /// draw label
                p.setFont( m_labelFont );
                p.setPen( m_labelColor );
                w = p.fontMetrics().width( widget->m_labels.at( i ) );
                p.drawText( x, zhongweny, w, h, Qt::AlignCenter, widget->m_labels.at( i ) );
                x += w;
                /// draw candidate
                p.setFont( m_candidateFont );
                p.setPen( m_candidateColor );
                w = p.fontMetrics().width( widget->m_candidates.at( i ).trimmed() + ' ' );
                p.drawText( x, zhongweny, w, h, Qt::AlignCenter, widget->m_candidates.at( i ).trimmed() + ' ' );
                x += w;
            }
        }
    }
}

void ThemerFcitx::drawStatusBar( StatusBar* widget )
{
    QPainter p( widget );

    if ( KIMToySettings::self()->backgroundColorizing() ) {
        QPainterPath path;
        path.addRegion( statusBarSkin.currentRegion() );
        p.fillPath( path, KIMToySettings::self()->statusBarColorize() );
    }

    statusBarSkin.drawPixmap( &p, widget->width(), widget->height() );
}

void ThemerFcitx::drawPropertyWidget( PropertyWidget* widget )
{
    QPainter p( widget );
    if ( !widget->m_iconName.isEmpty() )
        p.drawPixmap( widget->rect(), MainBarIcon( widget->m_iconName ) );
    else {
        p.setPen( m_preEditColor );
        p.drawText( widget->rect(), Qt::AlignCenter, widget->m_name );
    }
}
