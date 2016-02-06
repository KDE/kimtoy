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

#ifndef SCIM_KCONFIG_CONFIG_H
#define SCIM_KCONFIG_CONFIG_H

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_C_STDIO

#define ENABLE_DEBUG 9

#include <scim.h>

class KConfig;

using namespace scim;

class KConfigConfig : public ConfigBase
{
public:
    explicit KConfigConfig();
    virtual ~KConfigConfig();
    virtual bool valid() const;
    virtual String get_name() const;
    virtual bool read(const String& key, String *ret) const;
    virtual bool read(const String& key, int *ret) const;
    virtual bool read(const String& key, double *ret) const;
    virtual bool read(const String& key, bool *ret) const;
    virtual bool read(const String& key, std::vector <String> *ret) const;
    virtual bool read(const String& key, std::vector <int> *ret) const;
    virtual bool write(const String& key, const String& value);
    virtual bool write(const String& key, int value);
    virtual bool write(const String& key, double value);
    virtual bool write(const String& key, bool value);
    virtual bool write(const String& key, const std::vector <String>& value);
    virtual bool write(const String& key, const std::vector <int>& value);
    virtual bool flush();
    virtual bool erase(const String& key);
    virtual bool reload();
private:
    KConfig* m_config;
};

#endif // SCIM_KCONFIG_CONFIG_H
