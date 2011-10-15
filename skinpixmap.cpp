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

#include "skinpixmap.h"

#include <QBitmap>
#include <QPainter>
#include <QSize>

/**
 *          |         |          |
 * ---------+---------+---------vst
 *          |         | ^vstm    |
 * ---------+---------+---------vsb
 *          |<--hstm->|          |
 * --------hsl-------hsr-------skinw-----skinh
 */

SkinPixmap::SkinPixmap()
{
    m_skinw = 0;
    m_skinh = 0;
}

SkinPixmap::SkinPixmap(const QPixmap& skinpix, int hsl, int hsr, int vst, int vsb, int hstm, int vstm)
{
    m_skinw = skinpix.width();
    m_skinh = skinpix.height();
    m_hsl = hsl, m_hsr = hsr;
    m_vst = vst, m_vsb = vsb;
    m_hstm = hstm, m_vstm = vstm;
    o_topleft = skinpix.copy(0, 0, hsl, vst);
    o_top = skinpix.copy(hsl, 0, hsr - hsl, vst);
    o_topright = skinpix.copy(hsr, 0, m_skinw - hsr, vst);
    o_left = skinpix.copy(0, vst, hsl, vsb - vst);
    o_center = skinpix.copy(hsl, vst, hsr - hsl, vsb - vst);
    o_right = skinpix.copy(hsr, vst, m_skinw - hsr, vsb - vst);
    o_bottomleft = skinpix.copy(0, vsb, hsl, m_skinh - vsb);
    o_bottom = skinpix.copy(hsl, vsb, hsr - hsl, m_skinh - vsb);
    o_bottomright = skinpix.copy(hsr, vsb, m_skinw - hsr, m_skinh - vsb);
    m_topleft = o_topleft;
    m_top = o_top;
    m_topright = o_topright;
    m_left = o_left;
    m_center = o_center;
    m_right = o_right;
    m_bottomleft = o_bottomleft;
    m_bottom = o_bottom;
    m_bottomright = o_bottomright;
    m_topleftRegion = m_topleft.mask();
    m_topRegion = m_top.mask();
    m_toprightRegion = m_topright.mask();
    m_leftRegion = m_left.mask();
    m_centerRegion = m_center.mask();
    m_rightRegion = m_right.mask();
    m_bottomleftRegion = m_bottomleft.mask();
    m_bottomRegion = m_bottom.mask();
    m_bottomrightRegion = m_bottomright.mask();
}

void SkinPixmap::resizePixmap(const QSize& size)
{
    const int leftrightheight = size.height() - m_topleft.height() - m_bottomleft.height();
    const int topbottomwidth = size.width() - m_topleft.width() - m_topright.width();

    /// corners

    /// edges
    if (m_hstm == 0) {
        /// scale
        if (m_top.width() != topbottomwidth) {
            m_top = o_top.scaled(topbottomwidth, m_top.height());
            m_bottom = o_bottom.scaled(topbottomwidth, m_bottom.height());
            m_topRegion = m_top.mask();
            m_bottomRegion = m_bottom.mask();
        }
    }
    if (m_vstm == 0) {
        /// scale
        if (m_left.height() != leftrightheight) {
            m_left = o_left.scaled(m_left.width(), leftrightheight);
            m_right = o_right.scaled(m_right.width(), leftrightheight);
            m_leftRegion = m_left.mask();
            m_rightRegion = m_right.mask();
        }
    }

    /// center
    if (m_hstm == 0) {
        /// scale
        if (m_vstm == 0) {
            /// scale
            if (m_center.width() != topbottomwidth || m_center.height() != leftrightheight) {
                m_center = o_center.scaled(topbottomwidth, leftrightheight);
                m_centerRegion = m_center.mask();
            }
        }
        else {
            /// tilling
            if (m_center.width() != topbottomwidth) {
                m_center = o_center.scaled(topbottomwidth, m_center.height());
                m_centerRegion = m_center.mask();
            }
        }
    }
    else {
        /// tilling
        if (m_vstm == 0) {
            /// scale
            if (m_center.height() != leftrightheight) {
                m_center = o_center.scaled(m_center.width(), leftrightheight);
                m_centerRegion = m_center.mask();
            }
        }
    }
}

void SkinPixmap::resizeRegion(const QSize& size)
{
    const int middlepixh = m_vsb - m_vst;
    const int middlepixw = m_hsr - m_hsl;

    const int leftrightheight = size.height() - m_topleft.height() - m_bottomleft.height();
    const int topbottomwidth = size.width() - m_topleft.width() - m_topright.width();

    /// corners
    QRegion topleft = m_topleftRegion;
    QRegion topright = m_toprightRegion;
    topright.translate(m_hsl + topbottomwidth, 0);
    QRegion bottomleft = m_bottomleftRegion;
    bottomleft.translate(0, m_vst + leftrightheight);
    QRegion bottomright = m_bottomrightRegion;
    bottomright.translate(m_hsl + topbottomwidth, m_vst + leftrightheight);

    /// edges
    QRegion left, top, right, bottom;
    if (m_hstm == 0) {
        /// scale
        top = m_topRegion;
        bottom = m_bottomRegion;
    }
    else {
        /// tilling
        top = QRegion(QRect(0, 0, topbottomwidth, m_vst));
        bottom = QRegion(QRect(0, 0, topbottomwidth, m_bottom.height()));
        QRegion toppixRegion(m_topRegion);
        QRegion bottompixRegion(m_bottomRegion);
        QRegion tmpRegion = toppixRegion;
        QRegion tmp2Region = bottompixRegion;
        for (int i = 0; i < topbottomwidth; i += middlepixw) {
            toppixRegion |= tmpRegion;
            bottompixRegion |= tmp2Region;
            tmpRegion.translate(middlepixw, 0);
            tmp2Region.translate(middlepixw, 0);
        }
        top &= toppixRegion;
        bottom &= bottompixRegion;
    }
    if (m_vstm == 0) {
        /// scale
        left = m_leftRegion;
        right = m_rightRegion;
    }
    else {
        /// tilling
        left = QRegion(QRect(0, 0, m_hsl, leftrightheight));
        right = QRegion(QRect(0, 0, m_right.width(), leftrightheight));
        QRegion leftpixRegion(m_leftRegion);
        QRegion rightpixRegion(m_rightRegion);
        QRegion tmpRegion = leftpixRegion;
        QRegion tmp2Region = rightpixRegion;
        for (int i = 0; i < leftrightheight; i += middlepixh) {
            leftpixRegion |= tmpRegion;
            rightpixRegion |= tmp2Region;
            tmpRegion.translate(0, middlepixh);
            tmp2Region.translate(0, middlepixh);
        }
        left &= leftpixRegion;
        right &= rightpixRegion;
    }
    left.translate(0, m_vst);
    top.translate(m_hsl, 0);
    right.translate(m_hsl + topbottomwidth, m_vst);
    bottom.translate(m_hsl, m_vst + leftrightheight);

    /// center
    QRegion center(QRect(0, 0, topbottomwidth, leftrightheight));
    if (m_hstm == 0) {
        /// scale
        if (m_vstm == 0) {
            /// scale
            center = m_centerRegion;
        }
        else {
            /// tilling
            QRegion centerpixRegion(m_centerRegion);
            QRegion tmpRegion = centerpixRegion;
            int i;
            for (i = 0; i < leftrightheight; i += middlepixh) {
                centerpixRegion |= tmpRegion;
                tmpRegion.translate(0, middlepixh);
            }
            center &= centerpixRegion;
        }
    }
    else {
        /// tilling
        if (m_vstm == 0) {
            /// scale
            QRegion centerpixRegion(m_centerRegion);
            QRegion tmpRegion = centerpixRegion;
            int i;
            for (i = 0; i < topbottomwidth; i += middlepixw) {
                centerpixRegion |= tmpRegion;
                tmpRegion.translate(middlepixw, 0);
            }
            center &= centerpixRegion;
        }
        else {
            /// tilling
            QRegion centerpixRegion(m_centerRegion);
            QRegion tmpRegion = centerpixRegion;
            int i, j;
            for (i = 0; i < topbottomwidth; i += middlepixw) {
                QRegion tmp2Region = tmpRegion;
                for (j = 0; j < leftrightheight; j += middlepixh) {
                    centerpixRegion |= tmp2Region;
                    tmp2Region.translate(0, middlepixh);
                }
                tmpRegion.translate(middlepixw, 0);
            }
            center &= centerpixRegion;
        }
    }
    center.translate(m_hsl, m_vst);

    m_currentRegion = topleft | top | topright | left | center | right | bottomleft | bottom | bottomright;
}

void SkinPixmap::drawPixmap(QPainter* p, int width, int height) const
{
    const int leftrightheight = height - m_topleft.height() - m_bottomleft.height();
    const int topbottomwidth = width - m_topleft.width() - m_topright.width();

    /// corners
    p->drawPixmap(0, 0, m_topleft);
    p->drawPixmap(m_hsl + topbottomwidth, 0, m_topright);
    p->drawPixmap(0, m_vst + leftrightheight, m_bottomleft);
    p->drawPixmap(m_hsl + topbottomwidth, m_vst + leftrightheight, m_bottomright);

    /// edges
    if (m_hstm == 0) {
        /// scale
        p->drawPixmap(m_hsl, 0, m_top);
        p->drawPixmap(m_hsl, m_vst + leftrightheight, m_bottom);
    }
    else {
        /// tilling
        p->drawTiledPixmap(m_hsl, 0, topbottomwidth, m_vst, m_top);
        p->drawTiledPixmap(m_hsl, m_vst + leftrightheight, topbottomwidth, m_bottom.height(), m_bottom);
    }
    if (m_vstm == 0) {
        /// scale
        p->drawPixmap(0, m_vst, m_left);
        p->drawPixmap(m_hsl + topbottomwidth, m_vst, m_right);
    }
    else {
        /// tilling
        p->drawTiledPixmap(0, m_vst, m_hsl, leftrightheight, m_left);
        p->drawTiledPixmap(m_hsl + topbottomwidth, m_vst, m_right.width(), leftrightheight, m_right);
    }

    /// center
    if (m_hstm == 0 && m_vstm == 0) {
        /// scale
        p->drawPixmap(m_hsl, m_vst, m_center);
    }
    else {
        /// tilling
        p->drawTiledPixmap(m_hsl, m_vst, topbottomwidth, leftrightheight, m_center);
    }
}

QRegion SkinPixmap::currentRegion() const
{
    return m_currentRegion;
}
