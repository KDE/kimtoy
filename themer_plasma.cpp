/*
 *  This file is part of KIMToy, an input method frontend for KDE
 *  Copyright (C) 2011-2015 Ni Hui <shuizhuyuanluo@126.com>
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

#include "themer_plasma.h"

#include <QApplication>
#include <QBitmap>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

#include <KIconLoader>
#include <Plasma/Theme>
#include <KWindowEffects>

#include "preeditbar.h"
#include "propertywidget.h"
#include "statusbar.h"
#include "statusbarlayout.h"

#include "kimtoysettings.h"

#include <KDebug>

ThemerPlasma* ThemerPlasma::m_self = 0;

ThemerPlasma* ThemerPlasma::self()
{
    if (!m_self)
        m_self = new ThemerPlasma;
    return m_self;
}

ThemerPlasma::ThemerPlasma()
        : Themer()
{
}

ThemerPlasma::~ThemerPlasma()
{
}

bool ThemerPlasma::loadTheme()
{
    QString themeUri = KIMToySettings::self()->themeUri();

    // "__plasma__" + themeName
    Plasma::Theme plasmaTheme(themeUri.mid(10));

    const QString imagePath = plasmaTheme.imagePath("widgets/background");
    m_statusBarSvg.setImagePath(imagePath);
    m_statusBarSvg.setEnabledBorders(Plasma::FrameSvg::AllBorders);

    m_preeditBarSvg.setImagePath(imagePath);
    m_preeditBarSvg.setEnabledBorders(Plasma::FrameSvg::AllBorders);

    m_preEditFont = QApplication::font();//plasmaTheme.font(Plasma::Theme::DefaultFont);
    m_labelFont = QApplication::font();//plasmaTheme.font(Plasma::Theme::DesktopFont);
    m_candidateFont = QApplication::font();//plasmaTheme.font(Plasma::Theme::DefaultFont);

    m_preEditFontHeight = QFontMetrics(m_preEditFont).height();
    m_labelFontHeight = QFontMetrics(m_labelFont).height();
    m_candidateFontHeight = QFontMetrics(m_candidateFont).height();

    m_preEditColor = plasmaTheme.color(Plasma::Theme::TextColor);
    m_labelColor = plasmaTheme.color(Plasma::Theme::HighlightColor);
    m_candidateColor = plasmaTheme.color(Plasma::Theme::TextColor);
    m_candidateCursorColor = plasmaTheme.color(Plasma::Theme::HighlightColor);

    return true;
}

QSize ThemerPlasma::sizeHintPreEditBar(const PreEditBar* widget) const
{
    int w = 0;
    int h = 0;

    if (widget->preeditVisible || widget->auxVisible) {
        /// preedit and aux
        int pinyinauxw = QFontMetrics(m_preEditFont).width(widget->m_text + widget->m_auxText);
        w = qMax(pinyinauxw, w);
        h += m_preEditFontHeight;

        /// spacing between preedit and lookuptable
        h += 4;
    }

    if (widget->lookuptableVisible) {
        /// lookuptable
        if (KIMToySettings::self()->verticalPreeditBar()) {
            int count = qMin(widget->m_labels.count(), widget->m_candidates.count());
            for (int i = 0; i < count; ++i) {
                QString tmp = widget->m_labels.at(i).trimmed() + widget->m_candidates.at(i).trimmed();
                w = qMax(QFontMetrics(m_candidateFont).width(tmp), w);
                h += m_candidateFontHeight;
            }
        }
        else {
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

    qreal left, top, right, bottom;
    m_preeditBarSvg.getMargins(left, top, right, bottom);

    w += left + right;
    h += top + bottom;

    return QSize(w, h);
}

QSize ThemerPlasma::sizeHintStatusBar(const StatusBar* widget) const
{
    int w = widget->m_layout->count() * 22;
    int h = 22;

    qreal left, top, right, bottom;
    m_preeditBarSvg.getMargins(left, top, right, bottom);

    w += left + right;
    h += top + bottom;

    return QSize(w, h);
}

QPoint ThemerPlasma::anchorPos() const
{
    qreal left, top, right, bottom;
    m_preeditBarSvg.getMargins(left, top, right, bottom);

    return QPoint(left, 0);
}

void ThemerPlasma::layoutStatusBar(StatusBarLayout* layout) const
{
    qreal left, top, right, bottom;
    m_preeditBarSvg.getMargins(left, top, right, bottom);

    int itemCount = layout->count();
    for (int i = 0; i < itemCount; ++i) {
        QLayoutItem* item = layout->m_items.at(i);
        item->setGeometry(QRect(i * 22 + left, top, 22, 22));
    }
}

void ThemerPlasma::resizePreEditBar(const QSize& size)
{
    m_preeditBarSvg.resizeFrame(size);
}

void ThemerPlasma::resizeStatusBar(const QSize& size)
{
    m_statusBarSvg.resizeFrame(size);
}

void ThemerPlasma::maskPreEditBar(PreEditBar* widget)
{
    widget->setMask(m_preeditBarSvg.mask());
}

void ThemerPlasma::maskStatusBar(StatusBar* widget)
{
    widget->setMask(m_statusBarSvg.mask());
}

void ThemerPlasma::maskPropertyWidget(PropertyWidget* widget)
{
    if (!widget->iconName().isEmpty())
        widget->setMask(MainBarIcon(widget->iconName()).mask());
    else
        widget->clearMask();
}

void ThemerPlasma::blurPreEditBar(PreEditBar* widget)
{
    KWindowEffects::enableBlurBehind(widget->winId(), true, m_preeditBarSvg.mask());
}

void ThemerPlasma::blurStatusBar(StatusBar* widget)
{
    KWindowEffects::enableBlurBehind(widget->winId(), true, m_statusBarSvg.mask());
}

void ThemerPlasma::drawPreEditBar(PreEditBar* widget)
{
    QPainter p(widget);

    if (KIMToySettings::self()->backgroundColorizing()) {
        QPainterPath path;
        path.addRegion(m_preeditBarSvg.mask());
        p.fillPath(path, KIMToySettings::self()->preeditBarColorize());
    }

    m_preeditBarSvg.paintFrame(&p);

    qreal left, top, right, bottom;
    m_preeditBarSvg.getMargins(left, top, right, bottom);
    p.translate(left, top);

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

        /// spacing between preedit and lookuptable
        y += 4;
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
                p.setPen(i == widget->m_candidateCursor ? m_candidateCursorColor : m_labelColor);
                w = p.fontMetrics().width(widget->m_labels.at(i).trimmed());
                p.drawText(x, y, w, h, Qt::AlignCenter, widget->m_labels.at(i).trimmed());
                x += w;
                /// draw candidate
                p.setFont(m_candidateFont);
                p.setPen(i == widget->m_candidateCursor ? m_candidateCursorColor : m_candidateColor);
                w = p.fontMetrics().width(widget->m_candidates.at(i).trimmed());
                p.drawText(x, y, w, h, Qt::AlignCenter, widget->m_candidates.at(i).trimmed());
                y += h;
            }
        }
        else {
            for (int i = 0; i < count; ++i) {
                /// draw label
                p.setFont(m_labelFont);
                p.setPen(i == widget->m_candidateCursor ? m_candidateCursorColor : m_labelColor);
                w = p.fontMetrics().width(widget->m_labels.at(i).trimmed());
                p.drawText(x, y, w, h, Qt::AlignCenter, widget->m_labels.at(i).trimmed());
                x += w;
                /// draw candidate
                p.setFont(m_candidateFont);
                p.setPen(i == widget->m_candidateCursor ? m_candidateCursorColor : m_candidateColor);
                w = p.fontMetrics().width(widget->m_candidates.at(i).trimmed() + ' ');
                p.drawText(x, y, w, h, Qt::AlignCenter, widget->m_candidates.at(i).trimmed() + ' ');
                x += w;
            }
        }
    }
}

void ThemerPlasma::drawStatusBar(StatusBar* widget)
{
    QPainter p(widget);

    if (KIMToySettings::self()->backgroundColorizing()) {
        QPainterPath path;
        path.addRegion(m_statusBarSvg.mask());
        p.fillPath(path, KIMToySettings::self()->statusBarColorize());
    }

    m_statusBarSvg.paintFrame(&p);
}

void ThemerPlasma::drawPropertyWidget(PropertyWidget* widget)
{
    QPainter p(widget);
    if (!widget->iconName().isEmpty())
        p.drawPixmap(widget->rect(), MainBarIcon(widget->iconName()));
    else {
        p.setPen(m_preEditColor);
        p.drawText(widget->rect(), Qt::AlignCenter, widget->name());
    }
}
