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

#ifndef THEMER_H
#define THEMER_H

#include <QColor>
#include <QFont>
#include <QPixmap>
#include <QRegion>

class PreEditBar;
class PropertyWidget;
class StatusBar;
class StatusBarLayout;

class Themer
{
public:
    explicit Themer();
    virtual ~Themer();

    virtual bool loadTheme() = 0;
    void loadSettings();

    virtual QSize sizeHintPreEditBar(const PreEditBar* widget) const = 0;
    virtual QSize sizeHintStatusBar(const StatusBar* widget) const = 0;

    virtual void layoutStatusBar(StatusBarLayout* layout) const = 0;

    virtual void resizePreEditBar(const QSize& size);
    virtual void resizeStatusBar(const QSize& size);

    virtual void maskPreEditBar(PreEditBar* widget) = 0;
    virtual void maskStatusBar(StatusBar* widget) = 0;

    virtual void blurPreEditBar(PreEditBar* widget);
    virtual void blurStatusBar(StatusBar* widget);

    virtual void drawPreEditBar(PreEditBar* widget) = 0;
    virtual void drawStatusBar(StatusBar* widget) = 0;
    virtual void drawPropertyWidget(PropertyWidget* widget) = 0;

protected:
    QFont m_preEditFont;
    QFont m_labelFont;
    QFont m_candidateFont;
    int m_preEditFontHeight;
    int m_labelFontHeight;
    int m_candidateFontHeight;
    QColor m_preEditColor;
    QColor m_labelColor;
    QColor m_candidateColor;
};

#endif // THEMER_H
