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

#include "themeragent.h"

#include <QSize>

#include "themer_fcitx.h"
#include "themer_none.h"
#include "themer_plasma.h"
#include "themer_sogou.h"

#include "kimtoysettings.h"

static Themer* m_themer = 0;

void ThemerAgent::loadSettings()
{
    QString themeUri = KIMToySettings::self()->themeUri();
    if (themeUri == "__none__") {
        m_themer = ThemerNone::self();
    }
    else if (themeUri.startsWith("__plasma__")) {
        m_themer = ThemerPlasma::self();
    }
    else if (themeUri.endsWith(".fskin")) {
        m_themer = ThemerFcitx::self();
    }
    else if (themeUri.endsWith(".ssf")) {
        m_themer = ThemerSogou::self();
    }
    else {
        m_themer = ThemerNone::self();
    }
}

void ThemerAgent::loadTheme()
{
    bool success = m_themer->loadTheme();
    if (!success) {
        m_themer = ThemerNone::self();
        m_themer->loadTheme();
    }

    m_themer->loadSettings();
}

QSize ThemerAgent::sizeHintPreEditBar(const PreEditBar* widget)
{
    return m_themer->sizeHintPreEditBar(widget);
}

QSize ThemerAgent::sizeHintStatusBar(const StatusBar* widget)
{
    if (KIMToySettings::self()->noStatusBarTheme())
        return ThemerNone::self()->sizeHintStatusBar(widget);
    return m_themer->sizeHintStatusBar(widget);
}

void ThemerAgent::layoutStatusBar(StatusBarLayout* layout)
{
    if (KIMToySettings::self()->noStatusBarTheme())
        return ThemerNone::self()->layoutStatusBar(layout);
    m_themer->layoutStatusBar(layout);
}

void ThemerAgent::resizePreEditBar(const QSize& size)
{
    m_themer->resizePreEditBar(size);
}

void ThemerAgent::resizeStatusBar(const QSize& size)
{
    m_themer->resizeStatusBar(size);
}

void ThemerAgent::maskPreEditBar(PreEditBar* widget)
{
    m_themer->maskPreEditBar(widget);
}

void ThemerAgent::maskStatusBar(StatusBar* widget)
{
    if (KIMToySettings::self()->noStatusBarTheme())
        return ThemerNone::self()->maskStatusBar(widget);
    m_themer->maskStatusBar(widget);
}

void ThemerAgent::maskPropertyWidget(PropertyWidget* widget)
{
    if (KIMToySettings::self()->noStatusBarTheme())
        return ThemerNone::self()->maskPropertyWidget(widget);
    m_themer->maskPropertyWidget(widget);
}

void ThemerAgent::blurPreEditBar(PreEditBar* widget)
{
    m_themer->blurPreEditBar(widget);
}

void ThemerAgent::blurStatusBar(StatusBar* widget)
{
    m_themer->blurStatusBar(widget);
}

void ThemerAgent::drawPreEditBar(PreEditBar* widget)
{
    m_themer->drawPreEditBar(widget);
}

void ThemerAgent::drawStatusBar(StatusBar* widget)
{
    if (KIMToySettings::self()->noStatusBarTheme())
        return ThemerNone::self()->drawStatusBar(widget);
    m_themer->drawStatusBar(widget);
}

void ThemerAgent::drawPropertyWidget(PropertyWidget* widget)
{
    m_themer->drawPropertyWidget(widget);
}
