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

#include "inputmethods.h"

#include <QByteArray>
#include <QString>
#include <QStringList>

#include <KDebug>
#include <KProcess>

#include "envsettings.h"

#include "kimtoysettings.h"

static bool isProcessRunning(const KUrl& cmdPath)
{
    return KProcess::execute("ps", QStringList() << "-C" << cmdPath.fileName()) == 0;
}

static void killProcess(const KUrl& cmdPath)
{
    KProcess::startDetached("killall", QStringList() << "-9" << cmdPath.fileName());
}

FcitxInputMethod* FcitxInputMethod::m_self = 0;

FcitxInputMethod* FcitxInputMethod::self()
{
    if (!m_self)
        m_self = new FcitxInputMethod;
    return m_self;
}

FcitxInputMethod::FcitxInputMethod()
        : InputMethod()
{
}

FcitxInputMethod::~FcitxInputMethod()
{
}

void FcitxInputMethod::run()
{
    KUrl fcitxCmd = KIMToySettings::self()->fcitxCmd();
    if (isProcessRunning(fcitxCmd)) {
        return;
    }

    QStringList args;
    if (!KIMToySettings::self()->fcitxArgs().isEmpty())
        args = KIMToySettings::self()->fcitxArgs().split(' ');

    KProcess p;
    p.setProgram(fcitxCmd.pathOrUrl(), args);
    p.startDetached();
}

void FcitxInputMethod::kill()
{
    killProcess(KIMToySettings::self()->fcitxCmd());
}

void FcitxInputMethod::saveEnvSettings() const
{
    EnvSettings::setXIM(KIMToySettings::self()->fcitxXIM());
    EnvSettings::setXIM_PROGRAM(KIMToySettings::self()->fcitxXIM_PROGRAM());
    EnvSettings::setXMODIFIERS(KIMToySettings::self()->fcitxXMODIFIERS());
    EnvSettings::setGTK_IM_MODULE(KIMToySettings::self()->fcitxGTK_IM_MODULE());
    EnvSettings::setQT_IM_MODULE(KIMToySettings::self()->fcitxQT_IM_MODULE());
    EnvSettings::save();
}

bool FcitxInputMethod::getVersion(QString& version) const
{
    KProcess p;
    p.setReadChannel(QProcess::StandardOutput);
    p.setOutputChannelMode(KProcess::SeparateChannels);
    p.setProgram(KIMToySettings::self()->fcitxCmd().pathOrUrl(), QStringList() << "-v");
    p.start();
    p.waitForFinished();
    QByteArray data = p.readAllStandardOutput();

    data = data.split('\n').first();
    if (data.isEmpty())
        return false;

    // "fcitx version: 4.1.1"
    if (!data.startsWith("fcitx version: "))
        return false;
    version = data.mid(15);
    return true;
}


IBusInputMethod* IBusInputMethod::m_self = 0;

IBusInputMethod* IBusInputMethod::self()
{
    if (!m_self)
        m_self = new IBusInputMethod;
    return m_self;
}

IBusInputMethod::IBusInputMethod()
        : InputMethod()
{
}

IBusInputMethod::~IBusInputMethod()
{
}

void IBusInputMethod::run()
{
    KUrl iBusCmd = KIMToySettings::self()->iBusCmd();

    QStringList args;
    if (!KIMToySettings::self()->iBusArgs().isEmpty())
        args = KIMToySettings::self()->iBusArgs().split(' ');

    if (isProcessRunning(iBusCmd) && !args.contains("-r")) {
        return;
    }

    KProcess p;
    p.setProgram(iBusCmd.pathOrUrl(), args);
    p.startDetached();
}

void IBusInputMethod::kill()
{
    killProcess(KIMToySettings::self()->iBusCmd());
}

void IBusInputMethod::saveEnvSettings() const
{
    EnvSettings::setXIM(KIMToySettings::self()->iBusXIM());
    EnvSettings::setXIM_PROGRAM(KIMToySettings::self()->iBusXIM_PROGRAM());
    EnvSettings::setXMODIFIERS(KIMToySettings::self()->iBusXMODIFIERS());
    EnvSettings::setGTK_IM_MODULE(KIMToySettings::self()->iBusGTK_IM_MODULE());
    EnvSettings::setQT_IM_MODULE(KIMToySettings::self()->iBusQT_IM_MODULE());
    EnvSettings::save();
}

bool IBusInputMethod::getVersion(QString& version) const
{
    KProcess p;
    p.setReadChannel(QProcess::StandardOutput);
    p.setOutputChannelMode(KProcess::SeparateChannels);
    p.setProgram(KIMToySettings::self()->iBusCmd().pathOrUrl(), QStringList() << "-V");
    p.start();
    p.waitForFinished();
    QByteArray data = p.readAllStandardOutput();

    data = data.split('\n').first();
    if (data.isEmpty())
        return false;

    // "ibus-daemon - Version 1.3.99.20110817"
    if (!data.startsWith("ibus-daemon - Version "))
        return false;
    version = data.mid(22);
    return true;
}


SCIMInputMethod* SCIMInputMethod::m_self = 0;

SCIMInputMethod* SCIMInputMethod::self()
{
    if (!m_self)
        m_self = new SCIMInputMethod;
    return m_self;
}

SCIMInputMethod::SCIMInputMethod()
        : InputMethod()
{
}

SCIMInputMethod::~SCIMInputMethod()
{
}

void SCIMInputMethod::run()
{
    KUrl sCIMCmd = KIMToySettings::self()->sCIMCmd();
    if (isProcessRunning(sCIMCmd)) {
        return;
    }

    {
        /// set the default panel program to kimtoy-scim-panel
        QStringList args;
        args << "-c" << "global";
        args << "-s";
        args << "/DefaultPanelProgram=scim-panel-impanel";
        KProcess::execute("scim-config-agent", args);
    }
    QStringList args;
    if (!KIMToySettings::self()->sCIMArgs().isEmpty())
        args = KIMToySettings::self()->sCIMArgs().split(' ');

    KProcess p;
    p.setProgram(sCIMCmd.pathOrUrl(), args);
    p.startDetached();
}

void SCIMInputMethod::kill()
{
    killProcess(KIMToySettings::self()->sCIMCmd());
}

void SCIMInputMethod::saveEnvSettings() const
{
    EnvSettings::setXIM(KIMToySettings::self()->sCIMXIM());
    EnvSettings::setXIM_PROGRAM(KIMToySettings::self()->sCIMXIM_PROGRAM());
    EnvSettings::setXMODIFIERS(KIMToySettings::self()->sCIMXMODIFIERS());
    EnvSettings::setGTK_IM_MODULE(KIMToySettings::self()->sCIMGTK_IM_MODULE());
    EnvSettings::setQT_IM_MODULE(KIMToySettings::self()->sCIMQT_IM_MODULE());
    EnvSettings::save();
}

bool SCIMInputMethod::getVersion(QString& version) const
{
    KProcess p;
    p.setReadChannel(QProcess::StandardOutput);
    p.setOutputChannelMode(KProcess::SeparateChannels);
    p.setProgram(KIMToySettings::self()->sCIMCmd().pathOrUrl(), QStringList() << "-v");
    p.start();
    p.waitForFinished();
    QByteArray data = p.readAllStandardOutput();

    data = data.split('\n').first();
    if (data.isEmpty())
        return false;

    // "Smart Common Input Method 1.4.10"
    if (!data.startsWith("Smart Common Input Method "))
        return false;
    version = data.mid(26);
    return true;
}
