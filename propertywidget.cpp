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

#include "propertywidget.h"

#include <QMouseEvent>
#include <KIcon>

#include "themeragent.h"

PropertyWidget::PropertyWidget()
{
    setFixedSize( QSize( 22, 22 ) );
}

PropertyWidget::~PropertyWidget()
{
}

void PropertyWidget::setProperty( const QString& name,
                                  const QString& iconName,
                                  const QString& description )
{
    m_name = name;
    m_iconName = iconName;
    m_description = description;
    update();
}

bool PropertyWidget::operator==( const PropertyWidget& rhs )
{
    return m_name == rhs.m_name
    && m_iconName == rhs.m_iconName
    && m_description == rhs.m_description;
}

void PropertyWidget::mouseReleaseEvent( QMouseEvent* event )
{
    QWidget::mouseReleaseEvent( event );
    if ( event->button() == Qt::LeftButton )
        emit clicked();
}

void PropertyWidget::paintEvent( QPaintEvent* event )
{
    ThemerAgent::drawPropertyWidget( this );
}
