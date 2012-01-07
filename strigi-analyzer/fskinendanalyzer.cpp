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

#include "fskinendanalyzer.h"

#include <strigi/analysisresult.h>
#include <strigi/analyzerconfiguration.h>
#include <strigi/fieldtypes.h>
#include <strigi/inputstreamreader.h>
#include <strigi/gzipinputstream.h>
#include <strigi/tarinputstream.h>

#include <QByteArray>
// #include <QDebug>
#include <QString>
#include <QStringList>
#include <QTextStream>

FskinEndAnalyzer::FskinEndAnalyzer(const FskinEndAnalyzerFactory* f)
        : StreamEndAnalyzer(),factory(f)
{
}

bool FskinEndAnalyzer::checkHeader(const char* header, int32_t headersize) const
{
    /// GZipInputStream::checkHeader() treats header as signed char array
    /// TODO fix it in strigi/libstreams --- nihui
    return headersize > 2 && (unsigned char)header[0] == 0x1f && (unsigned char)header[1] == 0x8b;
}

signed char FskinEndAnalyzer::analyze(AnalysisResult& idx, InputStream* in)
{
    GZipInputStream gz(in);
    TarInputStream tar(&gz);
    InputStream* s = tar.nextEntry();
    if (tar.status() != Ok) {
        m_error = tar.error();
        return -1;
    }

    while (s) {
        // check if done
        int64_t max = idx.config().maximalStreamReadLength(idx);
        if (max != -1 && in->position() > max) {
            return 0;
        }
        // check if the analysis has been aborted
        if (!idx.config().indexMore()) {
            return 0;
        }
        if (tar.entryInfo().filename.find("/fcitx_skin.conf") != std::string::npos) {
            const char* buf;
            int64_t size = tar.entryInfo().size;
            int32_t nread = s->read(buf, size, size);
            QByteArray data(buf, nread);
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

//                 qWarning() << key << value;

                if (key == "Name") {
                    idx.addValue(factory->nameField, value.toUtf8().constData());
                }
                else if (key == "Version") {
                    idx.addValue(factory->versionField, value.toUtf8().constData());
                }
                else if (key == "Author") {
                    idx.addValue(factory->authorField, value.toUtf8().constData());
                }
                else if (key == "Desc") {
                    idx.addValue(factory->descField, value.toUtf8().constData());
                }
            }
            while (!line.isNull());

            break;
        }
        s = tar.nextEntry();
    }

    if (tar.status() == Error) {
        m_error = tar.error();
        return -1;
    }
    else {
        m_error.resize(0);
    }

    return 0;
}

const char* FskinEndAnalyzer::name() const
{
    return "FskinEndAnalyzer";
}

const char* FskinEndAnalyzerFactory::name() const
{
    return "FskinEndAnalyzer";
}

void FskinEndAnalyzerFactory::registerFields(FieldRegister& reg)
{
    nameField = reg.registerField("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title");
    versionField = reg.registerField("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#version");
    authorField = reg.registerField("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#fullname");
    descField = reg.registerField("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description");
    typeField = reg.typeField;

    addField(nameField);
    addField(versionField);
    addField(authorField);
    addField(descField);
    addField(typeField);
}

Strigi::StreamEndAnalyzer* FskinEndAnalyzerFactory::newInstance() const
{
    return new FskinEndAnalyzer(this);
}

std::list<StreamEndAnalyzerFactory*> Factory::streamEndAnalyzerFactories() const
{
    std::list<StreamEndAnalyzerFactory*> list;
    list.push_back(new FskinEndAnalyzerFactory);
    return list;
}

STRIGI_ANALYZER_FACTORY( Factory )
