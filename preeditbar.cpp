/*
 *  This file is part of KIMToy, an input method frontend for KDE
 *  Copyright (C) 2011-2016 Ni Hui <shuizhuyuanluo@126.com>
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

#include "preeditbar.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDebug>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QVBoxLayout>

#include <KWindowSystem>

#include "themeragent.h"

#include "kimtoysettings.h"

#include <QX11Info>
#include <X11/Xlib.h>

PreEditBar::PreEditBar()
{
    bool enableTransparency = KIMToySettings::self()->backgroundTransparency();
    setAttribute(Qt::WA_TranslucentBackground, enableTransparency);

    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::BypassWindowManagerHint);
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove);
    KWindowSystem::setType(winId(), NET::Tooltip);
//     setAttribute(Qt::WA_X11NetWmWindowTypeToolTip);

    installEventFilter(this);

    m_moving = false;

    spotX = 0;
    spotY = 0;

    preeditVisible = false;
    auxVisible = false;
    lookuptableVisible = false;

    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateSpotLocation",
                       this, SLOT(slotUpdateSpotLocation(int,int)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowPreedit",
                       this, SLOT(slotShowPreedit(bool)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowAux",
                       this, SLOT(slotShowAux(bool)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowLookupTable",
                       this, SLOT(slotShowLookupTable(bool)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdatePreeditCaret",
                       this, SLOT(slotUpdatePreeditCaret(int)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdatePreeditText",
                       this, SLOT(slotUpdatePreeditText(QString,QString)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateAux",
                       this, SLOT(slotUpdateAux(QString,QString)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateLookupTableCursor",
                       this, SLOT(slotUpdateLookupTableCursor(int)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateLookupTable",
                       this, SLOT(slotUpdateLookupTable(QStringList,QStringList,QStringList,bool,bool)));

    updateSize();
}

PreEditBar::~PreEditBar()
{
}

bool PreEditBar::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            m_pointPos = mouseEvent->pos();
            m_moving = true;
            return true;
        }
        m_moving = false;
        return QObject::eventFilter(object, event);
    }
    if (event->type() == QEvent::MouseMove && m_moving) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        move(mouseEvent->globalPos() - m_pointPos);
        return true;
    }
    return QObject::eventFilter(object, event);
}

void PreEditBar::resizeEvent(QResizeEvent* event)
{
    ThemerAgent::resizePreEditBar(event->size());
    if (KIMToySettings::self()->enableWindowMask()) {
        ThemerAgent::maskPreEditBar(this);
    }
    slotUpdateSpotLocation(spotX, spotY);
    if (KIMToySettings::self()->enableBackgroundBlur()) {
        ThemerAgent::blurPreEditBar(this);
    }
}

void PreEditBar::showEvent(QShowEvent* event)
{
//     Plasma::WindowEffects::overrideShadow(winId(), true);
    Display* dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_NET_WM_SHADOW", False);
    XDeleteProperty(dpy, winId(), atom);
}

void PreEditBar::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    ThemerAgent::drawPreEditBar(this);
}

void PreEditBar::slotUpdateSpotLocation(int x, int y)
{
    spotX = x;
    spotY = y;

    QRect screenRect = QApplication::desktop()->screenGeometry(QPoint(x, y));
    x = qMin(x, screenRect.x() + screenRect.width() - width());
    y = qMin(y, screenRect.y() + screenRect.height());

    QPoint anchorPos = ThemerAgent::anchorPos();
    x -= anchorPos.x();
    y -= anchorPos.y();

    if (y + height() > screenRect.y() + screenRect.height()) {
        /// minus 20 to make preedit bar never overlap the input context
        y -= height() - anchorPos.y() + 20;
    }
    if (QPoint(x, y) != pos()) {
        move(x, y);
    }
}

void PreEditBar::slotShowPreedit(bool show)
{
    preeditVisible = show;
    updateVisible();
    updateSize();
    update();
}

void PreEditBar::slotShowAux(bool show)
{
    auxVisible = show;
    updateVisible();
    updateSize();
    update();
}

void PreEditBar::slotShowLookupTable(bool show)
{
    lookuptableVisible = show;
    updateVisible();
    updateSize();
    update();
}

void PreEditBar::slotUpdatePreeditCaret(int pos)
{
    m_cursorPos = pos;
    update();
}

void PreEditBar::slotUpdatePreeditText(const QString& text,
                                       const QString& attrs)
{
    Q_UNUSED(attrs)
    m_text = text;
    updateSize();
    update();
}

void PreEditBar::slotUpdateAux(const QString& text,
                               const QString& attrs)
{
    Q_UNUSED(attrs)
    m_auxText = text;
    updateSize();
    update();
}

void PreEditBar::slotUpdateLookupTableCursor(int pos)
{
    m_candidateCursor = pos;
    update();
}

void PreEditBar::slotUpdateLookupTable(const QStringList& labels,
                                       const QStringList& candidates,
                                       const QStringList& attrs,
                                       bool hasPrev,
                                       bool hasNext)
{
    Q_UNUSED(attrs)
    m_labels = labels;
    m_candidates = candidates;
    m_hasPrev = hasPrev;
    m_hasNext = hasNext;
    updateSize();
    update();
}

void PreEditBar::updateVisible()
{
    bool visible = preeditVisible || auxVisible || lookuptableVisible;
    if (isVisible() != visible) {
        setVisible(visible);
    }
}

void PreEditBar::updateSize()
{
    resize(ThemerAgent::sizeHintPreEditBar(this));
}
