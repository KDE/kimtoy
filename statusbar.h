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

#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QWidget>
#include <QHash>
#include <QTimer>

class QPushButton;
class QSignalMapper;
class KStatusNotifierItem;

class PreEditBar;
class PropertyWidget;
class StatusBarLayout;
// class Themer;
class ThemerFcitx;
class ThemerNone;
class ThemerPlasma;
class ThemerSogou;

class StatusBar : public QWidget
{
    Q_OBJECT
public:
    explicit StatusBar();
    virtual ~StatusBar();
protected:
    virtual bool eventFilter(QObject* object, QEvent* event);
    virtual void resizeEvent(QResizeEvent* event);
    virtual void paintEvent(QPaintEvent* event);
    virtual void showEvent(QShowEvent* event);
private Q_SLOTS:
    void slotEnable(bool enable);
    void slotSetVisibleDelayed();
    void slotTriggerProperty(const QString& objectPath);
    void slotRegisterProperties(const QStringList& props);
    void slotUpdateProperty(const QString& prop);
    void slotRemoveProperty(const QString& prop);
    void slotExecDialog(const QString& prop);
    void slotExecMenu(const QStringList& actions);
private Q_SLOTS:
    void slotAutostartToggled(bool enable);
    void slotTrayiconModeToggled(bool enable);
    void slotConfigureIMTriggered();
    void preferences();
    void slotAboutActionTriggered();
    void loadSettings();
    void slotFilterChanged(const QString& objectPath, bool checked);
    void slotFilterMenuDestroyed();
    void slotConnectKIMPanel();
    void slotDisconnectKIMPanel();
private:
    void updateSize();
    void showFilterMenu();
private:
//         friend class Themer;
    friend class ThemerFcitx;
    friend class ThemerNone;
    friend class ThemerPlasma;
    friend class ThemerSogou;
    PreEditBar* m_preeditBar;
    KStatusNotifierItem* m_tray;
    QPoint m_pointPos;
    bool m_rmbdown;
    bool m_moving;
    QHash<QString, PropertyWidget*> m_propertyWidgets;
    QHash<QString, KStatusNotifierItem*> m_trayWidgets;
    QSignalMapper* m_signalMapper;
    StatusBarLayout* m_layout;
    QStringList m_filters;
    bool m_visible;
    QTimer m_visibleDelayer;
};

#endif // STATUSBAR_H
