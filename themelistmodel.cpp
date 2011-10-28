/*
 *  This file is part of KIMToy, an input method frontend for KDE
 *  Copyright (C) 2011 Ni Hui <shuizhuyuanluo@126.com>
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

#include "themelistmodel.h"

#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QIcon>
#include <QFileInfo>
#include <QPainter>
#include <QStyle>

#include <KDebug>
#include <KFileItem>
#include <KUrl>
#include <KIO/PreviewJob>
#include <KStandardDirs>

#include <Plasma/FrameSvg>
#include <Plasma/Theme>

#include "kimtoysettings.h"

ThemeListModel::ThemeListModel(QObject* parent)
        : QAbstractListModel(parent)
{
    m_previewWidth = 0;
    m_previewJob = 0;
}

ThemeListModel::~ThemeListModel()
{
    if (m_previewJob) {
        m_previewJob->kill();
        delete m_previewJob;
        m_previewJob = 0;
    }
}

QVariant ThemeListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
        case Qt::DisplayRole:
            return m_themes.at(index.row());
        case Qt::DecorationRole:
            return m_previews.value(m_themes.at(index.row()));
        case Qt::SizeHintRole:
            return m_previews.value(m_themes.at(index.row())).size();
        default:
            return QVariant();
    }
}

Qt::ItemFlags ThemeListModel::flags(const QModelIndex& index) const
{
    return QAbstractListModel::flags(index);
}

int ThemeListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_themes.size();
}

int ThemeListModel::previewWidth() const
{
    return m_previewWidth;
}

void ThemeListModel::setPreviewWidth(int width)
{
    if (m_previewWidth == width)
        return;

    if (m_previewJob) {
        m_previewJob->kill();
        delete m_previewJob;
        m_previewJob = 0;
    }

    m_previewWidth = width;
    generatePreviews();
}

void ThemeListModel::reloadThemes()
{
    if (m_previewJob) {
        m_previewJob->kill();
        delete m_previewJob;
        m_previewJob = 0;
    }

    beginRemoveRows(QModelIndex(), 0, m_themes.count() - 1);
    m_themes.clear();
    m_previews.clear();
    endRemoveRows();

    loadNoneTheme();
    loadPlasmaThemes();
    loadFileThemes();

    generatePreviews();
}

void ThemeListModel::loadNoneTheme()
{
    beginInsertRows(QModelIndex(), m_themes.count(), m_themes.count());
    m_themes << "__none__";
    endInsertRows();
}

void ThemeListModel::loadPlasmaThemes()
{
    KStandardDirs dirs;
    const QStringList themes = dirs.findAllResources("data", "desktoptheme/*/metadata.desktop", KStandardDirs::NoDuplicates);

    foreach(const QString& theme, themes) {
        QString themeRoot = theme.left(theme.lastIndexOf('/', -1));
        QString themeName = themeRoot.mid(themeRoot.lastIndexOf('/', -1) + 1);

        beginInsertRows(QModelIndex(), m_themes.count(), m_themes.count());
        m_themes << "__plasma__" + themeName;
        endInsertRows();
    }
}

void ThemeListModel::loadFileThemes()
{
    // load local themes
    QString themeFolder = KIMToySettings::self()->themeFolder().path();
//     kWarning() << themeFolder;
    QDir dir(themeFolder);
    QFileInfoList es = dir.entryInfoList(QStringList() << "*.fskin" << "*.ssf");

    // load downloaded themes
    QString knsFolder = KStandardDirs::locateLocal("appdata", "themes/");
//     kWarning() << knsFolder;
    if (knsFolder != themeFolder) {
        QDir knsThemeDir(knsFolder);
        es << knsThemeDir.entryInfoList(QStringList() << "*.fskin" << "*.ssf");
    }

    KFileItemList items;
    foreach(const QFileInfo& e, es) {
        KFileItem item(KFileItem::Unknown, KFileItem::Unknown, KUrl(e.absoluteFilePath()));

        beginInsertRows(QModelIndex(), m_themes.count(), m_themes.count());
        m_themes << item.localPath();
        endInsertRows();

        items << item;
    }
}

void ThemeListModel::generatePreviews()
{
    KFileItemList items;

    QStringList::ConstIterator it = m_themes.constBegin();
    QStringList::ConstIterator end = m_themes.constEnd();
    while (it != end) {
        QString e = *it;
        ++it;
        if (e == "__none__") {
            QFont preEditFont = KIMToySettings::self()->preeditFont();
            QFont labelFont = KIMToySettings::self()->labelFont();
            QFont candidateFont = KIMToySettings::self()->candidateFont();
            int preEditFontHeight = QFontMetrics(preEditFont).height();
            int labelFontHeight = QFontMetrics(labelFont).height();
            int candidateFontHeight = QFontMetrics(candidateFont).height();
            QColor preEditColor = KIMToySettings::self()->preeditColor();
            QColor labelColor = KIMToySettings::self()->labelColor();
            QColor candidateColor = KIMToySettings::self()->candidateColor();

            int pinyinh = preEditFontHeight;
            int zhongwenh = qMax(labelFontHeight, candidateFontHeight);
            int pinyinw = QFontMetrics(preEditFont).width("ABC pinyin");
            int labelw = QFontMetrics(labelFont).width("1");
            int candidatew = QFontMetrics(candidateFont).width("candidate");

            int width = qMax(pinyinw, labelw + candidatew);
            int height = pinyinh + zhongwenh;

            QPixmap preview(qMax(width, m_previewWidth), height);
            preview.fill(Qt::gray);

            QPainter p(&preview);
            p.setFont(preEditFont);
            p.setPen(preEditColor);
            p.drawText(0, 0, pinyinw, pinyinh, Qt::AlignLeft, "ABC pinyin");
            p.drawLine(pinyinw, 0, pinyinw, pinyinh);
            p.setFont(labelFont);
            p.setPen(labelColor);
            p.drawText(0, pinyinh, labelw, zhongwenh, Qt::AlignCenter, "1");
            p.setFont(candidateFont);
            p.setPen(candidateColor);
            p.drawText(labelw, pinyinh, candidatew, zhongwenh, Qt::AlignCenter, "candidate");
            p.end();

            if (m_previewWidth < width)
                preview = preview.scaled(m_previewWidth, height, Qt::KeepAspectRatio);

            m_previews[ e ] = preview;
        }
        else if (e.startsWith("__plasma__")) {
            Plasma::Theme plasmaTheme;
            plasmaTheme.setThemeName(e.mid(10));

            const QString imagePath = plasmaTheme.imagePath("widgets/background");
            QFont preEditFont = plasmaTheme.font(Plasma::Theme::DefaultFont);
            QFont labelFont = plasmaTheme.font(Plasma::Theme::DesktopFont);
            QFont candidateFont = plasmaTheme.font(Plasma::Theme::DefaultFont);
            int preEditFontHeight = QFontMetrics(preEditFont).height();
            int labelFontHeight = QFontMetrics(labelFont).height();
            int candidateFontHeight = QFontMetrics(candidateFont).height();
            QColor candidateColor = plasmaTheme.color(Plasma::Theme::TextColor);
            QColor preEditColor = plasmaTheme.color(Plasma::Theme::TextColor);
            QColor labelColor = plasmaTheme.color(Plasma::Theme::HighlightColor);

            Plasma::FrameSvg svg;
            svg.setImagePath(imagePath);
            svg.setEnabledBorders(Plasma::FrameSvg::AllBorders);

            qreal left, top, right, bottom;
            svg.getMargins(left, top, right, bottom);

            int pinyinh = preEditFontHeight;
            int zhongwenh = qMax(labelFontHeight, candidateFontHeight);
            int pinyinw = QFontMetrics(preEditFont).width("ABC pinyin");
            int labelw = QFontMetrics(labelFont).width("1");
            int candidatew = QFontMetrics(candidateFont).width("candidate");

            int width = left + qMax(pinyinw, labelw + candidatew) + right;
            int height = top + pinyinh + zhongwenh + bottom;

            svg.resizeFrame(QSize(qMax(width, m_previewWidth), height));
            QPixmap preview = svg.framePixmap();

            QPainter p(&preview);
            p.translate(left, top);
            p.setFont(preEditFont);
            p.setPen(preEditColor);
            p.drawText(0, 0, pinyinw, pinyinh, Qt::AlignLeft, "ABC pinyin");
            p.drawLine(pinyinw, 0, pinyinw, pinyinh);
            p.setFont(labelFont);
            p.setPen(labelColor);
            p.drawText(0, pinyinh, labelw, zhongwenh, Qt::AlignCenter, "1");
            p.setFont(candidateFont);
            p.setPen(candidateColor);
            p.drawText(labelw, pinyinh, candidatew, zhongwenh, Qt::AlignCenter, "candidate");
            p.end();

            if (m_previewWidth < width)
                preview = preview.scaled(m_previewWidth, height, Qt::KeepAspectRatio);

            m_previews[ e ] = preview;
        }
        else {
            KFileItem item(KFileItem::Unknown, KFileItem::Unknown, KUrl(e));
            items << item;
        }
    }
// qWarning() << KIO::PreviewJob::availablePlugins();

    QStringList enabledPlugins;
    enabledPlugins << "fskinthumbnail" << "ssfthumbnail";
//     m_previewJob = KIO::filePreview(items, QSize(m_previewWidth, 400), &enabledPlugins);
    m_previewJob = KIO::filePreview(items, m_previewWidth, 400, 0, 70, false, false, &enabledPlugins);
    m_previewJob->setAutoDelete(false);
//     m_previewJob->setScaleType(KIO::PreviewJob::Unscaled);

    connect(m_previewJob, SIGNAL(finished(KJob*)), this, SIGNAL(relayoutNeeded()));
    connect(m_previewJob, SIGNAL(gotPreview(const KFileItem&, const QPixmap&)),
            this, SLOT(addPreview(const KFileItem&, const QPixmap&)));
    connect(m_previewJob, SIGNAL(failed(const KFileItem&)),
            this, SLOT(failed(const KFileItem&)));
}

void ThemeListModel::addPreview(const KFileItem& item, const QPixmap& preview)
{
// qWarning() << "addPreview" << item.localPath();
    m_previews[ item.localPath()] = preview;
}

void ThemeListModel::failed(const KFileItem& item)
{
// qWarning() << "failed" << item.localPath() << m_previewJob->errorString();
//     QPixmap preview( m_previewWidth, 40 );
//     m_previews[ item.localPath() ] = preview;
}

ThemeListDelegate::ThemeListDelegate(QObject* parent)
        : QAbstractItemDelegate(parent)
{
}

ThemeListDelegate::~ThemeListDelegate()
{
}

void ThemeListDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItemV4 opt(option);
    QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    QPixmap pix = index.data(Qt::DecorationRole).value<QPixmap>();
    painter->drawPixmap(option.rect.topLeft().x() + 4, option.rect.topLeft().y() + 4, pix);
}

QSize ThemeListDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // add margin
    QSize pixmapSize = index.data(Qt::SizeHintRole).toSize();
    return QSize(pixmapSize.width() + 2 * 4, pixmapSize.height() + 2 * 4);
}

