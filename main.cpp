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

#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>
#include <KLocale>

#include "kimtoy.h"

int main(int argc, char** argv)
{
    KAboutData aboutData("kimtoy", 0, ki18n("KIMToy"),
                         "1.8", ki18n("The KDE input method toy"),
                         KAboutData::License_GPL, ki18n("(c) 2011-2012, Ni Hui"));
    aboutData.addAuthor(ki18n("Ni Hui"), ki18n("Author"), "shuizhuyuanluo@126.com");
    aboutData.setProgramIconName("draw-freehand");

    KCmdLineArgs::init(argc, argv, &aboutData);

    if (!KIMToy::start()) {
        kWarning() << "kimtoy is already running!";
        return 1;
    }

    KIMToy* kimtoy = new KIMToy;
    int ret = kimtoy->exec();
    delete kimtoy;
    kWarning() << ret;
    return ret;
}
