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

#include "statusbar.h"

#include <QAction>
#include <QDBusConnection>
#include <QMenu>
#include <QMouseEvent>
#include <QSignalMapper>

#include <KAboutApplicationDialog>
#include <KAction>
#include <KActionCollection>
#include <KApplication>
#include <KConfig>
#include <KConfigDialog>
#include <KConfigGroup>
#include <KDebug>
#include <KGlobal>
#include <KIcon>
#include <KLocale>
#include <KMenu>
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
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::KeepAbove);
    KWindowSystem::setType(winId(), NET::PopupMenu);

    ThemerAgent::loadSettings();

    m_preeditBar = new PreEditBar;

    m_tray = new KStatusNotifierItem(this);
    m_tray->setAssociatedWidget(m_tray->contextMenu());
    m_tray->setIconByName("draw-freehand");
    m_tray->setTitle(i18n("KIMToy"));
    m_tray->setToolTipIconByName("draw-freehand");
    m_tray->setToolTipTitle(i18n("KIMToy"));
    m_tray->setToolTipSubTitle(i18n("Input method toy"));
    m_tray->setCategory(KStatusNotifierItem::ApplicationStatus);
    m_tray->setStatus(KStatusNotifierItem::Passive);

    KToggleAction* autostartAction = new KToggleAction(i18n("A&utostart"), this);
    autostartAction->setChecked(KIMToySettings::self()->autostartKIMToy());
    connect(autostartAction, SIGNAL(toggled(bool)), this, SLOT(slotAutostartToggled(bool)));
    m_tray->contextMenu()->addAction(autostartAction);

    KAction* prefAction = KStandardAction::preferences(this, SLOT(preferences()), 0);
    m_tray->contextMenu()->addAction(prefAction);

    KAction* aboutAction = new KAction(KIcon("draw-freehand"), i18n("&About KIMToy..."), this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(slotAboutActionTriggered()));
    m_tray->contextMenu()->addAction(aboutAction);

    m_signalMapper = new QSignalMapper(this);
    connect(m_signalMapper, SIGNAL(mapped(QString)),
            this, SLOT(slotTriggerProperty(QString)));

    m_layout = new StatusBarLayout;
    setLayout(m_layout);

//     m_hideButton = new QPushButton;
//     m_hideButton->installEventFilter( this );
//     m_hideButton->setFlat( true );
//     m_hideButton->setFixedSize( QSize( 22, 22 ) );
//     m_hideButton->setIcon( KIcon( "arrow-down-double" ) );
//     connect( m_hideButton, SIGNAL(clicked()),
//              this, SLOT(hide()) );

    setAttribute(Qt::WA_AlwaysShowToolTips, true);

    installEventFilter(this);

    m_rmbdown = false;
    m_moving = false;

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

    KConfigGroup group(KGlobal::config(), "General");
    QPoint pos = group.readEntry("XYPosition", QPoint(100, 0));
    move(pos);

    m_filters = group.readEntry("Filters", QStringList());

    connect(Animator::self(), SIGNAL(animateStatusBar()), this, SLOT(update()));
    connect(Animator::self(), SIGNAL(animatePreEditBar()), m_preeditBar, SLOT(update()));

    loadSettings();

    IMPanelAgent::PanelCreated();
}

StatusBar::~StatusBar()
{
    KConfigGroup group(KGlobal::config(), "General");
    group.writeEntry("XYPosition", pos());
    group.writeEntry("Filters", m_filters);
    delete m_preeditBar;
    qDeleteAll(m_propertyWidgets);
    m_propertyWidgets.clear();
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
}

void StatusBar::slotEnable(bool enable)
{
    setVisible(enable);
}

void StatusBar::slotTriggerProperty(const QString& objectPath)
{
//     kWarning() << "trigger property" << objectPath;
    IMPanelAgent::TriggerProperty(objectPath);
}

void StatusBar::slotRegisterProperties(const QStringList& props)
{
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

        /// update property
        pw->setProperty(objectPath, name, iconName, description);

        if (needUpdate && !m_filters.contains(objectPath)) {
            /// add to layout if just registered and not filtered
            m_layout->addWidget(pw);
            updateSize();
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
        kWarning() << "update property without register it! " << objectPath;
        return;
    }

    /// update property
    pw->setProperty(objectPath, name, iconName, description);
}

void StatusBar::slotRemoveProperty(const QString& prop)
{
    QString objectPath, name, iconName, description;
    extractProperty(prop, objectPath, name, iconName, description);
    PropertyWidget* pw = m_propertyWidgets.take(objectPath);
    if (!pw) {
        /// no such objectPath
        kWarning() << "remove property without register it! " << objectPath;
        return;
    }

    if (!m_filters.contains(objectPath)) {
        /// remove from layout if not filtered
        m_layout->removeWidget(pw);
        updateSize();
    }
    delete pw;
}

void StatusBar::slotExecDialog(const QString& prop)
{
    QString objectPath, name, iconName, description;
    extractProperty(prop, objectPath, name, iconName, description);
    KMessageBox::information(0, description, name);
}

void StatusBar::slotExecMenu(const QStringList& actions)
{
    QMenu menu;
    QString objectPath, name, iconName, description;
    foreach(const QString& a, actions) {
        extractProperty(a, objectPath, name, iconName, description);
        QAction* action = new QAction(KIcon(iconName), name, &menu);
        connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
        m_signalMapper->setMapping(action, objectPath);
        menu.addAction(action);
    }
    menu.exec(QCursor::pos());
}

void StatusBar::slotAutostartToggled(bool enable)
{
    KIMToySettings::self()->setAutostartKIMToy(enable);
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
    KAboutApplicationDialog dlg(KGlobal::mainComponent().aboutData());
    dlg.exec();
}

void StatusBar::loadSettings()
{
    bool enableTransparency = KIMToySettings::self()->backgroundTransparency();
    setAttribute(Qt::WA_TranslucentBackground, enableTransparency);
    m_preeditBar->setAttribute(Qt::WA_TranslucentBackground, enableTransparency);

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

    menu->move(mapToGlobal(m_pointPos));
    menu->show();
}
