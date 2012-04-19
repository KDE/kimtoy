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

#include "themer_sogou.h"

#include <QBitmap>
#include <QColor>
#include <QFile>
#include <QFontMetrics>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QTextStream>

#include <KDebug>
#include <KIconLoader>
#include <KZip>
#include <KZipFileEntry>
#include <Plasma/WindowEffects>

#include "animator.h"

#include "preeditbar.h"
#include "statusbar.h"
#include "statusbarlayout.h"
#include "propertywidget.h"

#include "kimtoysettings.h"

static void calculateOverlaySurrounding(const QHash<QString, OverlayPixmap*>& overlays, int& opt, int& opb, int& opl, int& opr)
{
    opl = 0;
    opr = 0;
    opt = 0;
    opb = 0;
    QHash<QString, OverlayPixmap*>::ConstIterator it = overlays.constBegin();
    QHash<QString, OverlayPixmap*>::ConstIterator end = overlays.constEnd();
    while (it != end) {
        const OverlayPixmap* op = it.value();
        const QPixmap& pixmap = op->currentPixmap();
        switch (op->alignArea) {
            case 1:
                opl = qMax(opl, pixmap.width());
                opt = qMax(opt, pixmap.height());
                break;
            case 2:
                opt = qMax(opt, pixmap.height());
                break;
            case 3:
                opr = qMax(opr, pixmap.width());
                opt = qMax(opt, pixmap.height());
                break;
            case 4:
                opl = qMax(opl, pixmap.width());
                break;
            case 5:
                /// center pixmap, no addition
                break;
            case 6:
                opr = qMax(opr, pixmap.width());
                break;
            case 7:
                opl = qMax(opl, pixmap.width());
                opb = qMax(opb, pixmap.height());
                break;
            case 8:
                opb = qMax(opb, pixmap.height());
                break;
            case 9:
                opr = qMax(opr, pixmap.width());
                opb = qMax(opb, pixmap.height());
                break;
            default:
                /// never arrive here
                break;
        }
        ++it;
    }
}

ThemerSogou* ThemerSogou::m_self = 0;

ThemerSogou* ThemerSogou::self()
{
    if (!m_self)
        m_self = new ThemerSogou;
    return m_self;
}

ThemerSogou::ThemerSogou()
        : Themer()
{
    m_statusBarSkin = 0;
}

ThemerSogou::~ThemerSogou()
{
}

bool ThemerSogou::loadTheme()
{
    QString file = KIMToySettings::self()->themeUri();
    if (!QFile::exists(file))
        return false;

    KZip zip(file);
    if (!zip.open(QIODevice::ReadOnly)) {
        return false;
    }

    const KArchiveEntry* entry = zip.directory()->entry("skin.ini");
    const KZipFileEntry* skinini = static_cast<const KZipFileEntry*>(entry);

    if (!skinini) {
        entry = zip.directory()->entry("Skin.ini");
        skinini = static_cast<const KZipFileEntry*>(entry);
        if (!skinini)
            return false;
    }

    QByteArray data = skinini->data();

    /// parse ini file content
    bool general = false;
    bool display = false;
    bool scheme_h1 = false;
    bool scheme_v1 = false;
    bool statusbar = false;
    QPixmap h1skin;
    QPixmap v1skin;
    int fontPixelSize = 12;
    QString font_ch, font_en;
    QString pinyin_color, zhongwen_color;
    int h_hsl = 0, h_hsr = 0, h_vst = 0, h_vsb = 0, h_hstm = 0, h_vstm = 0;
    int v_hsl = 0, v_hsr = 0, v_vst = 0, v_vsb = 0, v_hstm = 0, v_vstm = 0;

    qDeleteAll(h_overlays);
    qDeleteAll(v_overlays);
    h_overlays.clear();
    v_overlays.clear();
    delete m_statusBarSkin;
    m_statusBarSkin = 0;
    qDeleteAll(s_overlays);
    s_overlays.clear();
    m_pwpos.clear();
    m_pwpix.clear();
    h_separatorColor = Qt::transparent;
    v_separatorColor = Qt::transparent;
    h_sepl = 0, h_sepr = 0;
    v_sepl = 0, v_sepr = 0;

    QTextStream ss(data);
    QString line;
    QString key, value;
    do {
        line = ss.readLine();
        if (line.isEmpty())
            continue;

        if (line.at(0) == '[') {
            general = (line == "[General]");
            display = (line == "[Display]");
            scheme_h1 = (line == "[Scheme_H1]");
            scheme_v1 = (line == "[Scheme_V1]");
            statusbar = (line == "[StatusBar]");
            continue;
        }

        if (!line.contains('='))
            continue;

        key = line.split('=').at(0);
        value = line.split('=').at(1);

        if (value.isEmpty())
            continue;

        if (general) {
            /// extract meta info
        }
        else if (display) {
            if (key == "font_size")
                fontPixelSize = value.trimmed().toInt();
            else if (key == "font_ch")
                font_ch = value;
            else if (key == "font_en")
                font_en = value;
            else if (key == "pinyin_color")
                pinyin_color = value;
            else if (key == "zhongwen_color")
                zhongwen_color = value;
        }
        else if (scheme_h1) {
            if (key == "pic") {
                const KArchiveEntry* e = zip.directory()->entry(value);
                const KZipFileEntry* pix = static_cast<const KZipFileEntry*>(e);
                if (pix)
                    h1skin.loadFromData(pix->data());
            }
            else if (key == "layout_horizontal") {
                QStringList list = value.split(',');
                h_hstm = list.at(0).trimmed().toInt();
                h_hsl = list.at(1).trimmed().toInt();
                h_hsr = list.at(2).trimmed().toInt();
            }
            else if (key == "layout_vertical") {
                QStringList list = value.split(',');
                h_vstm = list.at(0).trimmed().toInt();
                h_vst = list.at(1).trimmed().toInt();
                h_vsb = list.at(2).trimmed().toInt();
                //WARNING: it seems that v_vst/v_vsb is always unused in sogou theme
                h_vstm = 0;//WARNING: assume scale mode --- nihui
                h_vst = 1;
                h_vsb = 1;
            }
            else if (key == "separator") {
                QStringList list = value.split(',');
                QString sep_color = list.at(0).trimmed();
                h_separatorColor = sep_color.leftJustified(8, '0').replace("0x", "#");
                h_sepl = list.at(1).trimmed().toInt();
                h_sepr = list.at(2).trimmed().toInt();
            }
            else if (key == "pinyin_marge") {
                QStringList list = value.split(',');
                h_pt = list.at(0).trimmed().toInt();
                h_pb = list.at(1).trimmed().toInt();
                h_pl = list.at(2).trimmed().toInt();
                h_pr = list.at(3).trimmed().toInt();
            }
            else if (key == "zhongwen_marge") {
                QStringList list = value.split(',');
                h_zt = list.at(0).trimmed().toInt();
                h_zb = list.at(1).trimmed().toInt();
                h_zl = list.at(2).trimmed().toInt();
                h_zr = list.at(3).trimmed().toInt();
            }
            else if (key.endsWith("_display")) {
                QString name = key.left(key.length() - 8);
                if (!h_overlays.contains(name)) {
                    h_overlays.insert(name, new OverlayPixmap);
                }
            }
            else if (key.endsWith("_align")) {
                QString name = key.left(key.length() - 6);
                QStringList numbers = value.split(',');
                if (!h_overlays.contains(name)) {
                    h_overlays.insert(name, new OverlayPixmap);
                }
                OverlayPixmap* op = h_overlays[ name ];
                op->mt = numbers.at(0).toInt();
                op->mb = numbers.at(1).toInt();
                op->ml = numbers.at(2).toInt();
                op->mr = numbers.at(3).toInt();
                op->alignVMode = numbers.at(4).toInt() + numbers.at(5).toInt();    /// FIXME: right or wrong?
                op->alignHMode = numbers.at(6).toInt() + numbers.at(7).toInt();    /// FIXME: right or wrong?
                op->alignArea = numbers.at(8).toInt();
                op->alignTarget = numbers.at(9).toInt();
            }
            else if (h_overlays.contains(key)) {
                const KArchiveEntry* e = zip.directory()->entry(value);
                const KZipFileEntry* pix = static_cast<const KZipFileEntry*>(e);
                if (pix) {
//                     h_overlays[ key ]->pixmap.loadFromData(pix->data());
                    OverlayPixmap* op = h_overlays[ key ];
                    QBuffer* d = new QBuffer;
                    d->setData(pix->data());
                    op->setDevice(d);
                    op->setFormat(value.endsWith(".gif") ? "gif" : "apng");
                    d->setParent(op);
                    Animator::self()->connectPreEditBarMovie(op);
                }
            }
        }
        else if (scheme_v1) {
            if (key == "pic") {
                const KArchiveEntry* e = zip.directory()->entry(value);
                const KZipFileEntry* pix = static_cast<const KZipFileEntry*>(e);
                if (pix)
                    v1skin.loadFromData(pix->data());
            }
            else if (key == "layout_horizontal") {
                QStringList list = value.split(',');
                v_hstm = list.at(0).trimmed().toInt();
                v_hsl = list.at(1).trimmed().toInt();
                v_hsr = list.at(2).trimmed().toInt();
            }
            else if (key == "layout_vertical") {
                QStringList list = value.split(',');
                v_vstm = list.at(0).trimmed().toInt();
                v_vst = list.at(1).trimmed().toInt();
                v_vsb = list.at(2).trimmed().toInt();
            }
            else if (key == "separator") {
                QStringList list = value.split(',');
                QString sep_color = list.at(0).trimmed();
                v_separatorColor = sep_color.leftJustified(8, '0').replace("0x", "#");
                v_sepl = list.at(1).trimmed().toInt();
                v_sepr = list.at(2).trimmed().toInt();
            }
            else if (key == "pinyin_marge") {
                QStringList list = value.split(',');
                v_pt = list.at(0).trimmed().toInt();
                v_pb = list.at(1).trimmed().toInt();
                v_pl = list.at(2).trimmed().toInt();
                v_pr = list.at(3).trimmed().toInt();
            }
            else if (key == "zhongwen_marge") {
                QStringList list = value.split(',');
                v_zt = list.at(0).trimmed().toInt();
                v_zb = list.at(1).trimmed().toInt();
                v_zl = list.at(2).trimmed().toInt();
                v_zr = list.at(3).trimmed().toInt();
            }
            else if (key.endsWith("_display")) {
                QString name = key.left(key.length() - 8);
                if (!v_overlays.contains(name)) {
                    v_overlays.insert(name, new OverlayPixmap);
                }
            }
            else if (key.endsWith("_align")) {
                QString name = key.left(key.length() - 6);
                QStringList numbers = value.split(',');
                if (!v_overlays.contains(name)) {
                    v_overlays.insert(name, new OverlayPixmap);
                }
                OverlayPixmap* op = v_overlays[ name ];
                op->mt = numbers.at(0).toInt();
                op->mb = numbers.at(1).toInt();
                op->ml = numbers.at(2).toInt();
                op->mr = numbers.at(3).toInt();
                op->alignVMode = numbers.at(4).toInt() + numbers.at(5).toInt();    /// FIXME: right or wrong?
                op->alignHMode = numbers.at(6).toInt() + numbers.at(7).toInt();    /// FIXME: right or wrong?
                op->alignArea = numbers.at(8).toInt();
                op->alignTarget = numbers.at(9).toInt();
            }
            else if (v_overlays.contains(key)) {
                const KArchiveEntry* e = zip.directory()->entry(value);
                const KZipFileEntry* pix = static_cast<const KZipFileEntry*>(e);
                if (pix) {
//                     v_overlays[ key ]->pixmap.loadFromData(pix->data());
                    OverlayPixmap* op = v_overlays[ key ];
                    QBuffer* d = new QBuffer;
                    d->setData(pix->data());
                    op->setDevice(d);
                    op->setFormat(value.endsWith(".gif") ? "gif" : "apng");
                    d->setParent(op);
                    Animator::self()->connectPreEditBarMovie(op);
                }
            }
        }
        else if (statusbar) {
            if (key == "pic") {
                const KArchiveEntry* e = zip.directory()->entry(value);
                const KZipFileEntry* pix = static_cast<const KZipFileEntry*>(e);
                if (pix) {
                    m_statusBarSkin = new QMovie;
//                     m_statusBarSkin.loadFromData(pix->data());
                    QBuffer* d = new QBuffer;
                    d->setData(pix->data());
                    m_statusBarSkin->setDevice(d);
                    m_statusBarSkin->setFormat(value.endsWith(".gif") ? "gif" : "apng");
                    d->setParent(m_statusBarSkin);
                    Animator::self()->connectStatusBarMovie(m_statusBarSkin);
                }
            }
#define LOAD_PWPIX_VALUE(p1, p2) \
    do { \
        QStringList pics = value.split(','); \
        const KArchiveEntry* e0 = zip.directory()->entry(pics.at(0)); \
        const KZipFileEntry* pix0 = static_cast<const KZipFileEntry*>(e0); \
        if (pix0) { \
            QPixmap& pix = m_pwpix[ p1 ]; \
            pix.loadFromData(pix0->data()); \
        } \
        const KArchiveEntry* e1 = zip.directory()->entry(pics.at(1)); \
        const KZipFileEntry* pix1 = static_cast<const KZipFileEntry*>(e1); \
        if (pix1) { \
            QPixmap& pix = m_pwpix[ p2 ]; \
            pix.loadFromData(pix1->data()); \
        } \
    } while(0);
            else if (key == "cn_en") {
                LOAD_PWPIX_VALUE(IM_Chinese, IM_Direct)
            }
            else if (key == "quan_shuang") {
                LOAD_PWPIX_VALUE(IM_Pinyin, IM_Shuangpin)
            }
            else if (key == "quan_ban") {
                LOAD_PWPIX_VALUE(Letter_Full, Letter_Half)
            }
            else if (key == "biaodian") {
                LOAD_PWPIX_VALUE(Punct_Full, Punct_Half)
            }
            else if (key == "fan_jian") {
                LOAD_PWPIX_VALUE(Chinese_Traditional, Chinese_Simplified)
            }
            else if (key == "softkeyboard") {
                const KArchiveEntry* e = zip.directory()->entry(value);
                const KZipFileEntry* pix = static_cast<const KZipFileEntry*>(e);
                if (pix) {
                    QPixmap& pwpix1 = m_pwpix[ SoftKeyboard_On ];
                    pwpix1.loadFromData(pix->data());
                    QPixmap& pwpix2 = m_pwpix[ SoftKeyboard_Off ];
                    pwpix2.loadFromData(pix->data());
                }
            }
            else if (key == "menu") {
                const KArchiveEntry* e = zip.directory()->entry(value);
                const KZipFileEntry* pix = static_cast<const KZipFileEntry*>(e);
                if (pix) {
                    QPixmap& pwpix = m_pwpix[ Setup ];
                    pwpix.loadFromData(pix->data());
                }
            }
#undef LOAD_PWPIX_VALUE
#define LOAD_PWPOS_VALUE(p1, p2) \
    do { \
        QStringList numbers = value.split(','); \
        int x = numbers.at(0).toInt(); \
        int y = numbers.at(1).toInt(); \
        m_pwpos[ p1 ] = QPoint(x, y); \
        m_pwpos[ p2 ] = QPoint(x, y); \
    } while(0);
            else if (key == "cn_en_pos") {
                LOAD_PWPOS_VALUE(IM_Chinese, IM_Direct)
            }
            else if (key == "quan_shuang_pos") {
                LOAD_PWPOS_VALUE(IM_Pinyin, IM_Shuangpin)
            }
            else if (key == "quan_ban_pos") {
                LOAD_PWPOS_VALUE(Letter_Full, Letter_Half)
            }
            else if (key == "biaodian_pos") {
                LOAD_PWPOS_VALUE(Punct_Full, Punct_Half)
            }
            else if (key == "fan_jian_pos") {
                LOAD_PWPOS_VALUE(Chinese_Simplified, Chinese_Traditional)
            }
            else if (key == "softkeyboard_pos") {
                LOAD_PWPOS_VALUE(SoftKeyboard_On, SoftKeyboard_Off)
            }
            else if (key == "menu_pos") {
                QStringList numbers = value.split(',');
                int top = numbers.at(0).toInt();
                int left = numbers.at(1).toInt();
                m_pwpos[ Setup ] = QPoint(left, top);
            }
#undef LOAD_PWPOS_VALUE
            else if (key.startsWith("custom") && key.endsWith("_display")) {
                QString name = key.left(key.length() - 8);
                if (!s_overlays.contains(name)) {
                    s_overlays.insert(name, new OverlayPixmap);
                }
            }
            else if (key.startsWith("custom") && key.endsWith("_pos")) {
                QString name = key.left(key.length() - 4);
                QStringList numbers = value.split(',');
                if (!s_overlays.contains(name)) {
                    s_overlays.insert(name, new OverlayPixmap);
                }
                OverlayPixmap* op = s_overlays[ name ];
                op->mt = numbers.at(1).toInt();
                op->ml = numbers.at(0).toInt();
            }
            else if (s_overlays.contains(key)) {
                const KArchiveEntry* e = zip.directory()->entry(value);
                const KZipFileEntry* pix = static_cast<const KZipFileEntry*>(e);
                if (pix) {
//                     s_overlays[ key ]->pixmap.loadFromData(pix->data());
                    OverlayPixmap* op = s_overlays[ key ];
                    QBuffer* d = new QBuffer;
                    d->setData(pix->data());
                    op->setDevice(d);
                    op->setFormat(value.endsWith(".gif") ? "gif" : "apng");
                    d->setParent(op);
                    Animator::self()->connectStatusBarMovie(op);
                }
            }
//             else if (key.endsWith("_pos")) {
//                 QStringList list = value.split(',');
//                 int x = list.at(0).trimmed().toInt();
//                 int y = list.at(1).trimmed().toInt();
//                 if (x != 0 && y != 0) {
//                     m_otherpos.append(QPoint(x, y));
//                 }
//             }
        }
    }
    while (!line.isNull());

    h_hsr = h1skin.width() - h_hsr;
    h_vsb = h1skin.height() - h_vsb;
    v_hsr = v1skin.width() - v_hsr;
    v_vsb = v1skin.height() - v_vsb;
    if (h_hsl > h_hsr) qSwap(h_hsl, h_hsr);
    if (h_vst > h_vsb) qSwap(h_vst, h_vsb);
    if (v_hsl > v_hsr) qSwap(v_hsl, v_hsr);
    if (v_vst > v_vsb) qSwap(v_vst, v_vsb);

    h_preEditBarSkin = SkinPixmap(h1skin, h_hsl, h_hsr, h_vst, h_vsb, h_hstm, h_vstm);
    v_preEditBarSkin = SkinPixmap(v1skin, v_hsl, v_hsr, v_vst, v_vsb, v_hstm, v_vstm);

    /// calculate overlay pixmap surrounding size
    calculateOverlaySurrounding(h_overlays, h_opt, h_opb, h_opl, h_opr);
    calculateOverlaySurrounding(v_overlays, v_opt, v_opb, v_opl, v_opr);

    m_preEditFont.setFamily(font_en);
    m_preEditFont.setPixelSize(fontPixelSize);
    m_preEditFont.setBold(true);
    m_candidateFont.setFamily(font_ch);
    m_candidateFont.setPixelSize(fontPixelSize);
    m_candidateFont.setBold(true);
    m_labelFont = m_candidateFont;

    m_preEditFontHeight = QFontMetrics(m_preEditFont).height();
    m_labelFontHeight = QFontMetrics(m_labelFont).height();
    m_candidateFontHeight = QFontMetrics(m_candidateFont).height();

    pinyin_color = pinyin_color.leftJustified(8, '0').replace("0x", "#");
    zhongwen_color = zhongwen_color.leftJustified(8, '0').replace("0x", "#");
    m_preEditColor = QColor(pinyin_color);
    m_candidateColor = QColor(zhongwen_color);
    m_labelColor = m_candidateColor;

    return true;
}

QSize ThemerSogou::sizeHintPreEditBar(const PreEditBar* widget) const
{
    const SkinPixmap& skin = KIMToySettings::self()->verticalPreeditBar()
                             ? v_preEditBarSkin : h_preEditBarSkin;
    int w = skin.skinw();
    int h = skin.skinh();

    if (KIMToySettings::self()->verticalPreeditBar()) {
        int widgetsh = v_pt + v_pb + v_zt + v_zb;

        /// preedit and aux
        int pinyinauxw = QFontMetrics(m_preEditFont).width(widget->m_text + widget->m_auxText);
        w = qMax(pinyinauxw + v_pl + v_pr + v_opl + v_opr, w);
        widgetsh += m_preEditFontHeight;

        /// lookuptable
        int lookuptablew = 0;
        int count = qMin(widget->m_labels.count(), widget->m_candidates.count());
        for (int i = 0; i < count; ++i) {
            QString tmp = widget->m_labels.at(i).trimmed() + widget->m_candidates.at(i).trimmed();
            lookuptablew = qMax(QFontMetrics(m_candidateFont).width(tmp), lookuptablew);
            widgetsh += m_candidateFontHeight;
        }
        w = qMax(lookuptablew + v_zl + v_zr + v_opl + v_opr, w);

        h = qMax(widgetsh + v_opt + v_opb, h);
    }
    else {
        int widgetsh = h_pt + h_pb + h_zt + h_zb;

        /// preedit and aux
        int pinyinauxw = QFontMetrics(m_preEditFont).width(widget->m_text + widget->m_auxText);
        w = qMax(pinyinauxw + h_pl + h_pr + h_opl + h_opr, w);
        widgetsh += m_preEditFontHeight;

        /// lookuptable
        QString tmp = widget->m_labels.join(QString());
        int count = qMin(widget->m_labels.count(), widget->m_candidates.count());
        for (int i = 0; i < count; ++i) {
            tmp += widget->m_labels.at(i).trimmed() + widget->m_candidates.at(i).trimmed();
        }
        int lookuptablew = QFontMetrics(m_candidateFont).width(tmp);
        w = qMax(lookuptablew + h_zl + h_zr + h_opl + h_opr, w);
        widgetsh += m_candidateFontHeight;

        h = qMax(widgetsh + h_opt + h_opb, h);
    }

    if (!KIMToySettings::self()->enablePreeditResizing()) {
        /// align with skin width + 70 * x
        const int align = 70;
        if (w > skin.skinw()) {
            w = skin.skinw() + ((w - skin.skinw() - 1) / align + 1) * align;
        }
    }

    return QSize(w, h);
}

QSize ThemerSogou::sizeHintStatusBar(const StatusBar* widget) const
{
    Q_UNUSED(widget);
    if (m_statusBarSkin)
        return m_statusBarSkin->currentPixmap().size();
    return QSize(0, 0);
}

void ThemerSogou::layoutStatusBar(StatusBarLayout* layout) const
{
    QList<QLayoutItem*> nopositems;
    QList<QPoint> remainpos = m_pwpos.values();
    int itemCount = layout->count();
    for (int i = 0; i < itemCount; ++i) {
        QLayoutItem* item = layout->m_items.at(i);
        PropertyWidget* pw = static_cast<PropertyWidget*>(item->widget());
        if (m_pwpos.contains(pw->type())) {
            item->setGeometry(QRect(m_pwpos.value(pw->type()), item->maximumSize()));
            remainpos.removeAll(m_pwpos.value(pw->type()));
        }
        else {
            nopositems.append(item);
        }
    }

    int nopositemCount = nopositems.count();
    for (int i = 0, j = 0, k = 0; i < nopositemCount; ++i) {
        QLayoutItem* item = nopositems.at(i);
        if (j < remainpos.count()) {
            item->setGeometry(QRect(remainpos.at(j), QSize(22, 22)));
            remainpos.removeAll(remainpos.at(j));
            ++j;
        }
        else {
            item->setGeometry(QRect(k * 22, 0, 22, 22));
            ++k;
        }
    }
}

void ThemerSogou::resizePreEditBar(const QSize& size)
{
    /// calculate mask if necessary
    if (KIMToySettings::self()->enableWindowMask()
            || KIMToySettings::self()->enableBackgroundBlur()
            || KIMToySettings::self()->backgroundColorizing()) {
        updatePreEditBarMask(size);
    }
}

void ThemerSogou::resizeStatusBar(const QSize& size)
{
    /// calculate mask if necessary
    if (KIMToySettings::self()->enableWindowMask()
            || KIMToySettings::self()->enableBackgroundBlur()
            || KIMToySettings::self()->backgroundColorizing()) {
        updateStatusBarMask(size);
    }
}

void ThemerSogou::updatePreEditBarMask(const QSize& size)
{
    int opt, opb, opl, opr;

    if (KIMToySettings::self()->verticalPreeditBar()) {
        v_preEditBarSkin.resizeRegion(size);
        m_preEditBarMask = v_preEditBarSkin.currentRegion();
        opt = v_opt, opb = v_opb, opl = v_opl, opr = v_opr;
    }
    else {
        h_preEditBarSkin.resizeRegion(size);
        m_preEditBarMask = h_preEditBarSkin.currentRegion();
        opt = h_opt, opb = h_opb, opl = h_opl, opr = h_opr;
    }

    /// overlay pixmap regions
    const QHash<QString, OverlayPixmap*>& overlays = KIMToySettings::self()->verticalPreeditBar()
            ? v_overlays : h_overlays;
    QHash<QString, OverlayPixmap*>::ConstIterator it = overlays.constBegin();
    QHash<QString, OverlayPixmap*>::ConstIterator end = overlays.constEnd();
    while (it != end) {
        const OverlayPixmap* op = it.value();
        const QPixmap& pixmap = op->currentPixmap();
        QRegion opRegion = pixmap.mask();
        switch (op->alignArea) {
            case 1:
                opRegion.translate(-op->mr, -op->mb);
                break;
            case 2:
                if (op->alignHMode == 0) {
                    opRegion.translate((size.width() + opl - opr - pixmap.width()) / 2, 0);
                    opRegion.translate(0, -op->mb);
                }
                else if (op->alignHMode == 1) {
                    opRegion.translate(opl, 0);
                    opRegion.translate(op->ml, -op->mb);
                }
                else if (op->alignHMode == 2) {
                    opRegion.translate(size.width() - opr - pixmap.width(), 0);
                    opRegion.translate(-op->mr, -op->mb);
                }
                break;
            case 3:
                opRegion.translate(size.width() - opr, 0);
                opRegion.translate(op->ml, -op->mb);
                break;
            case 4:
                if (op->alignVMode == 0) {
                    opRegion.translate(0, (size.height() - opb + opt - pixmap.height()) / 2);
                    opRegion.translate(-op->mr, 0);
                }
                else if (op->alignVMode == 1) {
                    opRegion.translate(0, opt);
                    opRegion.translate(-op->mr, op->mt);
                }
                else if (op->alignVMode == 2) {
                    opRegion.translate(0, size.height() - opb - pixmap.height());
                    opRegion.translate(-op->mr, -op->mb);
                }
                break;
            case 5:
                if (op->alignHMode == 0) {
                    opRegion.translate((size.width() + opl - opr - pixmap.width()) / 2, 0);
                }
                else if (op->alignHMode == 1) {
                    opRegion.translate(opl, 0);
                    opRegion.translate(op->ml, 0);
                }
                else if (op->alignHMode == 2) {
                    opRegion.translate(size.width() - opr - pixmap.width(), 0);
                    opRegion.translate(-op->mr, 0);
                }
                if (op->alignVMode == 0) {
                    opRegion.translate(0, (size.height() - opb + opt - pixmap.height()) / 2);
                }
                else if (op->alignVMode == 1) {
                    opRegion.translate(0, opt);
                    opRegion.translate(0, op->mt);
                }
                else if (op->alignVMode == 2) {
                    opRegion.translate(0, size.height() - opb - pixmap.height());
                    opRegion.translate(0, -op->mb);
                }
                break;
            case 6:
                if (op->alignVMode == 0) {
                    opRegion.translate(size.width() - opr, (size.height() - opb + opt - pixmap.height()) / 2);
                    opRegion.translate(op->ml, 0);
                }
                else if (op->alignVMode == 1) {
                    opRegion.translate(size.width() - opr, opt);
                    opRegion.translate(op->ml, op->mt);
                }
                else if (op->alignVMode == 2) {
                    opRegion.translate(size.width() - opr, size.height() - opb - pixmap.height());
                    opRegion.translate(op->ml, -op->mb);
                }
                break;
            case 7:
                opRegion.translate(0, size.height() - opb);
                opRegion.translate(-op->mr, op->mt);
                break;
            case 8:
                if (op->alignHMode == 0) {
                    opRegion.translate((size.width() + opl - opr - pixmap.width()) / 2, size.height() - opb);
                    opRegion.translate(0, op->mt);
                }
                else if (op->alignHMode == 1) {
                    opRegion.translate(opl, size.height() - opb);
                    opRegion.translate(op->ml, op->mt);
                }
                else if (op->alignHMode == 2) {
                    opRegion.translate(size.width() - opr - pixmap.width(), size.height() - opb);
                    opRegion.translate(-op->mr, op->mt);
                }
                break;
            case 9:
                opRegion.translate(size.width() - opr, size.height() - opb);
                opRegion.translate(op->ml, op->mt);
                break;
            default:
                /// never arrive here
                break;
        }
        m_preEditBarMask |= opRegion;
        ++it;
    }
}

void ThemerSogou::updateStatusBarMask(const QSize& size)
{
//     m_statusBarSkin = m_statusBarSkin.scaled(size);
if (m_statusBarSkin)
    m_statusBarMask = m_statusBarSkin->currentPixmap().mask();
}

void ThemerSogou::maskPreEditBar(PreEditBar* widget)
{
    widget->setMask(m_preEditBarMask);
}

void ThemerSogou::maskStatusBar(StatusBar* widget)
{
    QRegion mask = m_statusBarMask;
    foreach(const QLayoutItem* item, widget->m_layout->m_items) {
        mask |= item->geometry();
    }
    widget->setMask(mask);
}

void ThemerSogou::maskPropertyWidget(PropertyWidget* widget)
{
    if (m_pwpix.contains(widget->type()))
        widget->setMask(m_pwpix.value(widget->type()).mask());
    else if (!widget->iconName().isEmpty())
        widget->setMask(MainBarIcon(widget->iconName()).mask());
    else
        widget->clearMask();
}

void ThemerSogou::blurPreEditBar(PreEditBar* widget)
{
    Plasma::WindowEffects::enableBlurBehind(widget->winId(), true, m_preEditBarMask);
}

void ThemerSogou::blurStatusBar(StatusBar* widget)
{
    Plasma::WindowEffects::enableBlurBehind(widget->winId(), true, m_statusBarMask);
}

void ThemerSogou::drawPreEditBar(PreEditBar* widget)
{
    int pt = 0, pb = 0, pl = 0, pr = 0;
    int zt = 0, zb = 0, zl = 0, zr = 0;
    int opt = 0, opb = 0, opl = 0, opr = 0;
    QColor separatorColor = Qt::transparent;
    int sepl = 0, sepr = 0;

    QPainter p(widget);

    if (KIMToySettings::self()->backgroundColorizing()) {
        QPainterPath path;
        path.addRegion(m_preEditBarMask);
        p.fillPath(path, KIMToySettings::self()->preeditBarColorize());
    }

    if (KIMToySettings::self()->verticalPreeditBar()) {
        v_preEditBarSkin.drawPixmap(&p, widget->width(), widget->height());
        pt = v_pt, pb = v_pb, pl = v_pl, pr = v_pr;
        zt = v_zt, zb = v_zb, zl = v_zl, zr = v_zr;
        opt = v_opt, opb = v_opb, opl = v_opl, opr = v_opr;
        separatorColor = v_separatorColor;
        sepl = v_sepl, sepr = v_sepr;
    }
    else {
        h_preEditBarSkin.drawPixmap(&p, widget->width(), widget->height());
        pt = h_pt, pb = h_pb, pl = h_pl, pr = h_pr;
        zt = h_zt, zb = h_zb, zl = h_zl, zr = h_zr;
        opt = h_opt, opb = h_opb, opl = h_opl, opr = h_opr;
        separatorColor = h_separatorColor;
        sepl = h_sepl, sepr = h_sepr;
    }

    /// draw overlay pixmap
    const QHash<QString, OverlayPixmap*>& overlays = KIMToySettings::self()->verticalPreeditBar()
            ? v_overlays : h_overlays;
    QHash<QString, OverlayPixmap*>::ConstIterator it = overlays.constBegin();
    QHash<QString, OverlayPixmap*>::ConstIterator end = overlays.constEnd();
    while (it != end) {
        const OverlayPixmap* op = it.value();
        const QPixmap& pixmap = op->currentPixmap();
        p.save();
        switch (op->alignArea) {
            case 1:
                p.translate(-op->mr, -op->mb);
                break;
            case 2:
                if (op->alignHMode == 0) {
                    p.translate((widget->width() + opl - opr - pixmap.width()) / 2, 0);
                    p.translate(0, -op->mb);
                }
                else if (op->alignHMode == 1) {
                    p.translate(opl, 0);
                    p.translate(op->ml, -op->mb);
                }
                else if (op->alignHMode == 2) {
                    p.translate(widget->width() - opr - pixmap.width(), 0);
                    p.translate(-op->mr, -op->mb);
                }
                break;
            case 3:
                p.translate(widget->width() - opr, 0);
                p.translate(op->ml, -op->mb);
                break;
            case 4:
                if (op->alignVMode == 0) {
                    p.translate(0, (widget->height() - opb + opt - pixmap.height()) / 2);
                    p.translate(-op->mr, 0);
                }
                else if (op->alignVMode == 1) {
                    p.translate(0, opt);
                    p.translate(-op->mr, op->mt);
                }
                else if (op->alignVMode == 2) {
                    p.translate(0, widget->height() - opb - pixmap.height());
                    p.translate(-op->mr, -op->mb);
                }
                break;
            case 5:
                if (op->alignHMode == 0) {
                    p.translate((widget->width() + opl - opr - pixmap.width()) / 2, 0);
                }
                else if (op->alignHMode == 1) {
                    p.translate(opl, 0);
                    p.translate(op->ml, 0);
                }
                else if (op->alignHMode == 2) {
                    p.translate(widget->width() - opr - pixmap.width(), 0);
                    p.translate(-op->mr, 0);
                }
                if (op->alignVMode == 0) {
                    p.translate(0, (widget->height() - opb + opt - pixmap.height()) / 2);
                }
                else if (op->alignVMode == 1) {
                    p.translate(0, opt);
                    p.translate(0, op->mt);
                }
                else if (op->alignVMode == 2) {
                    p.translate(0, widget->height() - opb - pixmap.height());
                    p.translate(0, -op->mb);
                }
                break;
            case 6:
                if (op->alignVMode == 0) {
                    p.translate(widget->width() - opr, (widget->height() - opb + opt - pixmap.height()) / 2);
                    p.translate(op->ml, 0);
                }
                else if (op->alignVMode == 1) {
                    p.translate(widget->width() - opr, opt);
                    p.translate(op->ml, op->mt);
                }
                else if (op->alignVMode == 2) {
                    p.translate(widget->width() - opr, widget->height() - opb - pixmap.height());
                    p.translate(op->ml, -op->mb);
                }
                break;
            case 7:
                p.translate(0, widget->height() - opb);
                p.translate(-op->mr, op->mt);
                break;
            case 8:
                if (op->alignHMode == 0) {
                    p.translate((widget->width() + opl - opr - pixmap.width()) / 2, widget->height() - opb);
                    p.translate(0, op->mt);
                }
                else if (op->alignHMode == 1) {
                    p.translate(opl, widget->height() - opb);
                    p.translate(op->ml, op->mt);
                }
                else if (op->alignHMode == 2) {
                    p.translate(widget->width() - opr - pixmap.width(), widget->height() - opb);
                    p.translate(-op->mr, op->mt);
                }
                break;
            case 9:
                p.translate(widget->width() - opr, widget->height() - opb);
                p.translate(op->ml, op->mt);
                break;
            default:
                /// never arrive here
                break;
        }
//         p.drawPixmap(0, 0, op->pixmap);
        p.drawPixmap(0, 0, pixmap);
        p.restore();
        ++it;
    }

    if (separatorColor != Qt::transparent) {
        /// draw separator
        int sepy = opt + pt + m_preEditFontHeight + pb;
        p.drawLine(opl + sepl, sepy, widget->width() - opr - sepr, sepy);
    }

    p.translate(opl, opt);
    int y = 0;

    y += pt;
    if (widget->preeditVisible || widget->auxVisible) {
        /// draw preedit / aux text
        p.save();
        p.setFont(m_preEditFont);
        p.setPen(m_preEditColor);

        p.drawText(pl, pt, widget->width() - pl - pr, m_preEditFontHeight, Qt::AlignLeft, widget->m_text + widget->m_auxText);
        if (widget->preeditVisible) {
            int pixelsWide = QFontMetrics(m_preEditFont).width(widget->m_text.left(widget->m_cursorPos));
            p.drawLine(pl + pixelsWide, pt, pl + pixelsWide, pt + m_preEditFontHeight);
        }
        p.restore();
    }
    /// always preserve space when theme enabled
    y += m_preEditFontHeight + pb;


    if (widget->lookuptableVisible) {
        /// draw lookup table
        int x = zl;
        y += zt;
        int w = 0;
        int h = qMax(m_labelFontHeight, m_candidateFontHeight);

        /// draw labels and candidates
        int count = qMin(widget->m_labels.count(), widget->m_candidates.count());

        if (KIMToySettings::self()->verticalPreeditBar()) {
            for (int i = 0; i < count; ++i) {
                /// draw label
                p.setFont(m_labelFont);
                p.setPen(m_labelColor);
                x = zl;
                w = p.fontMetrics().width(widget->m_labels.at(i));
                p.drawText(x, y, w, h, Qt::AlignCenter, widget->m_labels.at(i));
                x += w;
                /// draw candidate
                p.setFont(m_candidateFont);
                p.setPen(m_candidateColor);
                w = p.fontMetrics().width(widget->m_candidates.at(i).trimmed());
                p.drawText(x, y, w, h, Qt::AlignCenter, widget->m_candidates.at(i).trimmed());
                y += h;
            }
        }
        else {
            for (int i = 0; i < count; ++i) {
                /// draw label
                p.setFont(m_labelFont);
                p.setPen(m_labelColor);
                w = p.fontMetrics().width(widget->m_labels.at(i));
                p.drawText(x, y, w, h, Qt::AlignCenter, widget->m_labels.at(i));
                x += w;
                /// draw candidate
                p.setFont(m_candidateFont);
                p.setPen(m_candidateColor);
                w = p.fontMetrics().width(widget->m_candidates.at(i).trimmed() + ' ');
                p.drawText(x, y, w, h, Qt::AlignCenter, widget->m_candidates.at(i).trimmed() + ' ');
                x += w;
            }
        }
    }
}

void ThemerSogou::drawStatusBar(StatusBar* widget)
{
    QPainter p(widget);

    if (KIMToySettings::self()->backgroundColorizing()) {
        QPainterPath path;
        path.addRegion(m_statusBarMask);
        p.fillPath(path, KIMToySettings::self()->statusBarColorize());
    }

    if (m_statusBarSkin)
    p.drawPixmap(0, 0, m_statusBarSkin->currentPixmap());

    /// draw overlay pixmap
    QHash<QString, OverlayPixmap*>::ConstIterator it = s_overlays.constBegin();
    QHash<QString, OverlayPixmap*>::ConstIterator end = s_overlays.constEnd();
    while (it != end) {
        const OverlayPixmap* op = it.value();
        p.drawPixmap(op->ml, op->mt, op->currentPixmap());
        ++it;
    }
}

void ThemerSogou::drawPropertyWidget(PropertyWidget* widget)
{
    QPainter p(widget);
    if (m_pwpix.contains(widget->type()))
        p.drawPixmap(0, 0, m_pwpix.value(widget->type()));
    else if (!widget->iconName().isEmpty())
        p.drawPixmap(0, 0, MainBarIcon(widget->iconName()));
    else
        p.drawText(widget->rect(), Qt::AlignCenter, widget->name());
}
