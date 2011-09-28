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

#ifndef SKINPIXMAP_H
#define SKINPIXMAP_H

#include <QPixmap>
#include <QRegion>

class SkinPixmap
{
    public:
        explicit SkinPixmap();
        explicit SkinPixmap( const QPixmap& skinpix, int hsl, int hsr, int vst, int vsb, int hstm, int vstm );
        int skinw() const { return m_skinw; }
        int skinh() const { return m_skinh; }
        void resizePixmap( const QSize& size );
        void resizeRegion( const QSize& size );
        void drawPixmap( QPainter* p, int width, int height ) const;
        QRegion currentRegion() const;
    private:
        int m_skinw, m_skinh;
        int m_hsl, m_hsr, m_vst, m_vsb;
        int m_hstm, m_vstm;// stretch mode, 0->scale, 1->repeat
        /// original pixmaps
        QPixmap o_topleft,    o_top,        o_topright;
        QPixmap o_left,       o_center,     o_right;
        QPixmap o_bottomleft, o_bottom,     o_bottomright;
        /// scaled pixmaps
        QPixmap m_topleft,    m_top,        m_topright;
        QPixmap m_left,       m_center,     m_right;
        QPixmap m_bottomleft, m_bottom,     m_bottomright;
        QRegion m_topleftRegion, m_topRegion, m_toprightRegion;
        QRegion m_leftRegion, m_centerRegion, m_rightRegion;
        QRegion m_bottomleftRegion, m_bottomRegion, m_bottomrightRegion;
        QRegion m_currentRegion;
};

#endif // SKINPIXMAP_H
