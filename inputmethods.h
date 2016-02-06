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

#ifndef INPUTMETHODS_H
#define INPUTMETHODS_H

#include <QString>

class InputMethod
{
public:
    explicit InputMethod() {}
    virtual ~InputMethod() {}
    virtual void run() = 0;
    virtual void kill() = 0;
    virtual void saveEnvSettings() const = 0;
    virtual bool getVersion(QString& version) const = 0;
};

class FcitxInputMethod : public InputMethod
{
public:
    static FcitxInputMethod* self();
    virtual ~FcitxInputMethod();
    virtual void run();
    virtual void kill();
    virtual void saveEnvSettings() const;
    virtual bool getVersion(QString& version) const;
private:
    explicit FcitxInputMethod();
    static FcitxInputMethod* m_self;
};

class IBusInputMethod : public InputMethod
{
public:
    static IBusInputMethod* self();
    virtual ~IBusInputMethod();
    virtual void run();
    virtual void kill();
    virtual void saveEnvSettings() const;
    virtual bool getVersion(QString& version) const;
private:
    explicit IBusInputMethod();
    static IBusInputMethod* m_self;
};

class SCIMInputMethod : public InputMethod
{
public:
    static SCIMInputMethod* self();
    virtual ~SCIMInputMethod();
    virtual void run();
    virtual void kill();
    virtual void saveEnvSettings() const;
    virtual bool getVersion(QString& version) const;
private:
    explicit SCIMInputMethod();
    static SCIMInputMethod* m_self;
};

#endif // INPUTMETHODS_H
