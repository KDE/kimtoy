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

#include "ssfextractor.h"

#include <QByteArray>
#include <QDate>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include "../kssf.h"

SsfExtractor::SsfExtractor(QObject* parent)
        : KFileMetaData::ExtractorPlugin(parent)
{
}

QStringList SsfExtractor::mimetypes() const
{
    QStringList types;
    types << QStringLiteral("application/x-ssf");
    return types;
}

void SsfExtractor::extract(KFileMetaData::ExtractionResult* result)
{
    QString file = result->inputUrl();
    if (!QFile::exists(file))
        return;

    KSsf ssf(file);
    if (!ssf.open(QIODevice::ReadOnly))
        return;

    const KArchiveEntry* entry = ssf.directory()->entry("skin.ini");
    const KArchiveFile* skinini = static_cast<const KArchiveFile*>(entry);

    if (!skinini) {
        entry = ssf.directory()->entry("Skin.ini");
        skinini = static_cast<const KArchiveFile*>(entry);
        if (!skinini)
            return;
    }

    result->addType(KFileMetaData::Type::Archive);

    QByteArray data = skinini->data();

    QTextStream ss(data);
    QString line;
    QString key, value;
    bool general = false;
    do {
        line = ss.readLine();
        if (line.isEmpty())
            continue;

        if (line.at(0) == '[') {
            general = (line == "[General]");
            continue;
        }

        if (!general)
            continue;

        key = line.split('=').at(0);
        value = line.split('=').at(1);

        if (value.isEmpty())
            continue;

        if (key == "skin_id") {
//             result->add(KFileMetaData::Property::Subject, value);//TODO
        }
        else if (key == "skin_name") {
            result->add(KFileMetaData::Property::Title, value);
        }
        else if (key == "skin_version") {
//             result->add(KFileMetaData::Property::Subject, value);//TODO
        }
        else if (key == "skin_author") {
            result->add(KFileMetaData::Property::Author, value);
        }
        else if (key == "skin_email") {
//             result->add(KFileMetaData::Property::Subject, value);//TODO
        }
        else if (key == "skin_time") {
            result->add(KFileMetaData::Property::CreationDate, value);
        }
        else if (key == "skin_info") {
            result->add(KFileMetaData::Property::Subject, value);
        }
    }
    while (!line.isNull());
}
