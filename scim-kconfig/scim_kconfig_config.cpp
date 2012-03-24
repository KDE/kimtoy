/*
 *  This file is part of KIMToy, an input method frontend for KDE
 *  Copyright (C) 2011-2012 Ni Hui <shuizhuyuanluo@126.com>
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

#include "scim_kconfig_config.h"

#include <sys/time.h>

#include <QString>
#include <KConfig>
#include <KConfigGroup>

#define scim_module_init kconfig4_LTX_scim_module_init
#define scim_module_exit kconfig4_LTX_scim_module_exit
#define scim_config_module_init kconfig4_LTX_scim_config_module_init
#define scim_config_module_create_config kconfig4_LTX_scim_config_module_create_config

#define MODULE_EXPORT __attribute__((visibility("default")))

extern "C" {
    MODULE_EXPORT void scim_module_init()
    {
        SCIM_DEBUG_CONFIG(1) << "Initializing KConfig Config module...\n";
    }
    MODULE_EXPORT void scim_module_exit()
    {
        SCIM_DEBUG_CONFIG(1) << "Exiting KConfig Config module...\n";
    }
    MODULE_EXPORT void scim_config_module_init()
    {
        SCIM_DEBUG_CONFIG(1) << "Initializing KConfig Config module (more)...\n";
    }
    MODULE_EXPORT ConfigPointer scim_config_module_create_config()
    {
        SCIM_DEBUG_CONFIG(1) << "Creating a KConfig Config instance...\n";
        return new KConfigConfig;
    }
}

KConfigConfig::KConfigConfig()
: ConfigBase()
{
    m_config = new KConfig("scimrc");
}

KConfigConfig::~KConfigConfig()
{
    m_config->sync();
    delete m_config;
}

bool KConfigConfig::valid() const
{
    return ConfigBase::valid();
}

String KConfigConfig::get_name() const
{
    return "kconfig4";
}

bool KConfigConfig::read(const String& key, String *ret) const
{
    if (!valid() || !ret || key.empty())
        return false;

    KConfigGroup defaultGroup = m_config->group("SCIM");
    QString qkey = QString::fromUtf8(key.c_str());
    if (defaultGroup.hasKey(qkey)) {
        QString value = defaultGroup.readEntry(qkey, QString());
        *ret = String(value.toUtf8());
        return true;
    }
    else {
        *ret = String("");
        return false;
    }
}

bool KConfigConfig::read(const String& key, int *ret) const
{
    if (!valid() || !ret || key.empty())
        return false;

    KConfigGroup defaultGroup = m_config->group("SCIM");
    QString qkey = QString::fromUtf8(key.c_str());
    if (defaultGroup.hasKey(qkey)) {
        *ret = defaultGroup.readEntry(qkey, 0);
        return true;
    }
    else {
        *ret = 0;
        return false;
    }
}

bool KConfigConfig::read(const String& key, double *ret) const
{
    if (!valid() || !ret || key.empty())
        return false;

    KConfigGroup defaultGroup = m_config->group("SCIM");
    QString qkey = QString::fromUtf8(key.c_str());
    if (defaultGroup.hasKey(qkey)) {
        *ret = defaultGroup.readEntry(qkey, 0.0);
        return true;
    }
    else {
        *ret = 0.0;
        return false;
    }
}

bool KConfigConfig::read(const String& key, bool *ret) const
{
    if (!valid() || !ret || key.empty())
        return false;

    KConfigGroup defaultGroup = m_config->group("SCIM");
    QString qkey = QString::fromUtf8(key.c_str());
    if (defaultGroup.hasKey(qkey)) {
        *ret = defaultGroup.readEntry(qkey, false);
        return true;
    }
    else {
        *ret = false;
        return false;
    }
}

bool KConfigConfig::read(const String& key, std::vector <String> *ret) const
{
    if (!valid() || !ret || key.empty())
        return false;

    KConfigGroup defaultGroup = m_config->group("SCIM");
    QString qkey = QString::fromUtf8(key.c_str());
    if (defaultGroup.hasKey(qkey)) {
        QStringList value = defaultGroup.readEntry(qkey, QStringList());
        foreach(const QString& s, value) {
            ret->push_back(String(s.toUtf8()));
        }
        return true;
    }
    else
        return false;
}

bool KConfigConfig::read(const String& key, std::vector <int> *ret) const
{
    if (!valid() || !ret || key.empty())
        return false;

    KConfigGroup defaultGroup = m_config->group("SCIM");
    QString qkey = QString::fromUtf8(key.c_str());
    if (defaultGroup.hasKey(qkey)) {
        QList<int> value = defaultGroup.readEntry(qkey, QList<int>());
        foreach(int i, value) {
            ret->push_back(i);
        }
        return true;
    }
    else
        return false;
}

bool KConfigConfig::write(const String& key, const String& value)
{
    if (!valid() || key.empty())
        return false;

    KConfigGroup defaultGroup = m_config->group("SCIM");
    QString qkey = QString::fromUtf8(key.c_str());
    defaultGroup.writeEntry(qkey, QString::fromUtf8(value.c_str()));
    return true;
}

bool KConfigConfig::write(const String& key, int value)
{
    if (!valid() || key.empty())
        return false;

    KConfigGroup defaultGroup = m_config->group("SCIM");
    QString qkey = QString::fromUtf8(key.c_str());
    defaultGroup.writeEntry(qkey, value);
    return true;
}

bool KConfigConfig::write(const String& key, double value)
{
    if (!valid() || key.empty())
        return false;

    KConfigGroup defaultGroup = m_config->group("SCIM");
    QString qkey = QString::fromUtf8(key.c_str());
    defaultGroup.writeEntry(qkey, value);
    return true;
}

bool KConfigConfig::write(const String& key, bool value)
{
    if (!valid() || key.empty())
        return false;

    KConfigGroup defaultGroup = m_config->group("SCIM");
    QString qkey = QString::fromUtf8(key.c_str());
    defaultGroup.writeEntry(qkey, value);
    return true;
}

bool KConfigConfig::write(const String& key, const std::vector <String>& value)
{
    if (!valid() || key.empty())
        return false;

    KConfigGroup defaultGroup = m_config->group("SCIM");
    QString qkey = QString::fromUtf8(key.c_str());
    QStringList list;
    unsigned int i = 0;
    unsigned int c = value.size();
    for (; i < c; ++i) {
        list.append(QString::fromUtf8(value[i].c_str()));
    }
    defaultGroup.writeEntry(qkey, list);
    return true;
}

bool KConfigConfig::write(const String& key, const std::vector <int>& value)
{
    if (!valid() || key.empty())
        return false;

    KConfigGroup defaultGroup = m_config->group("SCIM");
    QString qkey = QString::fromUtf8(key.c_str());
    QList<int> list;
    unsigned int i = 0;
    unsigned int c = value.size();
    for (; i < c; ++i) {
        list.append(value[i]);
    }
    defaultGroup.writeEntry(qkey, list);
    return true;
}

bool KConfigConfig::flush()
{
    if (!valid())
        return false;

    timeval update_timestamp;
    gettimeofday (&update_timestamp, 0);

    char buf[128];
    snprintf(buf, 128, "%lu:%lu", update_timestamp.tv_sec, update_timestamp.tv_usec);
    write(String(SCIM_CONFIG_UPDATE_TIMESTAMP), String(buf));

    m_config->sync();
    return true;
}

bool KConfigConfig::erase(const String& key)
{
    if (!valid() || key.empty())
        return false;

    KConfigGroup defaultGroup = m_config->group("SCIM");
    QString qkey = QString::fromUtf8(key.c_str());
    defaultGroup.deleteEntry(qkey);
    return true;
}

bool KConfigConfig::reload()
{
    if (!valid())
        return false;

    m_config->reparseConfiguration();
    return ConfigBase::reload();
}
