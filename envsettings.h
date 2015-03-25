/*
 *  This file is part of KIMToy, an input method frontend for KDE
 *  Copyright (C) 2011-2015 Ni Hui <shuizhuyuanluo@126.com>
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

#ifndef ENVSETTINGS_H
#define ENVSETTINGS_H

#include <QString>

namespace EnvSettings
{
void load();
void save();
void unset();
void setXIM(const QString& env);
QString XIM();
void setXIM_PROGRAM(const QString& env);
QString XIM_PROGRAM();
void setXMODIFIERS(const QString& env);
QString XMODIFIERS();
void setGTK_IM_MODULE(const QString& env);
QString GTK_IM_MODULE();
void setQT_IM_MODULE(const QString& env);
QString QT_IM_MODULE();
}

#endif // ENVSETTINGS_H
