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

#include "animator.h"

#include <QMovie>

Animator* Animator::m_self = 0;

Animator* Animator::self()
{
    if (!m_self)
        m_self = new Animator;
    return m_self;
}

Animator::Animator()
{
}

Animator::~Animator()
{
}

void Animator::connectPreEditBarMovie(QMovie* movie)
{
    connect(movie, SIGNAL(frameChanged(int)), this, SIGNAL(animatePreEditBar()));
    connect(this, SIGNAL(enabled()), movie, SLOT(start()));
    connect(this, SIGNAL(disabled()), movie, SLOT(stop()));
    movie->start();
}

void Animator::connectStatusBarMovie(QMovie* movie)
{
    connect(movie, SIGNAL(frameChanged(int)), this, SIGNAL(animateStatusBar()));
    connect(this, SIGNAL(enabled()), movie, SLOT(start()));
    connect(this, SIGNAL(disabled()), movie, SLOT(stop()));
    movie->start();
}

void Animator::enable()
{
    emit enabled();
}

void Animator::disable()
{
    emit disabled();
}
