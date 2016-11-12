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

#include "installskinfileitemaction.h"

#include <QAction>
#include <QCoreApplication>
// #include <QDebug>
#include <QEventLoop>

#include <KConfig>
#include <KConfigGroup>
#include <KFileItemListProperties>
#include <KIO/CopyJob>
#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(InstallSkinFileItemActionFactory, "installskinfileitemaction.json", registerPlugin<InstallSkinFileItemAction>();)

InstallSkinFileItemAction::InstallSkinFileItemAction(QObject* parent, const QVariantList& args)
        : KAbstractFileItemActionPlugin(parent)
{
}

QList<QAction*> InstallSkinFileItemAction::actions(const KFileItemListProperties& fileItemInfos, QWidget* parentWidget)
{
    QList<QAction*> actions;

    QList<QUrl> supportedUrls;
    foreach (const QUrl& url, fileItemInfos.urlList()) {
        QString fileName = url.fileName();
        if (!fileName.endsWith(".ssf") && !fileName.endsWith(".fskin")) {
            continue;
        }

        supportedUrls << url;
    }

    if (supportedUrls.isEmpty()) {
        return actions;
    }

    QAction* action = new QAction(QIcon::fromTheme("kimtoy"), i18n("Install skin"), parentWidget);
    action->setData(QVariant::fromValue(supportedUrls));
    connect(action, SIGNAL(triggered()), this, SLOT(installSkin()));

    actions << action;
    return actions;
}

void InstallSkinFileItemAction::installSkin()
{
    QAction* action = static_cast<QAction*>(sender());
    QList<QUrl> supportedUrls = action->data().value< QList<QUrl> >();

    // fool QStandardPaths as kimtoy application
    QString realorg = QCoreApplication::organizationName();
    QString realapp = QCoreApplication::applicationName();
    QCoreApplication::setOrganizationName(QString());
    QCoreApplication::setApplicationName("kimtoy");
    QString defaultFolder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/themes/";
    QCoreApplication::setOrganizationName(realorg);
    QCoreApplication::setApplicationName(realapp);

    KConfig config("kimtoyrc");
    QString folder = config.group("theme").readEntry("ThemeFolder", defaultFolder);
    QUrl destUrl = QUrl::fromLocalFile(folder);
//     qWarning() << destUrl;

    KJob* job = KIO::copy(supportedUrls, destUrl, KIO::HideProgressInfo);

    // synchronous copy
    QEventLoop loop;
    connect(job, SIGNAL(finished(KJob*)), &loop, SLOT(quit()));
    loop.exec();
}

#include "installskinfileitemaction.moc"
