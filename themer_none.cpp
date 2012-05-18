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

#include "themer_none.h"

#include <QBitmap>
#include <QPainter>

#include <KIconLoader>

#include "preeditbar.h"
#include "statusbar.h"
#include "statusbarlayout.h"
#include "propertywidget.h"

#include "kimtoysettings.h"

ThemerNone* ThemerNone::m_self = 0;

ThemerNone* ThemerNone::self()
{
    if (!m_self)
        m_self = new ThemerNone;
    return m_self;
}

ThemerNone::ThemerNone()
        : Themer()
{
}

ThemerNone::~ThemerNone()
{
}

bool ThemerNone::loadTheme()
{
    m_preEditFont = KIMToySettings::self()->preeditFont();
    m_labelFont = KIMToySettings::self()->labelFont();
    m_candidateFont = KIMToySettings::self()->candidateFont();

    m_preEditFontHeight = QFontMetrics(m_preEditFont).height();
    m_labelFontHeight = QFontMetrics(m_labelFont).height();
    m_candidateFontHeight = QFontMetrics(m_candidateFont).height();

    m_preEditColor = KIMToySettings::self()->preeditColor();
    m_labelColor = KIMToySettings::self()->labelColor();
    m_candidateColor = KIMToySettings::self()->candidateColor();
    return true;
}

QSize ThemerNone::sizeHintPreEditBar(const PreEditBar* widget) const
{
    int w = 0;
    int h = 0;

    if (widget->preeditVisible || widget->auxVisible) {
        /// preedit and aux
        int pinyinauxw = QFontMetrics(m_preEditFont).width(widget->m_text + widget->m_auxText);
        w = qMax(pinyinauxw, w);
        h += m_preEditFontHeight;
    }

    if (widget->lookuptableVisible) {
        if (KIMToySettings::self()->verticalPreeditBar()) {
            /// lookuptable
            int count = qMin(widget->m_labels.count(), widget->m_candidates.count());
            for (int i = 0; i < count; ++i) {
                QString tmp = widget->m_labels.at(i).trimmed() + widget->m_candidates.at(i).trimmed();
                w = qMax(QFontMetrics(m_candidateFont).width(tmp), w);
                h += m_candidateFontHeight;
            }
        }
        else {
            /// lookuptable
            QString tmp;
            int count = qMin(widget->m_labels.count(), widget->m_candidates.count());
            for (int i = 0; i < count; ++i) {
                tmp += widget->m_labels.at(i).trimmed() + widget->m_candidates.at(i).trimmed() + ' ';
            }
            int lookuptablew = QFontMetrics(m_candidateFont).width(tmp);
            w = qMax(lookuptablew, w);
            h += m_candidateFontHeight;
        }
    }

    if (!KIMToySettings::self()->enablePreeditResizing()) {
        /// align with skin width + 70 * x
        const int align = 70;
        w = ((w - 1) / align + 1) * align;
    }

    return QSize(w, h);
}

QSize ThemerNone::sizeHintStatusBar(const StatusBar* widget) const
{
    int w = widget->m_layout->count() * 22;
    int h = 22;
    return QSize(w, h);
}

void ThemerNone::layoutStatusBar(StatusBarLayout* layout) const
{
    int itemCount = layout->count();
    for (int i = 0; i < itemCount; ++i) {
        QLayoutItem* item = layout->m_items.at(i);
        item->setGeometry(QRect(i * 22, 0, 22, 22));
    }
}

void ThemerNone::maskPreEditBar(PreEditBar* widget)
{
    widget->clearMask();
}

void ThemerNone::maskStatusBar(StatusBar* widget)
{
    widget->clearMask();
}

void ThemerNone::maskPropertyWidget(PropertyWidget* widget)
{
    if (!widget->iconName().isEmpty())
        widget->setMask(MainBarIcon(widget->iconName()).mask());
    else
        widget->clearMask();
}

void ThemerNone::drawPreEditBar(PreEditBar* widget)
{
    QPainter p(widget);

    if (KIMToySettings::self()->backgroundColorizing()) {
        p.fillRect(widget->rect(), KIMToySettings::self()->preeditBarColorize());
    }

    int x = 0;
    int y = 0;

    if (widget->preeditVisible || widget->auxVisible) {
        /// draw preedit / aux text
        p.setFont(m_preEditFont);
        p.setPen(m_preEditColor);

        p.drawText(x, y, widget->width(), m_preEditFontHeight, Qt::AlignLeft, widget->m_text + widget->m_auxText);
        if (widget->preeditVisible) {
            int pixelsWide = QFontMetrics(m_preEditFont).width(widget->m_text.left(widget->m_cursorPos));
            p.drawLine(pixelsWide, 0, pixelsWide, m_preEditFontHeight);
        }
        y += m_preEditFontHeight;
    }

    if (widget->lookuptableVisible) {
        /// draw lookup table
        int w = 0;
        int h = qMax(m_labelFontHeight, m_candidateFontHeight);

        /// draw labels and candidates
        int count = qMin(widget->m_labels.count(), widget->m_candidates.count());

        if (KIMToySettings::self()->verticalPreeditBar()) {
            for (int i = 0; i < count; ++i) {
                /// draw label
                x = 0;
                p.setFont(m_labelFont);
                p.setPen(m_labelColor);
                w = p.fontMetrics().width(widget->m_labels.at(i).trimmed());
                p.drawText(x, y, w, h, Qt::AlignCenter, widget->m_labels.at(i).trimmed());
                x += w;
                /// draw candidate
                p.setFont(m_candidateFont);
                p.setPen(m_candidateColor);
                w = p.fontMetrics().width(widget->m_candidates.at(i).trimmed());
                p.drawText(x, y, w, h, Qt::AlignCenter, widget->m_candidates.at(i).trimmed());
                y += h;
            }
        }
        else {
            for (int i = 0; i < count; ++i) {
                /// draw label
                p.setFont(m_labelFont);
                p.setPen(m_labelColor);
                w = p.fontMetrics().width(widget->m_labels.at(i).trimmed());
                p.drawText(x, y, w, h, Qt::AlignCenter, widget->m_labels.at(i).trimmed());
                x += w;
                /// draw candidate
                p.setFont(m_candidateFont);
                p.setPen(m_candidateColor);
                w = p.fontMetrics().width(widget->m_candidates.at(i).trimmed() + ' ');
                p.drawText(x, y, w, h, Qt::AlignCenter, widget->m_candidates.at(i).trimmed() + ' ');
                x += w;
            }
        }
    }
}

void ThemerNone::drawStatusBar(StatusBar* widget)
{
    if (KIMToySettings::self()->backgroundColorizing()) {
        QPainter p(widget);
        p.fillRect(widget->rect(), KIMToySettings::self()->statusBarColorize());
    }
}

void ThemerNone::drawPropertyWidget(PropertyWidget* widget)
{
    QPainter p(widget);
    if (!widget->iconName().isEmpty())
        p.drawPixmap(widget->rect(), MainBarIcon(widget->iconName()));
    else
        p.drawText(widget->rect(), Qt::AlignCenter, widget->name());
}
