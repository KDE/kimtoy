/*
 *  This file is part of kde-thumbnailer-ssf
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

#include "ssfcreator.h"

#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QColor>
#include <QFile>
#include <QString>
#include <QTextStream>
#include <KZip>
#include <KZipFileEntry>
#include <kdemacros.h>

extern "C"
{
    KDE_EXPORT ThumbCreator* new_creator() {
        return new SsfCreator;
    }
}

class OverlayPixmap
{
public:
    QPixmap pixmap;
    /// TODO: only entire window is supported atm --- nihui
    int alignTarget;// 0->entire window, 1->preedit window, 2->candidate window
    int alignArea;// 1->lt, 2->t, 3->rt, 4->l, 5->center, 6->r, 7->lb, 8->b, 9->rb
    int alignHMode;// 0->align center, 1->align left, 2->align right
    int alignVMode;// 0->align center, 1->align top, 2->align bottom
    int mt, mb, ml, mr;// margins
};

SsfCreator::SsfCreator()
{
}

SsfCreator::~SsfCreator()
{
}

bool SsfCreator::create(const QString& path, int width, int height, QImage& img)
{
    if (!QFile::exists(path))
        return false;

    KZip zip(path);
    if (!zip.open(QIODevice::ReadOnly))
        return false;

    const KArchiveEntry* entry = zip.directory()->entry("skin.ini");
    const KZipFileEntry* skinini = static_cast<const KZipFileEntry*>(entry);

    if (!skinini) {
        entry = zip.directory()->entry("Skin.ini");
        skinini = static_cast<const KZipFileEntry*>(entry);
        if (!skinini)
            return false;
    }

    QFont preEditFont;
    QFont candidateFont;
    QColor preEditColor;
    QColor labelColor;
    QColor candidateColor;

    int pt = 0, pb = 0, pl = 0, pr = 0;
    int zt = 0, zb = 0, zl = 0, zr = 0;

    QByteArray data = skinini->data();

    /// parse ini file content
    bool display = false;
    bool scheme_h1 = false;
    QPixmap skin;
    int hstm = 0;
    int sl = 0, sr = 0;
    int fontPixelSize = 12;
    QString font_ch, font_en;
    QString color_ch, color_en;
    QHash<QString, OverlayPixmap> overlays;
    int opt = 0, opb = 0, opl = 0, opr = 0;
    QColor separatorColor = Qt::transparent;
    int sepl = 0, sepr = 0;

    QTextStream ss(data);
    QString line;
    QString key, value;
    do {
        line = ss.readLine();
        if (line.isEmpty())
            continue;

        if (line.at(0) == '[') {
            display = (line == "[Display]");
            scheme_h1 = (line == "[Scheme_H1]");
            continue;
        }

        key = line.split('=').at(0);
        value = line.split('=').at(1);

        if (value.isEmpty())
            continue;

        if (display) {
            if (key == "font_size")
                fontPixelSize = value.trimmed().toInt();
            else if (key == "font_ch")
                font_ch = value;
            else if (key == "font_en")
                font_en = value;
            else if (key == "pinyin_color")
                color_en = value;
            else if (key == "zhongwen_color")
                color_ch = value;
        }
        else if (scheme_h1) {
            if (key == "pic") {
                const KArchiveEntry* e = zip.directory()->entry(value);
                const KZipFileEntry* pix = static_cast<const KZipFileEntry*>(e);
                if (pix)
                    skin.loadFromData(pix->data());
            }
            else if (key == "layout_horizontal") {
                QStringList list = value.split(',');
                hstm = list.at(0).trimmed().toInt();
                sl = list.at(1).trimmed().toInt();
                sr = list.at(2).trimmed().toInt();
            }
            else if (key == "separator") {
                QStringList list = value.split(',');
                QString sep_color = list.at(0).trimmed();
                separatorColor = sep_color.leftJustified(8, '0').replace("0x", "#");
                sepl = list.at(1).trimmed().toInt();
                sepr = list.at(2).trimmed().toInt();
            }
            else if (key == "pinyin_marge") {
                QStringList list = value.split(',');
                pt = list.at(0).trimmed().toInt();
                pb = list.at(1).trimmed().toInt();
                pl = list.at(2).trimmed().toInt();
                pr = list.at(3).trimmed().toInt();
            }
            else if (key == "zhongwen_marge") {
                QStringList list = value.split(',');
                zt = list.at(0).trimmed().toInt();
                zb = list.at(1).trimmed().toInt();
                zl = list.at(2).trimmed().toInt();
                zr = list.at(3).trimmed().toInt();
            }
            else if (key.endsWith("_display")) {
                QString name = key.left(key.length() - 8);
                overlays.insert(name, OverlayPixmap());
            }
            else if (key.endsWith("_align")) {
                QString name = key.left(key.length() - 6);
                QStringList numbers = value.split(',');
                OverlayPixmap& op = overlays[ name ];
                op.mt = numbers.at(0).toInt();
                op.mb = numbers.at(1).toInt();
                op.ml = numbers.at(2).toInt();
                op.mr = numbers.at(3).toInt();
                op.alignVMode = numbers.at(4).toInt() + numbers.at(5).toInt();    /// FIXME: right or wrong?
                op.alignHMode = numbers.at(6).toInt() + numbers.at(7).toInt();    /// FIXME: right or wrong?
                op.alignArea = numbers.at(8).toInt();
                op.alignTarget = numbers.at(9).toInt();
            }
            else if (overlays.contains(key)) {
                const KArchiveEntry* e = zip.directory()->entry(value);
                const KZipFileEntry* pix = static_cast<const KZipFileEntry*>(e);
                if (pix)
                    overlays[ key ].pixmap.loadFromData(pix->data());
            }
        }
    }
    while (!line.isNull());

    /// calculate overlay pixmap surrounding
    QHash<QString, OverlayPixmap>::ConstIterator it = overlays.constBegin();
    QHash<QString, OverlayPixmap>::ConstIterator end = overlays.constEnd();
    while (it != end) {
        const OverlayPixmap& op = it.value();
        switch (op.alignArea) {
            case 1:
                opl = qMax(opl, op.pixmap.width());
                opt = qMax(opt, op.pixmap.height());
                break;
            case 2:
                opt = qMax(opt, op.pixmap.height());
                break;
            case 3:
                opr = qMax(opr, op.pixmap.width());
                opt = qMax(opt, op.pixmap.height());
                break;
            case 4:
                opl = qMax(opl, op.pixmap.width());
                break;
            case 5:
                /// center pixmap, no addition
                break;
            case 6:
                opr = qMax(opr, op.pixmap.width());
                break;
            case 7:
                opl = qMax(opl, op.pixmap.width());
                opb = qMax(opb, op.pixmap.height());
                break;
            case 8:
                opb = qMax(opb, op.pixmap.height());
                break;
            case 9:
                opr = qMax(opr, op.pixmap.width());
                opb = qMax(opb, op.pixmap.height());
                break;
            default:
                /// never arrive here
                break;
        }
        ++it;
    }

    preEditFont.setFamily(font_en);
    preEditFont.setPixelSize(fontPixelSize);
    preEditFont.setBold(true);
    candidateFont.setFamily(font_ch);
    candidateFont.setPixelSize(fontPixelSize);
    candidateFont.setBold(true);

    color_en = color_en.leftJustified(8, '0').replace("0x", "#");
    color_ch = color_ch.leftJustified(8, '0').replace("0x", "#");
    preEditColor = QColor(color_en);
    candidateColor = QColor(color_ch);
    labelColor = candidateColor;

    int pinyinh = QFontMetrics(preEditFont).height();
    int zhongwenh = QFontMetrics(candidateFont).height();
    int pinyinw = QFontMetrics(preEditFont).width("ABC pinyin");
    int zhongwenw = QFontMetrics(candidateFont).width("1candidate");

    /// save target size
    int targetHeight = height;
    int targetWidth = width;

    height = qMax(pt + pinyinh + pb + zt + zhongwenh + zb + opt + opb, skin.height());
    width = qMax(pl + pinyinw + pr, zl + zhongwenw + zr);
    width = qMax(width + opl + opr, targetWidth);
    width = qMax(width, skin.width());

    QPixmap pixmap(width, height);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);

    /// left right
    p.drawPixmap(0, 0, sl, height, skin, 0, 0, sl, skin.height());
    p.drawPixmap(width - sr, 0, sr, height, skin, skin.width() - sr, 0, sr, skin.height());

    /// middle
    QPixmap middlepix = skin.copy(sl, 0, skin.width() - sl - sr, skin.height());
    if (hstm == 0)
        p.drawPixmap(sl, 0, middlepix.scaled(width - sl - sr, height));
    else
        p.drawTiledPixmap(sl, 0, width - sl - sr, height, middlepix.scaled(middlepix.width(), height));

    /// draw overlay pixmap
    it = overlays.constBegin();
    end = overlays.constEnd();
    while (it != end) {
        const OverlayPixmap& op = it.value();
        p.save();
        switch (op.alignArea) {
            case 1:
                p.translate(-op.mr, -op.mb);
                break;
            case 2:
                if (op.alignHMode == 0) {
                    p.translate((width + opl - opr + op.pixmap.width()) / 2, 0);
                    p.translate(0, -op.mb);
                }
                else if (op.alignHMode == 1) {
                    p.translate(opl, 0);
                    p.translate(op.ml, -op.mb);
                }
                else if (op.alignHMode == 2) {
                    p.translate(width - opr - op.pixmap.width(), 0);
                    p.translate(-op.mr, -op.mb);
                }
                break;
            case 3:
                p.translate(width - opr, 0);
                p.translate(op.ml, -op.mb);
                break;
            case 4:
                if (op.alignVMode == 0) {
                    p.translate(0, (height - opb + opt + op.pixmap.height()) / 2);
                    p.translate(-op.mr, 0);
                }
                else if (op.alignVMode == 1) {
                    p.translate(0, opt);
                    p.translate(-op.mr, op.mt);
                }
                else if (op.alignVMode == 2) {
                    p.translate(0, height - opb - op.pixmap.height());
                    p.translate(-op.mr, -op.mb);
                }
                break;
            case 5:
                if (op.alignHMode == 0) {
                    p.translate((width + opl - opr + op.pixmap.width()) / 2, 0);
                }
                else if (op.alignHMode == 1) {
                    p.translate(opl, 0);
                    p.translate(op.ml, 0);
                }
                else if (op.alignHMode == 2) {
                    p.translate(width - opr - op.pixmap.width(), 0);
                    p.translate(-op.mr, 0);
                }
                if (op.alignVMode == 0) {
                    p.translate(0, (height - opb + opt + op.pixmap.height()) / 2);
                }
                else if (op.alignVMode == 1) {
                    p.translate(0, opt);
                    p.translate(0, op.mt);
                }
                else if (op.alignVMode == 2) {
                    p.translate(0, height - opb - op.pixmap.height());
                    p.translate(0, -op.mb);
                }
                break;
            case 6:
                if (op.alignVMode == 0) {
                    p.translate(width - opr, (height - opb + opt + op.pixmap.height()) / 2);
                    p.translate(op.ml, 0);
                }
                else if (op.alignVMode == 1) {
                    p.translate(width - opr, opt);
                    p.translate(op.ml, op.mt);
                }
                else if (op.alignVMode == 2) {
                    p.translate(width - opr, height - opb - op.pixmap.height());
                    p.translate(op.ml, -op.mb);
                }
                break;
            case 7:
                p.translate(0, height - opb);
                p.translate(-op.mr, op.mt);
                break;
            case 8:
                if (op.alignHMode == 0) {
                    p.translate((width + opl - opr + op.pixmap.width()) / 2, height - opb);
                    p.translate(0, op.mt);
                }
                else if (op.alignHMode == 1) {
                    p.translate(opl, height - opb);
                    p.translate(op.ml, op.mt);
                }
                else if (op.alignHMode == 2) {
                    p.translate(width - opr - op.pixmap.width(), height - opb);
                    p.translate(-op.mr, op.mt);
                }
                break;
            case 9:
                p.translate(width - opr, height - opb);
                p.translate(op.ml, op.mt);
                break;
            default:
                /// never arrive here
                break;
        }
        p.drawPixmap(0, 0, op.pixmap);
        p.restore();
        ++it;
    }

    if (separatorColor != Qt::transparent) {
        /// draw separator
        int sepy = opt + pt + pinyinh + pb;
        p.drawLine(opl + sepl, sepy, width - opr - sepr, sepy);
    }

    p.translate(opl, opt);
    int y = 0;

    /// draw preedit / aux text
    p.save();
    p.setFont(preEditFont);
    p.setPen(preEditColor);
    y += pt;
    p.drawText(pl, pt, width - pl - pr, pinyinh, Qt::AlignLeft, "ABC pinyin");
    int pixelsWide = QFontMetrics(preEditFont).width("ABC pinyin");
    p.drawLine(pl + pixelsWide, pt, pl + pixelsWide, pt + pinyinh);
    y += pinyinh + pb;
    p.restore();

    /// draw lookup table
    p.save();
    int x = zl;
    y += zt;
    int w = 0;
    p.setFont(candidateFont);
    /// draw label
    p.setPen(labelColor);
    w = p.fontMetrics().width("1");
    p.drawText(x, y, w, zhongwenh, Qt::AlignCenter, "1");
    x += w;
    /// draw candidate
    p.setPen(candidateColor);
    w = p.fontMetrics().width("candidate");
    p.drawText(x, y, w, zhongwenh, Qt::AlignCenter, "candidate");
    x += w;
    p.restore();

    if (targetWidth < width || targetHeight < height) {
        pixmap = pixmap.scaled(targetWidth, targetHeight, Qt::KeepAspectRatio);
    }

    img = pixmap.toImage();

    return true;
}
