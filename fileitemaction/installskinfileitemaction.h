#ifndef INSTALLSKINFILEITEMACTION_H
#define INSTALLSKINFILEITEMACTION_H

#include <KAbstractFileItemActionPlugin>

class InstallSkinFileItemAction : public KAbstractFileItemActionPlugin
{
    Q_OBJECT
public:
    explicit InstallSkinFileItemAction(QObject* parent, const QVariantList& args);
    virtual QList<QAction*> actions(const KFileItemListProperties& fileItemInfos, QWidget* parentWidget);
private Q_SLOTS:
    void installSkin();
};

#endif // INSTALLSKINFILEITEMACTION_H
