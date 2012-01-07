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

#ifndef PROPERTYWIDGET_H
#define PROPERTYWIDGET_H

#include <QWidget>

// class Themer;
class ThemerFcitx;
class ThemerNone;
class ThemerPlasma;
class ThemerSogou;

typedef enum {
    Unknown,
    IM_Direct,
    IM_Chinese,
    IM_Pinyin,
    IM_Shuangpin,
    Letter_Full,
    Letter_Half,
    Punct_Full,
    Punct_Half,
    Chinese_Simplified,
    Chinese_Traditional,
    Remind_On,
    Remind_Off,
    SoftKeyboard_On,
    SoftKeyboard_Off,
    Setup,
    Logo
} PropertyType;

class PropertyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PropertyWidget();
    virtual ~PropertyWidget();
    void setProperty(const QString& objectPath,
                     const QString& name,
                     const QString& iconName,
                     const QString& description);
    PropertyType type() const;
    static PropertyType determineType( const QString& objectPath, const QString& iconName );
    bool operator==(const PropertyWidget& rhs);
Q_SIGNALS:
    void clicked();
protected:
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void paintEvent(QPaintEvent* event);
private:
//         friend class Themer;
    friend class ThemerFcitx;
    friend class ThemerNone;
    friend class ThemerPlasma;
    friend class ThemerSogou;
    QString m_name;
    QString m_iconName;
    QString m_description;
    PropertyType m_type;
};

#endif // PROPERTYWIDGET_H
