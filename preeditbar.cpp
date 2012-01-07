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

#include "preeditbar.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QVBoxLayout>

#include <KDebug>
#include <KLocale>
#include <KWindowSystem>

#include "themeragent.h"

#include "kimtoysettings.h"

PreEditBar::PreEditBar()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::StaysOnTop);
    KWindowSystem::setType(winId(), NET::PopupMenu);

    installEventFilter(this);

    m_moving = false;

    preeditVisible = false;
    auxVisible = false;
    lookuptableVisible = false;

    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateSpotLocation",
                       this, SLOT(slotUpdateSpotLocation(int, int)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowPreedit",
                       this, SLOT(slotShowPreedit(bool)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowAux",
                       this, SLOT(slotShowAux(bool)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "ShowLookupTable",
                       this, SLOT(slotShowLookupTable(bool)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdatePreeditCaret",
                       this, SLOT(slotUpdatePreeditCaret(int)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdatePreeditText",
                       this, SLOT(slotUpdatePreeditText(const QString&,
                                                        const QString&)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateAux",
                       this, SLOT(slotUpdateAux(const QString&,
                                                const QString&)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateLookupTable",
                       this, SLOT(slotUpdateLookupTable(const QStringList&,
                                                        const QStringList&,
                                                        const QStringList&,
                                                        bool,
                                                        bool)));

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
    slotUpdateSpotLocation(x(), y());
    if (KIMToySettings::self()->enableBackgroundBlur()) {
        ThemerAgent::blurPreEditBar(this);
    }
}

void PreEditBar::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    ThemerAgent::drawPreEditBar(this);
}

void PreEditBar::slotUpdateSpotLocation(int x, int y)
{
    QRect screenRect = QApplication::desktop()->screenGeometry(QPoint(x, y));
    x = qMin(x, screenRect.x() + screenRect.width() - width());
    if (y > screenRect.y() + screenRect.height()) {
        y = screenRect.height();
    }

    if (y + height() > screenRect.y() + screenRect.height()) {
        /// minus 20 to make preedit bar never overlap the input context
        y -= height() + 20;
    }
    if (QPoint(x, y) != pos())
        move(x, y);
}

void PreEditBar::slotShowPreedit(bool show)
{
    preeditVisible = show;
    updateVisible();
}

void PreEditBar::slotShowAux(bool show)
{
    auxVisible = show;
    updateVisible();
}

void PreEditBar::slotShowLookupTable(bool show)
{
    lookuptableVisible = show;
    updateVisible();
}

void PreEditBar::slotUpdatePreeditCaret(int pos)
{
    m_cursorPos = pos;
    update();
}

void PreEditBar::slotUpdatePreeditText(const QString& text,
                                       const QString& attrs)
{
    Q_UNUSED(attrs);
    m_text = text;
    updateSize();
    update();
}

void PreEditBar::slotUpdateAux(const QString& text,
                               const QString& attrs)
{
    Q_UNUSED(attrs);
    m_auxText = text;
    updateSize();
    update();
}

void PreEditBar::slotUpdateLookupTable(const QStringList& labels,
                                       const QStringList& candidates,
                                       const QStringList& attrs,
                                       bool hasPrev,
                                       bool hasNext)
{
    Q_UNUSED(attrs);
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
        updateSize();
    }
}

void PreEditBar::updateSize()
{
    resize(ThemerAgent::sizeHintPreEditBar(this));
}
