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

#ifndef INPUTMETHOD_H
#define INPUTMETHOD_H

#include "ui_inputmethod.h"

#include <KIconLoader>

#include "inputmethods.h"

class InputMethodWidget : public QWidget, public Ui::InputMethod
{
public:
    explicit InputMethodWidget() {
        setupUi(this);

        QString version;
        bool ok;

        ok = FcitxInputMethod::self()->getVersion(version);
        kcfg_RunFcitx->setEnabled(ok);
        if (ok) {
            FcitxVersionWidget->setText(i18n("Found version: %1", version));
            FcitxVersionWidget->setPixmap(MainBarIcon("flag-green"));
        }
        else {
            FcitxVersionWidget->setText(i18n("Not found"));
            FcitxVersionWidget->setPixmap(MainBarIcon("flag-red"));
        }

        ok = IBusInputMethod::self()->getVersion(version);
        kcfg_RunIBus->setEnabled(ok);
        if (ok) {
            IBusVersionWidget->setText(i18n("Found version: %1", version));
            IBusVersionWidget->setPixmap(MainBarIcon("flag-green"));
        }
        else {
            IBusVersionWidget->setText(i18n("Not found"));
            IBusVersionWidget->setPixmap(MainBarIcon("flag-red"));
        }

        ok = SCIMInputMethod::self()->getVersion(version);
        kcfg_RunSCIM->setEnabled(ok);
        if (ok) {
            SCIMVersionWidget->setText(i18n("Found version: %1", version));
            SCIMVersionWidget->setPixmap(MainBarIcon("flag-green"));
        }
        else {
            SCIMVersionWidget->setText(i18n("Not found"));
            SCIMVersionWidget->setPixmap(MainBarIcon("flag-red"));
        }
    }
};

#endif // INPUTMETHOD_H
