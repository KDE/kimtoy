/*
 *  This file is part of KIMToy, an input method frontend for KDE
 *  Copyright (C) 2011-2012 Ni Hui <shuizhuyuanluo@126.com>
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

#ifndef THEMER_SOGOU_H
#define THEMER_SOGOU_H

#include "propertywidget.h"
#include "skinpixmap.h"
#include "themer.h"

#include <QHash>

#include <QMovie>
#include <QBuffer>

class OverlayPixmap : public QMovie
{
public:
    /**
     * overlay layout
     *
     *  lt|                t              |rt
     * ---+===============================+---
     *    |      ##### preedit #####      |
     *   l|-------------center------------|r
     *    |      #### candidate ####      |
     * ---+===============================+---
     *  lb|                b              |rb
     *
     */
    /// TODO: only entire window is supported atm --- nihui
    int alignTarget;// 0->entire window, 1->preedit window, 2->candidate window
    int alignArea;// 1->lt, 2->t, 3->rt, 4->l, 5->center, 6->r, 7->lb, 8->b, 9->rb
    int alignHMode;// 0->align center, 1->align left, 2->align right
    int alignVMode;// 0->align center, 1->align top, 2->align bottom
    int mt, mb, ml, mr;// margins
};

class ThemerSogou : public Themer
{
public:
    static ThemerSogou* self();
    virtual ~ThemerSogou();
    virtual bool loadTheme();
    virtual QSize sizeHintPreEditBar(const PreEditBar* widget) const;
    virtual QSize sizeHintStatusBar(const StatusBar* widget) const;
    virtual void layoutStatusBar(StatusBarLayout* layout) const;
    virtual void resizePreEditBar(const QSize& size);
    virtual void resizeStatusBar(const QSize& size);
    virtual void maskPreEditBar(PreEditBar* widget);
    virtual void maskStatusBar(StatusBar* widget);
    virtual void blurPreEditBar(PreEditBar* widget);
    virtual void blurStatusBar(StatusBar* widget);
    virtual void drawPreEditBar(PreEditBar* widget);
    virtual void drawStatusBar(StatusBar* widget);
    virtual void drawPropertyWidget(PropertyWidget* widget);
private:
    void updatePreEditBarMask(const QSize& size);
    void updateStatusBarMask(const QSize& size);

    /**
     * preedit bar layout
     * +===============================+
     * |              ^|pt             |
     * |<-pl->##### preedit #####<-pr->|
     * |              ^|pb             |
     * +-------------------------------+
     * |              ^|zt             |
     * |<-zl->#### candidate ####<-zr->|
     * |              ^|zb             |
     * +===============================+
     */
    // prefix h_->horizontal mode, v_->vertical mode
    SkinPixmap h_preEditBarSkin;
    SkinPixmap v_preEditBarSkin;
    int h_pt, h_pb, h_pl, h_pr;
    int v_pt, v_pb, v_pl, v_pr;
    int h_zt, h_zb, h_zl, h_zr;
    int v_zt, v_zb, v_zl, v_zr;

    /// optional
    QHash<QString, OverlayPixmap*> h_overlays;// horizontal overlay pixmap
    QHash<QString, OverlayPixmap*> v_overlays;// vertical overlay pixmap
    int h_opt, h_opb, h_opl, h_opr;
    int v_opt, v_opb, v_opl, v_opr;

    /// optional
    QColor h_separatorColor;
    QColor v_separatorColor;
    int h_sepl, h_sepr;
    int v_sepl, v_sepr;

    /**
     * status bar layout
     * +========================+
     * |                        |
     * | +p1     +p2     +p3    |
     * |                        |
     * | +p4     +p5     +p6    |
     * |                        |
     * +========================+
     * |                        |
     * |<------skin width------>|
     */
//     QPixmap m_statusBarSkin;
    QMovie* m_statusBarSkin;
    QHash<QString, OverlayPixmap*> s_overlays;

    QHash<PropertyType, QPoint> m_pwpos;
    QHash<PropertyType, QPixmap> m_pwpix;
    QList<QPoint> m_otherpos;

    QRegion m_preEditBarMask;
    QRegion m_statusBarMask;

    explicit ThemerSogou();
    static ThemerSogou* m_self;
};

#endif // THEMER_SOGOU_H
