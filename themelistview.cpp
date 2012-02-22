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

#include "themelistview.h"

#include <QScrollBar>

#include <KConfigDialogManager>
#include <KDebug>

#include "themelistmodel.h"

#include "kimtoysettings.h"

ThemeListView::ThemeListView(QWidget* parent)
        : QListView(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setAutoScroll(false);
    setAlternatingRowColors(true);

    ThemeListModel* tm = new ThemeListModel(this);
    ThemeListDelegate* td = new ThemeListDelegate(this);
    setModel(tm);
    setItemDelegate(td);
    connect(tm, SIGNAL(relayoutNeeded()), this, SLOT(relayout()));

    connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(slotCurrentChanged(const QModelIndex&, const QModelIndex&)));

    m_themeUri = KIMToySettings::self()->themeUri();

    KConfigDialogManager::changedMap()->insert("ThemeListView", SIGNAL(themeUriChanged(const QString&)));
}

ThemeListView::~ThemeListView()
{
}

QString ThemeListView::themeUri() const
{
    return m_themeUri;
}

void ThemeListView::setThemeUri(const QString& themeUri)
{
    m_themeUri = themeUri;

    ThemeListModel* tm = static_cast<ThemeListModel*>(model());
    QModelIndexList selects = tm->match(tm->index(0), Qt::DisplayRole, themeUri);
    if (selects.count() > 0) {
        selectionModel()->select(selects.first(), QItemSelectionModel::ClearAndSelect);
    }
}

void ThemeListView::reload()
{
    ThemeListModel* tm = static_cast<ThemeListModel*>(model());
    tm->reloadThemes();

    setThemeUri(KIMToySettings::self()->themeUri());
}

void ThemeListView::resizeEvent(QResizeEvent* event)
{
    QListView::resizeEvent(event);
    adaptSize();
}

void ThemeListView::slotCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous)

    m_themeUri = current.data(Qt::DisplayRole).toString();
    emit themeUriChanged(m_themeUri);
}

void ThemeListView::relayout()
{
    scheduleDelayedItemsLayout();
}

void ThemeListView::adaptSize()
{
    ThemeListDelegate* td = static_cast<ThemeListDelegate*>(itemDelegate());
    ThemeListModel* tm = static_cast<ThemeListModel*>(model());

    if (!td || !tm)
        return;

    tm->setPreviewWidth(width() - 2 * 4 - verticalScrollBar()->width());  // margin
}
