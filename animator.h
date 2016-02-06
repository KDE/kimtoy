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

#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <QObject>

class QMovie;

class Animator : public QObject
{
    Q_OBJECT
public:
    static Animator* self();
    virtual ~Animator();
    void connectPreEditBarMovie(QMovie* movie);
    void connectStatusBarMovie(QMovie* movie);
    void enable();
    void disable();
Q_SIGNALS:
    void animatePreEditBar();
    void animateStatusBar();
    void enabled();
    void disabled();
private:
    explicit Animator();
    static Animator* m_self;
};

#endif // ANIMATOR_H
