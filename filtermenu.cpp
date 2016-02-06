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

#include "filtermenu.h"

#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QIcon>
#include <QMenu>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionMenuItem>
#include <QVBoxLayout>

#include "propertywidget.h"

FilterMenu::FilterMenu()
{
    setWindowFlags(Qt::Popup | Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    m_menu = new QMenu;
    layout->addWidget(m_menu);
}

FilterMenu::~FilterMenu()
{
}

void FilterMenu::addEntry(const QString& objectPath, const PropertyWidget* pw, bool checked)
{
    QAction* act = new QAction(QIcon::fromTheme(pw->iconName()), pw->name(), this);
    act->setCheckable(true);
    act->setChecked(checked);
    act->setData(objectPath);
    connect(act, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)));
    m_menu->addAction(act);
}

void FilterMenu::paintEvent(QPaintEvent* event)
{
    QPainter p(this);

    QStyleOptionMenuItem menuOpt;
    menuOpt.initFrom(this);
    menuOpt.state = QStyle::State_None;
    menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
    menuOpt.maxIconWidth = 0;
    menuOpt.tabWidth = 0;
    style()->drawPrimitive(QStyle::PE_PanelMenu, &menuOpt, &p, this);
}

void FilterMenu::showEvent(QShowEvent* /*event*/)
{
    /// always popup inside screen
    const QRect sg = QApplication::desktop()->screenGeometry(this);
    int nx = x();
    int ny = y();
    if (nx + width() > sg.x() + sg.width())
        nx -= width();
    if (ny + height() > sg.y() + sg.height())
        ny -= height();
    move(nx, ny);
}

void FilterMenu::slotToggled(bool checked)
{
    QAction* act = static_cast<QAction*>(sender());
    QString objectPath = act->data().toString();
    emit filterChanged(objectPath, checked);
}
