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

#include "themer.h"

#include <Plasma/WindowEffects>

#include "preeditbar.h"
#include "statusbar.h"

#include "kimtoysettings.h"

Themer::Themer()
{
}

Themer::~Themer()
{
}

void Themer::loadSettings()
{
    if (KIMToySettings::self()->useCustomFont()) {
        m_preEditFont = KIMToySettings::self()->preeditFont();
        m_labelFont = KIMToySettings::self()->labelFont();
        m_candidateFont = KIMToySettings::self()->candidateFont();

        m_preEditFontHeight = QFontMetrics(m_preEditFont).height();
        m_labelFontHeight = QFontMetrics(m_labelFont).height();
        m_candidateFontHeight = QFontMetrics(m_candidateFont).height();
    }
    if (KIMToySettings::self()->useCustomColor()) {
        m_preEditColor = KIMToySettings::self()->preeditColor();
        m_labelColor = KIMToySettings::self()->labelColor();
        m_candidateColor = KIMToySettings::self()->candidateColor();
        m_firstCandidateColor = KIMToySettings::self()->firstCandidateColor();
    }
}

QPoint Themer::anchorPos() const
{
    return QPoint(0, 0);
}

void Themer::resizePreEditBar(const QSize& size)
{
    Q_UNUSED(size);
}

void Themer::resizeStatusBar(const QSize& size)
{
    Q_UNUSED(size);
}

void Themer::blurPreEditBar(PreEditBar* widget)
{
    Plasma::WindowEffects::enableBlurBehind(widget->winId(), true, widget->mask());
}

void Themer::blurStatusBar(StatusBar* widget)
{
    Plasma::WindowEffects::enableBlurBehind(widget->winId(), true, widget->mask());
}
