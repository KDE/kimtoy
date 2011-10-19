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

#ifndef STATUSBARLAYOUT_H
#define STATUSBARLAYOUT_H

#include <QLayout>
#include <QLayoutItem>

// class Themer;
class ThemerFcitx;
class ThemerNone;
class ThemerPlasma;
class ThemerSogou;

class StatusBarLayout : public QLayout
{
    Q_OBJECT
public:
    explicit StatusBarLayout(QWidget* parent = 0);
    virtual ~StatusBarLayout();
    virtual int count() const;
    virtual void addItem(QLayoutItem* item);
    virtual QLayoutItem* itemAt(int index) const;
    virtual QSize minimumSize() const;
    virtual QSize sizeHint() const;
    virtual QLayoutItem* takeAt(int index);
protected:
    virtual void setGeometry(const QRect& rect);
private:
//         friend class Themer;
    friend class ThemerFcitx;
    friend class ThemerNone;
    friend class ThemerPlasma;
    friend class ThemerSogou;
    QList<QLayoutItem*> m_items;
};

#endif // STATUSBARLAYOUT_H
