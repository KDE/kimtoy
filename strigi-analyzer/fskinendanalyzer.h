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

#ifndef FSKINENDANALYZER_H
#define FSKINENDANALYZER_H

#include <strigi/analyzerplugin.h>
#include <strigi/streamendanalyzer.h>

using namespace Strigi;

class FskinEndAnalyzerFactory;

class FskinEndAnalyzer : public StreamEndAnalyzer
{
public:
    explicit FskinEndAnalyzer(const FskinEndAnalyzerFactory* f);
    virtual bool checkHeader(const char* header, int32_t headersize) const;
    virtual signed char analyze(AnalysisResult& idx, InputStream* in);
    virtual const char* name() const;
private:
    const FskinEndAnalyzerFactory* const factory;
};

class FskinEndAnalyzerFactory : public StreamEndAnalyzerFactory
{
public:
    virtual const char* name() const;
    virtual void registerFields(FieldRegister& reg);
    virtual StreamEndAnalyzer* newInstance() const;
private:
    friend class FskinEndAnalyzer;
    const RegisteredField* nameField;
    const RegisteredField* versionField;
    const RegisteredField* authorField;
    const RegisteredField* descField;
    const RegisteredField* typeField;
};

class Factory : public AnalyzerFactoryFactory
{
public:
    virtual std::list<StreamEndAnalyzerFactory*> streamEndAnalyzerFactories() const;
};

#endif // FSKINENDANALYZER_H
