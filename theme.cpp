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

#include "theme.h"

#include <QFileDialog>
#include <KIO/CopyJob>
#include <KMessageBox>
#include <KNS3/DownloadDialog>

#include "kimtoysettings.h"

void ThemeWidget::installTheme()
{
    QString filePath = QFileDialog::getOpenFileName();

    if (filePath.isEmpty())
        return;

    if (!filePath.endsWith(".ssf") && !filePath.endsWith(".fskin")) {
        KMessageBox::error(this, i18n("Unsupported theme type."));
        return;
    }

    QUrl destUrl = KIMToySettings::self()->themeFolder();

    if (filePath.startsWith(destUrl.path())) {
        KMessageBox::error(this, i18n("This theme has already been installed."));
        return;
    }

    KJob* job = KIO::copy(QUrl::fromUserInput(filePath, QDir::currentPath(), QUrl::AssumeLocalFile), destUrl, KIO::HideProgressInfo);
    connect(job, SIGNAL(finished(KJob*)), kcfg_ThemeUri, SLOT(reload()));
}

void ThemeWidget::downloadTheme()
{
    KNS3::DownloadDialog dialog;
    dialog.exec();
    if (!dialog.changedEntries().isEmpty()) {
        kcfg_ThemeUri->reload();
    }
}
