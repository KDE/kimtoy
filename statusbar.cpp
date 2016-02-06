/*
 *  This file is part of KIMToy, an input method frontend for KDE
 *  Copyright (C) 2011-2015 Ni Hui <shuizhuyuanluo@126.com>
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

#include "statusbar.h"

#include <QAction>
#include <QDBusConnection>
#include <QDebug>
#include <QIcon>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QSignalMapper>

#include <KAboutApplicationDialog>
#include <KAboutData>
#include <KConfig>
#include <KConfigDialog>
#include <KConfigGroup>
// #include <KMenu>
#include <KMessageBox>
#include <KStandardAction>
#include <KStatusNotifierItem>
#include <KToggleAction>
#include <KWindowSystem>

#include "animator.h"
#include "filtermenu.h"
#include "impanelagent.h"
#include "propertywidget.h"
#include "preeditbar.h"
#include "statusbarlayout.h"
#include "themeragent.h"

#include "kimtoysettings.h"
#include "inputmethod.h"
#include "appearance.h"
#include "theme.h"
#include "performance.h"

static void extractProperty(const QString& str,
                            QString& objectPath,
                            QString& name,
                            QString& iconName,
                            QString& description)
{
    const QStringList list = str.split(':');
    objectPath = list.at(0);
    name = list.at(1);
    iconName = list.at(2);
    description = list.at(3);
}

StatusBar::StatusBar()
{
    ThemerAgent::loadSettings();

    bool enableTransparency = KIMToySettings::self()->backgroundTransparency();
    setAttribute(Qt::WA_TranslucentBackground, enableTransparency);

    setAttribute(Qt::WA_AlwaysShowToolTips, true);

    m_preeditBar = new PreEditBar;

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove);
    KWindowSystem::setType(winId(), NET::Dock);

    m_tray = new KStatusNotifierItem(this);
    m_tray->setAssociatedWidget(m_tray->contextMenu());
    m_tray->setIconByName("draw-freehand");
    m_tray->setTitle(i18n("KIMToy"));
    m_tray->setToolTipIconByName("draw-freehand");
    m_tray->setToolTipTitle(i18n("KIMToy"));
    m_tray->setToolTipSubTitle(i18n("Input method toy"));
    m_tray->setCategory(KStatusNotifierItem::ApplicationStatus);
    m_tray->setStatus(KStatusNotifierItem::Active);

    KToggleAction* autostartAction = new KToggleAction(i18n("A&utostart"), this);
    autostartAction->setChecked(KIMToySettings::self()->autostartKIMToy());
    connect(autostartAction, SIGNAL(toggled(bool)), this, SLOT(slotAutostartToggled(bool)));
    m_tray->contextMenu()->addAction(autostartAction);

    KToggleAction* trayiconModeAction = new KToggleAction(i18n("&Trayicon mode"), this);
    trayiconModeAction->setChecked(KIMToySettings::self()->trayiconMode());
    connect(trayiconModeAction, SIGNAL(toggled(bool)), this, SLOT(slotTrayiconModeToggled(bool)));
    m_tray->contextMenu()->addAction(trayiconModeAction);

    QAction* configureIMAction = new QAction(QIcon::fromTheme("preferences-desktop-keyboard"), i18n("C&onfigure input method..."), this);
    connect(configureIMAction, SIGNAL(triggered()), this, SLOT(slotConfigureIMTriggered()));
    m_tray->contextMenu()->addAction(configureIMAction);

    QAction* prefAction = KStandardAction::preferences(this, SLOT(preferences()), 0);
    m_tray->contextMenu()->addAction(prefAction);

    QAction* aboutAction = new QAction(QIcon::fromTheme("draw-freehand"), i18n("&About KIMToy..."), this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(slotAboutActionTriggered()));
    m_tray->contextMenu()->addAction(aboutAction);

    m_signalMapper = new QSignalMapper(this);
    connect(m_signalMapper, SIGNAL(mapped(QString)),
            this, SLOT(slotTriggerProperty(QString)));

    m_layout = new StatusBarLayout;
    setLayout(m_layout);

    installEventFilter(this);

    m_rmbdown = false;
    m_moving = false;

    m_visible = !KIMToySettings::self()->trayiconMode();
    m_visibleDelayer.setSingleShot(true);
    connect(&m_visibleDelayer, SIGNAL(timeout()), this, SLOT(slotSetVisibleDelayed()));

    slotConnectKIMPanel();

    KConfigGroup group(KSharedConfig::openConfig(), "General");
    QPoint pos = group.readEntry("XYPosition", QPoint(100, 0));
    move(pos);

    m_filters = group.readEntry("Filters", QStringList());

    connect(Animator::self(), SIGNAL(animateStatusBar()), this, SLOT(update()));
    connect(Animator::self(), SIGNAL(animatePreEditBar()), m_preeditBar, SLOT(update()));

    loadSettings();

    IMPanelAgent::PanelCreated();

    m_visibleDelayer.start(100);
}

StatusBar::~StatusBar()
{
    KConfigGroup group(KSharedConfig::openConfig(), "General");
    group.writeEntry("XYPosition", pos());
    group.writeEntry("Filters", m_filters);
    delete m_preeditBar;
    qDeleteAll(m_propertyWidgets);
    m_propertyWidgets.clear();
    qDeleteAll(m_trayWidgets);
    m_trayWidgets.clear();
    IMPanelAgent::Exit();
}

bool StatusBar::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            QWidget* w = static_cast<QWidget*>(object);
            if (w == this)
                m_pointPos = mouseEvent->pos();
            else
                m_pointPos = w->mapToParent(mouseEvent->pos());
            m_rmbdown = true;
            return true;
        }
        return QObject::eventFilter(object, event);
    }
    if (event->type() == QEvent::MouseMove && m_rmbdown) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        move(mouseEvent->globalPos() - m_pointPos);
        m_moving = true;
        return true;
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            if (!m_moving) {
                QWidget* w = static_cast<QWidget*>(object);
                if (w == this)
                    m_pointPos = mouseEvent->pos();
                else
                    m_pointPos = w->mapToParent(mouseEvent->pos());
                showFilterMenu();
            }
            m_rmbdown = false;
            m_moving = false;
            return true;
        }
        return QObject::eventFilter(object, event);
    }
    return QObject::eventFilter(object, event);
}

void StatusBar::resizeEvent(QResizeEvent* event)
{
    ThemerAgent::resizeStatusBar(event->size());
    if (KIMToySettings::self()->enableWindowMask()) {
        ThemerAgent::maskStatusBar(this);
    }
    if (KIMToySettings::self()->enableBackgroundBlur()) {
        ThemerAgent::blurStatusBar(this);
    }
}

void StatusBar::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    ThemerAgent::drawStatusBar(this);
}

void StatusBar::showEvent(QShowEvent* event)
{
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove);
    KWindowSystem::setType(winId(), NET::Dock);
}

void StatusBar::slotEnable(bool enable)
{
    if (!KIMToySettings::self()->trayiconMode())
        m_visible = enable;
    m_visibleDelayer.start(100);
}

void StatusBar::slotSetVisibleDelayed()
{
    setVisible(m_visible);
}

void StatusBar::slotTriggerProperty(const QString& objectPath)
{
//     kWarning() << "trigger property" << objectPath;
    IMPanelAgent::TriggerProperty(objectPath);
}

void StatusBar::slotRegisterProperties(const QStringList& props)
{
    QList<QString> toRemove = m_propertyWidgets.keys();

    QString objectPath, name, iconName, description;
    foreach(const QString& p, props) {
        extractProperty(p, objectPath, name, iconName, description);
//         kWarning() << objectPath << name << iconName << description;
        PropertyWidget* pw = m_propertyWidgets.value(objectPath);
        bool needUpdate = false;
        if (!pw) {
            /// no such objectPath, register it
            pw = new PropertyWidget;
            pw->installEventFilter(this);
            connect(pw, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
            m_signalMapper->setMapping(pw, objectPath);
            m_propertyWidgets.insert(objectPath, pw);
            needUpdate = true;
        }
        else {
            toRemove.removeAll(objectPath);
        }

        /// update property
        pw->setProperty(objectPath, name, iconName, description);

        if (needUpdate && !m_filters.contains(objectPath)) {
            /// add to layout if just registered and not filtered
            m_layout->addWidget(pw);
            updateSize();
        }

        if (KIMToySettings::self()->trayiconMode()) {
            KStatusNotifierItem* tw = m_trayWidgets.value(objectPath);
            if (!tw) {
                /// no such objectPath, register it
                tw = new KStatusNotifierItem(objectPath);
                connect(tw, SIGNAL(activateRequested(bool,QPoint)), m_signalMapper, SLOT(map()));
                connect(tw, SIGNAL(secondaryActivateRequested(QPoint)), m_signalMapper, SLOT(map()));
                m_signalMapper->setMapping(tw, objectPath);
                m_trayWidgets.insert(objectPath, tw);
            }
            /// update property
            tw->setTitle(name);
            tw->setToolTipTitle(name);
            tw->setToolTipSubTitle(description);
            if (!iconName.isEmpty()) {
                tw->setIconByName(iconName);
                tw->setToolTipIconByName(iconName);
            }
            else {
                // draw an icon from name text
                QPixmap iconpix(22, 22);
                iconpix.fill(Qt::white);
                QPainter p(&iconpix);
                p.drawText(0, 0, 22, 22, Qt::AlignCenter, name);
                tw->setIconByPixmap(iconpix);
                tw->setToolTipIconByPixmap(iconpix);
            }
        }
    }

    // remove old ones
    foreach (const QString& r, toRemove) {
        PropertyWidget* pw = m_propertyWidgets.take(r);
        if (pw && !m_filters.contains(r)) {
            /// remove from layout if not filtered
            m_layout->removeWidget(pw);
            updateSize();
        }
        delete pw;

        if (KIMToySettings::self()->trayiconMode()) {
            delete m_trayWidgets.take(r);
        }
    }
}

void StatusBar::slotUpdateProperty(const QString& prop)
{
    QString objectPath, name, iconName, description;
    extractProperty(prop, objectPath, name, iconName, description);
//     kWarning() << objectPath << name << iconName << description;
    PropertyWidget* pw = m_propertyWidgets.value(objectPath);
    if (!pw) {
        /// no such objectPath
        qWarning() << "update property without register it! " << objectPath;
        return;
    }

    /// update property
    pw->setProperty(objectPath, name, iconName, description);

    if (KIMToySettings::self()->trayiconMode()) {
        KStatusNotifierItem* tw = m_trayWidgets.value(objectPath);
        if (!tw) {
            qWarning() << "update property without register it! " << objectPath;
            return;
        }
        /// update property
        tw->setTitle(name);
        tw->setToolTipTitle(name);
        tw->setToolTipSubTitle(description);
        if (!iconName.isEmpty()) {
            tw->setIconByName(iconName);
            tw->setToolTipIconByName(iconName);
        }
        else {
            // draw an icon from name text
            QPixmap iconpix(22, 22);
            iconpix.fill(Qt::white);
            QPainter p(&iconpix);
            p.drawText(0, 0, 22, 22, Qt::AlignCenter, name);
            tw->setIconByPixmap(iconpix);
            tw->setToolTipIconByPixmap(iconpix);
        }
    }
}

void StatusBar::slotRemoveProperty(const QString& prop)
{
    QString objectPath, name, iconName, description;
    extractProperty(prop, objectPath, name, iconName, description);
    PropertyWidget* pw = m_propertyWidgets.take(objectPath);
    if (!pw) {
        /// no such objectPath
        qWarning() << "remove property without register it! " << objectPath;
        return;
    }

    if (!m_filters.contains(objectPath)) {
        /// remove from layout if not filtered
        m_layout->removeWidget(pw);
        updateSize();
    }
    delete pw;

    if (KIMToySettings::self()->trayiconMode()) {
        KStatusNotifierItem* tw = m_trayWidgets.take(objectPath);
        if (!tw) {
            qWarning() << "remove property without register it! " << objectPath;
            return;
        }
        delete tw;
    }
}

void StatusBar::slotExecDialog(const QString& prop)
{
    QString objectPath, name, iconName, description;
    extractProperty(prop, objectPath, name, iconName, description);
    KMessageBox::information(0, description, name);
}

void StatusBar::slotExecMenu(const QStringList& actions)
{
    QMenu* menu = new QMenu;
    menu->setWindowFlags(Qt::ToolTip | Qt::WindowDoesNotAcceptFocus);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QString objectPath, name, iconName, description;
    foreach(const QString& a, actions) {
        extractProperty(a, objectPath, name, iconName, description);
        QAction* action = new QAction(QIcon::fromTheme(iconName), name, menu);
        connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
        m_signalMapper->setMapping(action, objectPath);
        connect(action, SIGNAL(triggered()), menu, SLOT(close()));
        menu->addAction(action);
    }
    menu->popup(QCursor::pos());
    menu->grabMouse();
}

void StatusBar::slotAutostartToggled(bool enable)
{
    KIMToySettings::self()->setAutostartKIMToy(enable);
}

void StatusBar::slotTrayiconModeToggled(bool enable)
{
    KIMToySettings::self()->setTrayiconMode(enable);
    setVisible(!enable);
    if (enable) {
        // construct tray widgets from property widgets
        QHash<QString, PropertyWidget*>::ConstIterator it = m_propertyWidgets.constBegin();
        QHash<QString, PropertyWidget*>::ConstIterator end = m_propertyWidgets.constEnd();
        while (it != end) {
            const QString& objectPath = it.key();
            PropertyWidget* pw = it.value();
            KStatusNotifierItem* tw = m_trayWidgets.value(objectPath);
            if (!tw) {
                /// no such objectPath, register it
                tw = new KStatusNotifierItem(objectPath);
                connect(tw, SIGNAL(activateRequested(bool,QPoint)), m_signalMapper, SLOT(map()));
                connect(tw, SIGNAL(secondaryActivateRequested(QPoint)), m_signalMapper, SLOT(map()));
                m_signalMapper->setMapping(tw, objectPath);
                m_trayWidgets.insert(objectPath, tw);
            }
            /// update property
            tw->setTitle(pw->name());
            tw->setToolTipTitle(pw->name());
            tw->setToolTipSubTitle(pw->description());
            if (!pw->iconName().isEmpty()) {
                tw->setIconByName(pw->iconName());
                tw->setToolTipIconByName(pw->iconName());
            }
            else {
                // draw an icon from name text
                QPixmap iconpix(22, 22);
                iconpix.fill(Qt::white);
                QPainter p(&iconpix);
                p.drawText(0, 0, 22, 22, Qt::AlignCenter, pw->name());
                tw->setIconByPixmap(iconpix);
                tw->setToolTipIconByPixmap(iconpix);
            }
            ++it;
        }
    }
    else {
        qDeleteAll(m_trayWidgets);
        m_trayWidgets.clear();
    }
}

void StatusBar::slotConfigureIMTriggered()
{
    IMPanelAgent::Configure();
}

void StatusBar::preferences()
{
    if (KConfigDialog::showDialog("settings"))
        return;
    KConfigDialog* dialog = new KConfigDialog(this, "settings", KIMToySettings::self());
    dialog->setFaceType(KPageDialog::List);
    dialog->addPage(new InputMethodWidget, i18n("Input method"), "draw-freehand");
    dialog->addPage(new AppearanceWidget, i18n("Appearance"), "preferences-desktop-color");
    dialog->addPage(new ThemeWidget, i18n("Theme"), "tools-wizard");
    dialog->addPage(new PerformanceWidget, i18n("Performance"), "preferences-system-performance");
    connect(dialog, SIGNAL(settingsChanged(QString)),
            this, SLOT(loadSettings()));
    dialog->show();
}

void StatusBar::slotAboutActionTriggered()
{
    KAboutApplicationDialog dlg(KAboutData::applicationData());
    dlg.exec();
}

void StatusBar::loadSettings()
{
    ThemerAgent::loadSettings();
    ThemerAgent::loadTheme();

    if (KIMToySettings::self()->enableWindowMask()) {
        ThemerAgent::maskStatusBar(this);
        ThemerAgent::maskPreEditBar(m_preeditBar);
    }
    else {
        clearMask();
        m_preeditBar->clearMask();
    }

    if (KIMToySettings::self()->enableThemeAnimation()) {
        Animator::self()->enable();
    }
    else {
        Animator::self()->disable();
    }

    foreach (PropertyWidget* pw, m_propertyWidgets) {
        ThemerAgent::maskPropertyWidget(pw);
    }

    updateSize();
    m_preeditBar->resize(ThemerAgent::sizeHintPreEditBar(m_preeditBar));
}

void StatusBar::slotFilterChanged(const QString& objectPath, bool checked)
{
    PropertyWidget* pw = m_propertyWidgets.value(objectPath);

    if (checked) {
        m_layout->addWidget(pw);
        pw->show();
        m_filters.removeAll(objectPath);
    }
    else {
        m_layout->removeWidget(pw);
        pw->hide();
        m_filters.append(objectPath);
    }
    updateSize();
}

void StatusBar::slotFilterMenuDestroyed()
{
    QTimer::singleShot(100, this, SLOT(slotConnectKIMPanel()));
}

void StatusBar::slotConnectKIMPanel()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "Enable",
                       this, SLOT(slotEnable(bool)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "RegisterProperties",
                       this, SLOT(slotRegisterProperties(const QStringList&)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateProperty",
                       this, SLOT(slotUpdateProperty(const QString&)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "RemoveProperty",
                       this, SLOT(slotRemoveProperty(const QString&)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "ExecDialog",
                       this, SLOT(slotExecDialog(const QString&)));
    connection.connect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "ExecMenu",
                       this, SLOT(slotExecMenu(const QStringList&)));
}

void StatusBar::slotDisconnectKIMPanel()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.disconnect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "Enable",
                          this, SLOT(slotEnable(bool)));
    connection.disconnect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "RegisterProperties",
                          this, SLOT(slotRegisterProperties(const QStringList&)));
    connection.disconnect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "UpdateProperty",
                          this, SLOT(slotUpdateProperty(const QString&)));
    connection.disconnect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "RemoveProperty",
                          this, SLOT(slotRemoveProperty(const QString&)));
    connection.disconnect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "ExecDialog",
                          this, SLOT(slotExecDialog(const QString&)));
    connection.disconnect("", "/kimpanel", "org.kde.kimpanel.inputmethod", "ExecMenu",
                          this, SLOT(slotExecMenu(const QStringList&)));
}

void StatusBar::updateSize()
{
    resize(ThemerAgent::sizeHintStatusBar(this));
}

void StatusBar::showFilterMenu()
{
    FilterMenu* menu = new FilterMenu;

    QHash<QString, PropertyWidget*>::ConstIterator it = m_propertyWidgets.constBegin();
    QHash<QString, PropertyWidget*>::ConstIterator end = m_propertyWidgets.constEnd();
    while (it != end) {
        const QString& objectPath = it.key();
        PropertyWidget* pw = it.value();
        bool visible = (m_layout->indexOf(pw) != -1);
        menu->addEntry(objectPath, pw, visible);
        ++it;
    }

    connect(menu, SIGNAL(filterChanged(QString,bool)),
            this, SLOT(slotFilterChanged(QString,bool)));

    connect(menu, SIGNAL(destroyed()), this, SLOT(slotFilterMenuDestroyed()));

    slotDisconnectKIMPanel();

    menu->move(mapToGlobal(m_pointPos));
    menu->show();
}
