/*
 *  This file is part of KIMToy, an input method frontend for KDE
 *  Copyright (C) 2011 Ni Hui <shuizhuyuanluo@126.com>
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
#include <QDir>
#include <QFile>
#include <QTextStream>

#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KProcess>

#include "statusbar.h"
#include "envsettings.h"
#include "themer.h"

#include "kimtoysettings.h"

static bool isProcessRunning( const KUrl& cmdPath )
{
    return KProcess::execute( "pidof", QStringList() << cmdPath.fileName() ) == 0;
}

static void killProcess( const KUrl& cmdPath )
{
    KProcess::startDetached( "killall", QStringList() << "-9" << cmdPath.fileName() );
}

KIMToy::KIMToy() : KUniqueApplication()
{
//     disableSessionManagement();

//     QApplication::setAttribute( Qt::AA_DontCreateNativeWidgetSiblings );
    QApplication::setQuitOnLastWindowClosed( false );

    m_statusBar = new StatusBar;
    m_statusBar->show();

    QTimer::singleShot( 0, this, SLOT(init()) );
}

KIMToy::~KIMToy()
{
    delete m_statusBar;

//     EnvSettings::load();
    if ( KIMToySettings::self()->runFcitx() ) {
        killProcess( KIMToySettings::self()->fcitxCmd() );
        EnvSettings::setXIM( KIMToySettings::self()->fcitxXIM() );
        EnvSettings::setXIM_PROGRAM( KIMToySettings::self()->fcitxXIM_PROGRAM() );
        EnvSettings::setXMODIFIERS( KIMToySettings::self()->fcitxXMODIFIERS() );
        EnvSettings::setGTK_IM_MODULE( KIMToySettings::self()->fcitxGTK_IM_MODULE() );
        EnvSettings::setQT_IM_MODULE( KIMToySettings::self()->fcitxQT_IM_MODULE() );
        EnvSettings::save();
    }
    else if ( KIMToySettings::self()->runIBus() ) {
        killProcess( KIMToySettings::self()->iBusCmd() );
        EnvSettings::setXIM( KIMToySettings::self()->iBusXIM() );
        EnvSettings::setXIM_PROGRAM( KIMToySettings::self()->iBusXIM_PROGRAM() );
        EnvSettings::setXMODIFIERS( KIMToySettings::self()->iBusXMODIFIERS() );
        EnvSettings::setGTK_IM_MODULE( KIMToySettings::self()->iBusGTK_IM_MODULE() );
        EnvSettings::setQT_IM_MODULE( KIMToySettings::self()->iBusQT_IM_MODULE() );
        EnvSettings::save();
    }
    else if ( KIMToySettings::self()->runSCIM() ) {
        killProcess( KIMToySettings::self()->sCIMCmd() );
//         /// NOTE: workaround here for kimtoy-scim-panel
//         KProcess::startDetached( "killall", QStringList() << "-9" << "kimtoy-scim-panel" );
        EnvSettings::setXIM( KIMToySettings::self()->sCIMXIM() );
        EnvSettings::setXIM_PROGRAM( KIMToySettings::self()->sCIMXIM_PROGRAM() );
        EnvSettings::setXMODIFIERS( KIMToySettings::self()->sCIMXMODIFIERS() );
        EnvSettings::setGTK_IM_MODULE( KIMToySettings::self()->sCIMGTK_IM_MODULE() );
        EnvSettings::setQT_IM_MODULE( KIMToySettings::self()->sCIMQT_IM_MODULE() );
        EnvSettings::save();
    }
    else {
        EnvSettings::unset();
    }

    KIMToySettings::self()->writeConfig();
    kWarning() << "KIMToy::~KIMToy()";
}

void KIMToy::init()
{
    KProcess p;
    KUrl fcitxCmd = KIMToySettings::self()->fcitxCmd();
    KUrl iBusCmd = KIMToySettings::self()->iBusCmd();
    KUrl sCIMCmd = KIMToySettings::self()->sCIMCmd();
    if ( KIMToySettings::self()->runFcitx() && !isProcessRunning( fcitxCmd ) ) {
        QStringList args;
        if ( !KIMToySettings::self()->fcitxArgs().isEmpty() )
            args = KIMToySettings::self()->fcitxArgs().split( ' ' );
        p.setProgram( fcitxCmd.pathOrUrl(), args );
        p.startDetached();
    }
    else if ( KIMToySettings::self()->runIBus() && !isProcessRunning( iBusCmd ) ) {
        QStringList args;
        if ( !KIMToySettings::self()->iBusArgs().isEmpty() )
            args = KIMToySettings::self()->iBusArgs().split( ' ' );
        p.setProgram( iBusCmd.pathOrUrl(), args );
        p.startDetached();
    }
    else if ( KIMToySettings::self()->runSCIM() && !isProcessRunning( sCIMCmd ) ) {
        {
            /// set the default panel program to kimtoy-scim-panel
            QStringList args;
            args << "-c" << "global";
            args << "-s";
            args << QString( "/DefaultPanelProgram=" ) + KStandardDirs::locate( "appdata", "kimtoy-scim-panel" );
            KProcess::execute( "scim-config-agent", args );
        }
        QStringList args;
        if ( !KIMToySettings::self()->sCIMArgs().isEmpty() )
            args = KIMToySettings::self()->sCIMArgs().split( ' ' );
        p.setProgram( sCIMCmd.pathOrUrl(), args );
        p.startDetached();
    }
}
