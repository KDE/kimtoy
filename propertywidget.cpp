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

#include "propertywidget.h"

#include <QMouseEvent>
#include <KIcon>

#include "themeragent.h"

PropertyWidget::PropertyWidget()
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    m_type = Unknown;
}

PropertyWidget::~PropertyWidget()
{
}

void PropertyWidget::setProperty(const QString& objectPath,
                                 const QString& name,
                                 const QString& iconName,
                                 const QString& description)
{
    m_name = name;
    m_iconName = iconName;
    m_description = description;
    m_type = determineType(objectPath, iconName);
    setToolTip(description);
    ThemerAgent::maskPropertyWidget(this);
    update();
}

QString PropertyWidget::name() const
{
    return m_name;
}

QString PropertyWidget::iconName() const
{
    return m_iconName;
}

QString PropertyWidget::description() const
{
    return m_description;
}

PropertyType PropertyWidget::type() const
{
    return m_type;
}

PropertyType PropertyWidget::determineType( const QString& objectPath, const QString& iconName )
{
    // fcitx property
    if (objectPath == "/Fcitx/im") {
        if (iconName == "fcitx-eng") return IM_Direct;
        if (iconName == "fcitx-pinyin") return IM_Pinyin;
        if (iconName == "fcitx-shuangpin") return IM_Shuangpin;
    }
    if (objectPath == "/Fcitx/fullwidth") {
        if (iconName == "fcitx-fullwidth-active") return Letter_Full;
        if (iconName == "fcitx-fullwidth-inactive") return Letter_Half;
    }
    if (objectPath == "/Fcitx/punc") {
        if (iconName == "fcitx-punc-active") return Punct_Full;
        if (iconName == "fcitx-punc-inactive") return Punct_Half;
    }
    if (objectPath == "/Fcitx/chttrans") {
        if (iconName == "fcitx-chttrans-inactive") return Chinese_Simplified;
        if (iconName == "fcitx-chttrans-active") return Chinese_Traditional;
    }
    if (objectPath == "/Fcitx/remind") {
        if (iconName == "fcitx-remind-active") return Remind_On;
        if (iconName == "fcitx-remind-inactive") return Remind_Off;
    }
    if (objectPath == "/Fcitx/vk") {
        if (iconName == "fcitx-vk-inactive") return SoftKeyboard_Off;
        if (iconName == "fcitx-vk-active") return SoftKeyboard_On;
    }
    if (objectPath == "/Fcitx/logo") {
        if (iconName == "fcitx") return Logo;
    }

    // ibus property
    if (objectPath == "/IBus/status" || objectPath == "/IBus/mode.chinese") {
        if (iconName.endsWith("eng.svg")) return IM_Direct;
        if (iconName.endsWith("english.svg")) return IM_Direct;
        if (iconName.endsWith("han.svg")) return IM_Chinese;
        if (iconName.endsWith("chinese.svg")) return IM_Chinese;
    }
    if (objectPath == "/IBus/full_letter" || objectPath == "/IBus/mode.full") {
        if (iconName.endsWith("full.svg")) return Letter_Full;
        if (iconName.endsWith("fullwidth.svg")) return Letter_Full;
        if (iconName.endsWith("full-letter.svg")) return Letter_Full;
        if (iconName.endsWith("half.svg")) return Letter_Half;
        if (iconName.endsWith("halfwidth.svg")) return Letter_Half;
        if (iconName.endsWith("half-letter.svg")) return Letter_Half;
    }
    if (objectPath == "/IBus/full_punct" || objectPath == "/IBus/mode.full_punct") {
        if (iconName.endsWith("cnpunc.svg")) return Punct_Full;
        if (iconName.endsWith("full-punct.svg")) return Punct_Full;
        if (iconName.endsWith("enpunc.svg")) return Punct_Half;
        if (iconName.endsWith("half-punct.svg")) return Punct_Half;
    }
    if (objectPath == "/IBus/_trad chinese" || objectPath == "/IBus/mode.simp") {
        if (iconName.endsWith("simp-chinese.svg")) return Chinese_Simplified;
        if (iconName.endsWith("trad-chinese.svg")) return Chinese_Traditional;
    }
    if (objectPath == "/IBus/Logo") {
        if (iconName == "ibus") return Logo;
    }
    if (objectPath == "/IBus/setup") {
        if (iconName.endsWith("setup.svg")) return Setup;
    }

    // scim property
    if (objectPath == "/IMEngine/Pinyin/Letter") {
        if (iconName.endsWith("full-letter.png")) return Letter_Full;
        if (iconName.endsWith("half-letter.png")) return Letter_Half;
    }
    if (objectPath == "/IMEngine/Pinyin/Punct") {
        if (iconName.endsWith("full-punct.png")) return Punct_Full;
        if (iconName.endsWith("half-punct.png")) return Punct_Half;
    }
    if (objectPath == "/Logo") {
        if (iconName == "keyboard.png") return Logo;
        if (iconName == "trademark.png") return Logo;
    }

    return Unknown;
}

bool PropertyWidget::operator==(const PropertyWidget& rhs) const
{
    return m_name == rhs.m_name
           && m_iconName == rhs.m_iconName
           && m_description == rhs.m_description;
}

void PropertyWidget::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);
    if (event->button() == Qt::LeftButton)
        emit clicked();
}

void PropertyWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    ThemerAgent::drawPropertyWidget(this);
}
