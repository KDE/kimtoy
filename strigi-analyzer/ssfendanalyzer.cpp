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

#include "ssfendanalyzer.h"

#include <strigi/analysisresult.h>
#include <strigi/analyzerconfiguration.h>
#include <strigi/fieldtypes.h>
#include <strigi/inputstreamreader.h>
#include <strigi/zipinputstream.h>

#include <QByteArray>
#include <QDate>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QTextStream>

const std::string typeFieldName("http://www.w3.org/1999/02/22-rdf-syntax-ns#type");
const std::string fullnameFieldName("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#fullname");
const std::string contactClassName("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#Contact");
const std::string emailAddressClassName("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#EmailAddress");

SsfEndAnalyzer::SsfEndAnalyzer(const SsfEndAnalyzerFactory* f)
        : StreamEndAnalyzer(),factory(f)
{
}

bool SsfEndAnalyzer::checkHeader(const char* header, int32_t headersize) const
{
    return ZipInputStream::checkHeader(header, headersize);
}

signed char SsfEndAnalyzer::analyze(AnalysisResult& idx, InputStream* in)
{
    ZipInputStream zip(in);
    InputStream* s = zip.nextEntry();
    if (zip.status() != Ok) {
        m_error = zip.error();
        return -1;
    }

    idx.addValue(factory->typeField, emailAddressClassName);

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
        if (zip.entryInfo().filename == "skin.ini"
            || zip.entryInfo().filename == "Skin.ini") {
            const char* buf;
            int64_t size = zip.entryInfo().size;
            int32_t nread = s->read(buf, size, size);
            QByteArray data(buf, nread);
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
                    idx.addValue(factory->idField, value.toUtf8().constData());
                }
                else if (key == "skin_name") {
                    idx.addValue(factory->nameField, value.toUtf8().constData());
                }
                else if (key == "skin_version") {
                    idx.addValue(factory->versionField, value.toUtf8().constData());
                }
                else if (key == "skin_author") {
                    std::string skinAuthorUri = idx.newAnonymousUri();
                    idx.addValue(factory->authorField, skinAuthorUri);
                    idx.addTriplet(skinAuthorUri, typeFieldName, contactClassName);
                    idx.addTriplet(skinAuthorUri, fullnameFieldName, value.toUtf8().constData());
                }
                else if (key == "skin_email") {
                    idx.addValue(factory->emailField, value.toUtf8().constData());
                }
                else if (key == "skin_time") {
                    QDateTime dt = QDateTime::fromString(value, "yyyy.MM.dd");
                    idx.addValue(factory->timeField, dt.toTime_t());
                }
                else if (key == "skin_info") {
                    idx.addValue(factory->infoField, value.toUtf8().constData());
                }
            }
            while (!line.isNull());

            break;
        }
        s = zip.nextEntry();
    }

    if (zip.status() == Error) {
        m_error = zip.error();
        return -1;
    }
    else {
        m_error.resize(0);
    }

    return 0;
}

const char* SsfEndAnalyzer::name() const
{
    return "SsfEndAnalyzer";
}

const char* SsfEndAnalyzerFactory::name() const
{
    return "SsfEndAnalyzer";
}

void SsfEndAnalyzerFactory::registerFields(FieldRegister& reg)
{
    idField = reg.registerField("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#identifier");
    nameField = reg.registerField("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title");
    versionField = reg.registerField("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#version");
    authorField = reg.registerField("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator");
    emailField = reg.registerField("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#emailAddress");
    timeField = reg.registerField("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated");
    infoField = reg.registerField("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description");
    typeField = reg.typeField;

    addField(idField);
    addField(nameField);
    addField(versionField);
    addField(authorField);
    addField(emailField);
    addField(timeField);
    addField(infoField);
    addField(typeField);
}

Strigi::StreamEndAnalyzer* SsfEndAnalyzerFactory::newInstance() const
{
    return new SsfEndAnalyzer(this);
}

std::list<StreamEndAnalyzerFactory*> Factory::streamEndAnalyzerFactories() const
{
    std::list<StreamEndAnalyzerFactory*> list;
    list.push_back(new SsfEndAnalyzerFactory);
    return list;
}

STRIGI_ANALYZER_FACTORY( Factory )
