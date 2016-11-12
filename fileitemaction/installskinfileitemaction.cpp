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
