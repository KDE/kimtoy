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

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <KAboutData>
#include <KDBusService>
#include <KLocalizedString>

#include "kimtoy.h"

int main(int argc, char** argv)
{
    KIMToy app(argc, argv);

    KLocalizedString::setApplicationDomain("kimtoy");

    KAboutData aboutData("kimtoy", i18n("KIMToy"),
                         "1.91", i18n("The KDE input method toy"),
                         KAboutLicense::GPL, i18n("(c) 2011-2016, Ni Hui"));
    aboutData.addAuthor(i18n("Ni Hui"), i18n("Author"), "shuizhuyuanluo@126.com");
    aboutData.setOrganizationDomain(QByteArray("kde.org"));
    aboutData.setProductName(QByteArray("kimtoy"));

    KAboutData::setApplicationData(aboutData);

    app.setApplicationName(aboutData.componentName());
    app.setOrganizationDomain(aboutData.organizationDomain());
    app.setApplicationVersion(aboutData.version());
    app.setApplicationDisplayName(aboutData.displayName());
    app.setWindowIcon(QIcon::fromTheme("kimtoy"));

    KDBusService dbusService(KDBusService::Unique);

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    app.newInstance();

    return app.exec();
}
