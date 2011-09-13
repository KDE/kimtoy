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

#include "envsettings.h"

#include <QFile>

#include <KComponentData>
#include <KGlobal>
#include <KStandardDirs>

class EnvSettingsPrivate
{
    public:
        static EnvSettingsPrivate* self();
        ~EnvSettingsPrivate();
        void load();
        void save();
        void unset();
        QString XIM;
        QString XIM_PROGRAM;
        QString XMODIFIERS;
        QString GTK_IM_MODULE;
        QString QT_IM_MODULE;
    private:
        EnvSettingsPrivate();
        static EnvSettingsPrivate* m_self;
};

EnvSettingsPrivate* EnvSettingsPrivate::m_self = 0;

EnvSettingsPrivate* EnvSettingsPrivate::self()
{
    if ( !m_self )
        m_self = new EnvSettingsPrivate;
    return m_self;
}

EnvSettingsPrivate::EnvSettingsPrivate()
{
}

EnvSettingsPrivate::~EnvSettingsPrivate()
{
}

void EnvSettingsPrivate::load()
{
    QFile script( KGlobal::mainComponent().dirs()->localkdedir() + "env/" + "kimtoy-env.sh" );
    if ( !script.exists() ) {
        // try to get system defaults
        XIM = qgetenv( "XIM" );
        XIM_PROGRAM = qgetenv( "XIM_PROGRAM" );
        XMODIFIERS = qgetenv( "XMODIFIERS" );
        GTK_IM_MODULE = qgetenv( "GTK_IM_MODULE" );
        QT_IM_MODULE = qgetenv( "QT_IM_MODULE" );
        return;
    }
    if ( !script.open( QIODevice::ReadOnly | QIODevice::Text ) )
        return;

    while ( !script.atEnd() ) {
         QByteArray line = script.readLine();
         QString value = QString::fromUtf8( line.split( '\"' ).value( 1 ) );
         if ( line.startsWith( "export XIM=" ) )
             XIM = value;
         else if ( line.startsWith( "export XIM_PROGRAM=" ) )
             XIM_PROGRAM = value;
         else if ( line.startsWith( "export XMODIFIERS=" ) )
             XMODIFIERS = value;
         else if ( line.startsWith( "export GTK_IM_MODULE=" ) )
             GTK_IM_MODULE = value;
         else if ( line.startsWith( "export QT_IM_MODULE=" ) )
             QT_IM_MODULE = value;
     }
    script.close();
}

void EnvSettingsPrivate::save()
{
    QFile script( KGlobal::mainComponent().dirs()->localkdedir() + "env/" + "kimtoy-env.sh" );
    if ( !script.open( QIODevice::WriteOnly | QIODevice::Text ) )
        return;

    QString t( "export %1=\"%2\"\n" );
    script.write( t.arg( "XIM" ).arg( XIM ).toUtf8() );
    script.write( t.arg( "XIM_PROGRAM" ).arg( XIM_PROGRAM ).toUtf8() );
    script.write( t.arg( "XMODIFIERS" ).arg( XMODIFIERS ).toUtf8() );
    script.write( t.arg( "GTK_IM_MODULE" ).arg( GTK_IM_MODULE ).toUtf8() );
    script.write( t.arg( "QT_IM_MODULE" ).arg( QT_IM_MODULE ).toUtf8() );
    script.close();
}

void EnvSettingsPrivate::unset()
{
    QFile script( KGlobal::mainComponent().dirs()->localkdedir() + "env/" + "kimtoy-env.sh" );
    script.remove();
}

void EnvSettings::load()
{
    EnvSettingsPrivate::self()->load();
}

void EnvSettings::save()
{
    EnvSettingsPrivate::self()->save();
}

void EnvSettings::unset()
{
    EnvSettingsPrivate::self()->unset();
}

void EnvSettings::setXIM( const QString& env )
{
    EnvSettingsPrivate::self()->XIM = env;
}

QString EnvSettings::XIM()
{
    return EnvSettingsPrivate::self()->XIM;
}

void EnvSettings::setXIM_PROGRAM( const QString& env )
{
    EnvSettingsPrivate::self()->XIM_PROGRAM = env;
}

QString EnvSettings::XIM_PROGRAM()
{
    return EnvSettingsPrivate::self()->XIM_PROGRAM;
}

void EnvSettings::setXMODIFIERS( const QString& env )
{
    EnvSettingsPrivate::self()->XMODIFIERS = env;
}

QString EnvSettings::XMODIFIERS()
{
    return EnvSettingsPrivate::self()->XMODIFIERS;
}

void EnvSettings::setGTK_IM_MODULE( const QString& env )
{
    EnvSettingsPrivate::self()->GTK_IM_MODULE = env;
}

QString EnvSettings::GTK_IM_MODULE()
{
    return EnvSettingsPrivate::self()->GTK_IM_MODULE;
}

void EnvSettings::setQT_IM_MODULE( const QString& env )
{
    EnvSettingsPrivate::self()->QT_IM_MODULE = env;
}

QString EnvSettings::QT_IM_MODULE()
{
    return EnvSettingsPrivate::self()->QT_IM_MODULE;
}
