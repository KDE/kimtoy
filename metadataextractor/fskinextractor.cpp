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

#include "fskinextractor.h"

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <KTar>

FskinExtractor::FskinExtractor(QObject* parent)
        : KFileMetaData::ExtractorPlugin(parent)
{
}

QStringList FskinExtractor::mimetypes() const
{
    QStringList types;
    types << QStringLiteral("application/x-fskin");
    return types;
}

void FskinExtractor::extract(KFileMetaData::ExtractionResult* result)
{
    QString file = result->inputUrl();
    if (!QFile::exists(file))
        return;

    KTar tar(file);
    if (!tar.open(QIODevice::ReadOnly))
        return;

    QStringList entries = tar.directory()->entries();
    if (entries.count() != 1)
        return;

    const KArchiveEntry* entry = tar.directory()->entry(entries.first());
    if (!entry->isDirectory())
        return;

    const KArchiveDirectory* subdir = static_cast<const KArchiveDirectory*>(entry);
    const KArchiveEntry* skinconf_entry = subdir->entry("fcitx_skin.conf");
    const KArchiveFile* skinconf = static_cast<const KArchiveFile*>(skinconf_entry);
    if (!skinconf)
        return;

    result->addType(KFileMetaData::Type::Archive);

    QByteArray data = skinconf->data();

    QTextStream ss(data);
    QString line;
    QString key, value;
    bool skininfo = false;
    do {
        line = ss.readLine();
        if (line.isEmpty() || line.at(0) == '#')
            continue;

        if (line.at(0) == '[') {
            skininfo = (line == "[SkinInfo]");
            continue;
        }

        if (!skininfo)
            continue;

        key = line.split('=').at(0);
        value = line.split('=').at(1);

        if (value.isEmpty())
            continue;

        if (key == "Name") {
            result->add(KFileMetaData::Property::Title, value);
        }
        else if (key == "Version") {
//             result->add(KFileMetaData::Property::Subject, value);//TODO
        }
        else if (key == "Author") {
            result->add(KFileMetaData::Property::Author, value);
        }
        else if (key == "Desc") {
            result->add(KFileMetaData::Property::Subject, value);
        }
    }
    while (!line.isNull());
}
