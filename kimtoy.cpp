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

#include "kimtoy.h"

#include <QTimer>

#include "envsettings.h"
#include "inputmethods.h"
#include "statusbar.h"
#include "themer.h"

#include "kimtoysettings.h"

KIMToy::KIMToy(int& argc, char** argv) : QApplication(argc, argv)
{
//     QApplication::setAttribute( Qt::AA_DontCreateNativeWidgetSiblings );
    QApplication::setQuitOnLastWindowClosed(false);

    m_statusBar = 0;
}

KIMToy::~KIMToy()
{
    delete m_statusBar;

//     EnvSettings::load();
    if (KIMToySettings::self()->runFcitx()) {
        FcitxInputMethod::self()->kill();
        FcitxInputMethod::self()->saveEnvSettings();
    }
    else if (KIMToySettings::self()->runIBus()) {
        IBusInputMethod::self()->kill();
        IBusInputMethod::self()->saveEnvSettings();
    }
    else if (KIMToySettings::self()->runSCIM()) {
        SCIMInputMethod::self()->kill();
        SCIMInputMethod::self()->saveEnvSettings();
    }
    else {
        EnvSettings::unset();
    }

    KIMToySettings::self()->writeConfig();
}

void KIMToy::newInstance()
{
    if (!m_statusBar) {
        m_statusBar = new StatusBar;
        QTimer::singleShot(0, this, SLOT(init()));
    }
}

void KIMToy::init()
{
    if (KIMToySettings::self()->runFcitx()) {
        FcitxInputMethod::self()->run();
    }
    else if (KIMToySettings::self()->runIBus()) {
        IBusInputMethod::self()->run();
    }
    else if (KIMToySettings::self()->runSCIM()) {
        SCIMInputMethod::self()->run();
    }
}
