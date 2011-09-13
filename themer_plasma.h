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

#ifndef THEMER_PLASMA_H
#define THEMER_PLASMA_H

#include <Plasma/FrameSvg>

#include "themer.h"

class ThemerPlasma : public Themer
{
    public:
        static ThemerPlasma* self();
        virtual ~ThemerPlasma();
        virtual bool loadTheme();
        virtual QSize sizeHintPreEditBar( const PreEditBar* widget ) const;
        virtual QSize sizeHintStatusBar( const StatusBar* widget ) const;
        virtual void layoutStatusBar( StatusBarLayout* layout ) const;
        virtual void resizePreEditBar( const QSize& size );
        virtual void resizeStatusBar( const QSize& size );
        virtual void maskPreEditBar( PreEditBar* widget );
        virtual void maskStatusBar( StatusBar* widget );
        virtual void blurPreEditBar( PreEditBar* widget );
        virtual void blurStatusBar( StatusBar* widget );
        virtual void drawPreEditBar( PreEditBar* widget );
        virtual void drawStatusBar( StatusBar* widget );
        virtual void drawPropertyWidget( PropertyWidget* widget );
    private:
        Plasma::FrameSvg m_statusBarSvg;
        Plasma::FrameSvg m_preeditBarSvg;

        explicit ThemerPlasma();
        static ThemerPlasma* m_self;
};

#endif // THEMER_PLASMA_H
