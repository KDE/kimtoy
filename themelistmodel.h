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

#ifndef THEMELISTMODEL_H
#define THEMELISTMODEL_H

#include <QAbstractItemDelegate>
#include <QAbstractListModel>
#include <QHash>
#include <QPixmap>
#include <QString>

class KFileItem;

namespace KIO
{
class PreviewJob;
}

class ThemeListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ThemeListModel(QObject* parent = 0);
    virtual ~ThemeListModel();
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual int rowCount(const QModelIndex& parent) const;
    int previewWidth() const;
    void setPreviewWidth(int width);
public Q_SLOTS:
    void reloadThemes();
Q_SIGNALS:
    void relayoutNeeded();
private:
    void loadNoneTheme();
    void loadPlasmaThemes();
    void loadFileThemes();
    void generatePreviews();
private Q_SLOTS:
    void addPreview(const KFileItem& item, const QPixmap& preview);
    void failed(const KFileItem& item);
private:
    int m_previewWidth;
    QStringList m_themes;
    QHash<QString, QPixmap> m_previews;
    KIO::PreviewJob* m_previewJob;
};

class ThemeListDelegate : public QAbstractItemDelegate
{
public:
    explicit ThemeListDelegate(QObject* parent = 0);
    virtual ~ThemeListDelegate();
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // THEMELISTMODEL_H
