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
#include <QMatrix>
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

    m_topleftRegion = o_topleft.mask();
    m_topRegion = o_top.mask();
    m_toprightRegion = o_topright.mask();
    m_leftRegion = o_left.mask();
    m_centerRegion = o_center.mask();
    m_rightRegion = o_right.mask();
    m_bottomleftRegion = o_bottomleft.mask();
    m_bottomRegion = o_bottom.mask();
    m_bottomrightRegion = o_bottomright.mask();
}

void SkinPixmap::resizeRegion(const QSize& size)
{
    const int middlepixh = m_vsb - m_vst;
    const int middlepixw = m_hsr - m_hsl;

    const int leftrightheight = size.height() - o_topleft.height() - o_bottomleft.height();
    const int topbottomwidth = size.width() - o_topleft.width() - o_topright.width();

    qreal scaleX = m_hstm ? 1 : (qreal)topbottomwidth / (qreal)middlepixw;
    qreal scaleY = m_vstm ? 1 : (qreal)leftrightheight / (qreal)middlepixh;

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
        QMatrix scaleXMatrix;
        scaleXMatrix.scale(scaleX, 1.0);
        top = m_topRegion * scaleXMatrix;
        bottom = m_bottomRegion * scaleXMatrix;
    }
    else {
        /// tilling
        top = QRegion(QRect(0, 0, topbottomwidth, m_vst));
        bottom = QRegion(QRect(0, 0, topbottomwidth, o_bottom.height()));
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
        QMatrix scaleYMatrix;
        scaleYMatrix.scale(1.0, scaleY);
        left = m_leftRegion * scaleYMatrix;
        right = m_rightRegion * scaleYMatrix;
    }
    else {
        /// tilling
        left = QRegion(QRect(0, 0, m_hsl, leftrightheight));
        right = QRegion(QRect(0, 0, o_right.width(), leftrightheight));
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
            QMatrix scaleXYMatrix;
            scaleXYMatrix.scale(scaleX, scaleY);
            center = m_centerRegion * scaleXYMatrix;
        }
        else {
            /// tilling
            QMatrix scaleXMatrix;
            scaleXMatrix.scale(scaleX, 1.0);
            QRegion centerpixRegion(m_centerRegion * scaleXMatrix);
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
            QMatrix scaleYMatrix;
            scaleYMatrix.scale(1.0, scaleY);
            QRegion centerpixRegion(m_centerRegion * scaleYMatrix);
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
    const int middlepixh = m_vsb - m_vst;
    const int middlepixw = m_hsr - m_hsl;

    const int leftrightheight = height - o_topleft.height() - o_bottomleft.height();
    const int topbottomwidth = width - o_topleft.width() - o_topright.width();

    qreal scaleX = m_hstm ? 1 : (qreal)topbottomwidth / (qreal)middlepixw;
    qreal scaleY = m_vstm ? 1 : (qreal)leftrightheight / (qreal)middlepixh;

    /// corners
    p->drawPixmap(0, 0, o_topleft);
    p->drawPixmap(m_hsl + topbottomwidth, 0, o_topright);
    p->drawPixmap(0, m_vst + leftrightheight, o_bottomleft);
    p->drawPixmap(m_hsl + topbottomwidth, m_vst + leftrightheight, o_bottomright);

    /// edges
    if (m_hstm == 0) {
        /// scale
        p->save();
        p->translate(m_hsl, 0);
        p->scale(scaleX, 1.0);
        p->drawPixmap(0, 0, o_top);
        p->restore();
        p->save();
        p->translate(m_hsl, m_vst + leftrightheight);
        p->scale(scaleX, 1.0);
        p->drawPixmap(0, 0, o_bottom);
        p->restore();
    }
    else {
        /// tilling
        p->drawTiledPixmap(m_hsl, 0, topbottomwidth, m_vst, o_top);
        p->drawTiledPixmap(m_hsl, m_vst + leftrightheight, topbottomwidth, o_bottom.height(), o_bottom);
    }
    if (m_vstm == 0) {
        /// scale
        p->save();
        p->translate(0, m_vst);
        p->scale(1.0, scaleY);
        p->drawPixmap(0, 0, o_left);
        p->restore();
        p->save();
        p->translate(m_hsl + topbottomwidth, m_vst);
        p->scale(1.0, scaleY);
        p->drawPixmap(0, 0, o_right);
        p->restore();
    }
    else {
        /// tilling
        p->drawTiledPixmap(0, m_vst, m_hsl, leftrightheight, o_left);
        p->drawTiledPixmap(m_hsl + topbottomwidth, m_vst, o_right.width(), leftrightheight, o_right);
    }

    /// center
    if (m_hstm == 0) {
        /// scale
        if (m_vstm == 0) {
            /// scale
            p->save();
            p->translate(m_hsl, m_vst);
            p->scale(scaleX, scaleY);
            p->drawPixmap(0, 0, o_center);
            p->restore();
        }
        else {
            /// tilling
            QPixmap tmp(topbottomwidth, o_center.height());
            QPainter ptmp(&tmp);
            ptmp.scale(scaleX, 1.0);
            ptmp.drawPixmap(0, 0, o_center);
            p->drawTiledPixmap(m_hsl, m_vst, topbottomwidth, leftrightheight, tmp);
        }
    }
    else {
        /// tilling
        if (m_vstm == 0) {
            /// scale
            QPixmap tmp(o_center.width(), leftrightheight);
            QPainter ptmp(&tmp);
            ptmp.scale(1.0, scaleY);
            ptmp.drawPixmap(0, 0, o_center);
            p->drawTiledPixmap(m_hsl, m_vst, topbottomwidth, leftrightheight, tmp);
        }
        else {
            /// tilling
            p->drawTiledPixmap(m_hsl, m_vst, topbottomwidth, leftrightheight, o_center);
        }
    }
}

QRegion SkinPixmap::currentRegion() const
{
    return m_currentRegion;
}
