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

#include "statusbarlayout.h"

#include "themeragent.h"

StatusBarLayout::StatusBarLayout( QWidget* parent )
: QLayout(parent)
{
}

StatusBarLayout::~StatusBarLayout()
{
    qDeleteAll(m_items);
}

int StatusBarLayout::count() const
{
    return m_items.size();
}

void StatusBarLayout::addItem( QLayoutItem* item )
{
    m_items << item;
}

QLayoutItem* StatusBarLayout::itemAt( int index ) const
{
    return m_items.value( index );
}

QSize StatusBarLayout::minimumSize() const
{
    QSize s( 0, 0 );
    for ( int i = 0; i < m_items.size(); ++i ) {
        QLayoutItem* item = m_items.at( i );
        s = s.expandedTo( item->minimumSize() );
    }
    return s;
}

QSize StatusBarLayout::sizeHint() const
{
    QSize s( 0, 0 );
    for ( int i = 0; i < m_items.size(); ++i ) {
        QLayoutItem* item = m_items.at( i );
        s = s.expandedTo( item->sizeHint() );
    }
    return s;
}

QLayoutItem* StatusBarLayout::takeAt( int index )
{
    if ( index >= 0 && index < m_items.size() )
        return m_items.takeAt( index );
    else
        return 0;
}

void StatusBarLayout::setGeometry( const QRect& rect )
{
    QLayout::setGeometry( rect );

    ThemerAgent::layoutStatusBar( this );
}
