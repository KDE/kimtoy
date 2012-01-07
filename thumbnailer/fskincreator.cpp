/*
 *  This file is part of kde-thumbnailer-fskin
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

#include "fskincreator.h"

#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QColor>
#include <QFile>
#include <QString>
#include <QTextStream>
#include <KTar>
#include <kdemacros.h>

extern "C"
{
    KDE_EXPORT ThumbCreator* new_creator() {
        return new FskinCreator;
    }
}

static QColor value2color(const QString& value)
{
    QStringList list = value.split(' ');
    int r = list.at(0).toInt();
    int g = list.at(1).toInt();
    int b = list.at(2).toInt();
    return QColor(r, g, b);
}

FskinCreator::FskinCreator()
{
}

FskinCreator::~FskinCreator()
{
}

bool FskinCreator::create(const QString& path, int width, int height, QImage& img)
{
    if (!QFile::exists(path))
        return false;

    KTar tar(path);
    if (!tar.open(QIODevice::ReadOnly))
        return false;

    QStringList entries = tar.directory()->entries();
    if (entries.count() != 1)
        return false;

    const KArchiveEntry* entry = tar.directory()->entry(entries.first());
    if (!entry->isDirectory())
        return false;

    const KArchiveDirectory* subdir = static_cast<const KArchiveDirectory*>(entry);
    const KArchiveEntry* skinconf_entry = subdir->entry("fcitx_skin.conf");
    const KArchiveFile* skinconf = static_cast<const KArchiveFile*>(skinconf_entry);
    if (!skinconf)
        return false;

    QByteArray data = skinconf->data();

    /// parse ini file content
    bool skinfont = false;
    bool skininputbar = false;

    QPixmap skin;
    QString resizemode;
    /// margins
    int ml = 0, mr = 0, mt = 0, mb = 0;
    /// font size
    QFont font;
    QColor color_en;
    QColor color_label;
    QColor color_ch_1st;
    /// text y position
    int yen = 0, ych = 0;
    /// cursor color
    QColor color_cursor;
    /// back arrow
    QPixmap barrow;
    int xba = 0, yba = 0;
    /// forward arrow
    QPixmap farrow;
    int xfa = 0, yfa = 0;

    QTextStream ss(data);
    QString line;
    QString key, value;
    do {
        line = ss.readLine();
        if (line.isEmpty() || line.at(0) == '#')
            continue;

        if (line.at(0) == '[') {
            skinfont = (line == "[SkinFont]");
            skininputbar = (line == "[SkinInputBar]");
            continue;
        }

        QString key = line.split('=').at(0);
        QString value = line.split('=').at(1);

        if (value.isEmpty())
            continue;

        if (skinfont) {
            if (key == "FontSize") {
                font.setPixelSize(value.toInt());
            }
            else if (key == "InputColor") {
                color_en = value2color(value);
            }
            else if (key == "IndexColor") {
                color_label = value2color(value);
            }
            else if (key == "FirstCandColor") {
                color_ch_1st = value2color(value);
            }
            else if (key == "UserPhraseColor") {
                QColor color_ch_user = value2color(value);
            }
            else if (key == "OtherColor") {
                QColor color_ch_other = value2color(value);
            }
        }
        else if (skininputbar) {
            if (key == "BackImg") {
                const KArchiveEntry* e = subdir->entry(value);
                const KArchiveFile* pix = static_cast<const KArchiveFile*>(e);
                if (pix)
                    skin.loadFromData(pix->data());
            }
            else if (key == "Resize") {
                resizemode = value;
            }
            else if (key == "MarginLeft") {
                ml = value.toInt();
            }
            else if (key == "MarginRight") {
                mr = value.toInt();
            }
            else if (key == "MarginTop") {
                mt = value.toInt();
            }
            else if (key == "MarginBottom") {
                mb = value.toInt();
            }
            else if (key == "InputPos") {
                yen = value.toInt();
            }
            else if (key == "OutputPos") {
                ych = value.toInt();
            }
            else if (key == "CursorColor") {
                color_cursor = value2color(value);
            }
            else if (key == "BackArrow") {
                const KArchiveEntry* e = subdir->entry(value);
                const KArchiveFile* pix = static_cast<const KArchiveFile*>(e);
                if (pix)
                    barrow.loadFromData(pix->data());
            }
            else if (key == "ForwardArrow") {
                const KArchiveEntry* e = subdir->entry(value);
                const KArchiveFile* pix = static_cast<const KArchiveFile*>(e);
                if (pix)
                    farrow.loadFromData(pix->data());
            }
            else if (key == "BackArrowX") {
                xba = value.toInt();
            }
            else if (key == "BackArrowY") {
                yba = value.toInt();
            }
            else if (key == "ForwardArrowX") {
                xfa = value.toInt();
            }
            else if (key == "ForwardArrowY") {
                yfa = value.toInt();
            }
        }
    }
    while (!line.isNull());

    font.setBold(true);

    int fontHeight = QFontMetrics(font).height();
    int pinyinw = QFontMetrics(font).width("ABC pinyin");
    int zhongwenw = QFontMetrics(font).width("1candidate");

    /// save target size
    int targetHeight = height;
    int targetWidth = width;

    height = mt + ych + mb;
    width = qMax(ml + pinyinw + mr, ml + zhongwenw + mr);
    width = qMax(width, targetWidth);
    width = qMax(width, skin.width());

    yen = mt + yen - fontHeight;
    ych = mt + ych - fontHeight;

    QPixmap pixmap(width, height);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);

    /// corners lt, lb, rt, rb
    p.drawPixmap(0, 0, skin,
                 0, 0, ml, mt);
    p.drawPixmap(0, height - mb, skin,
                 0, skin.height() - mb, ml, mb);
    p.drawPixmap(width - mr, 0, skin,
                 skin.width() - mr, 0, mr, mt);
    p.drawPixmap(width - mr, height - mb, skin,
                 skin.width() - mr, skin.height() - mb, mr, mb);

    /// left right
    p.drawPixmap(0, mt, ml, height - mt - mb, skin,
                 0, mt, ml, skin.height() - mt - mb);
    p.drawPixmap(width - mr, mt, mr, height - mt - mb, skin,
                 skin.width() - mr, mt, mr, skin.height() - mt - mb);

    /// top bottom
    p.drawPixmap(ml, 0, width - ml - mr, mt, skin,
                 ml, 0, skin.width() - ml - mr, mt);
    p.drawPixmap(ml, height - mb, width - ml - mr, mb, skin,
                 ml, skin.height() - mb, skin.width() - ml - mr, mb);

    /// middle
    p.drawPixmap(ml, mt, width - ml - mr, height - mt - mb, skin,
                 ml, mt, skin.width() - ml - mr, skin.height() - mt - mb);

    /// draw arrows
    p.drawPixmap(width - xba, yba, barrow);
    p.drawPixmap(width - xfa, yfa, farrow);

    /// draw preedit / aux text
    p.setFont(font);
    p.setPen(color_en);
    p.drawText(ml, yen, pinyinw, fontHeight, Qt::AlignCenter, "ABC pinyin");
    p.drawLine(ml + pinyinw, yen, ml + pinyinw, yen + fontHeight);

    /// draw lookup table
    p.setPen(color_label);
    int labelw = p.fontMetrics().width("1");
    p.drawText(ml, ych, labelw, fontHeight, Qt::AlignCenter, "1");
    p.setPen(color_ch_1st);
    int candidatew = p.fontMetrics().width("candidate");
    p.drawText(ml + labelw, ych, candidatew, fontHeight, Qt::AlignCenter, "candidate");

    if (targetWidth < width || targetHeight < height) {
        pixmap = pixmap.scaled(targetWidth, targetHeight, Qt::KeepAspectRatio);
    }

    img = pixmap.toImage();

    return true;
}
