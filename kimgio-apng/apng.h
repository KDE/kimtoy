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

#ifndef QAPNGHANDLER_H
#define QAPNGHANDLER_H

#include <QImageIOHandler>
#include <QImageIOPlugin>

class QAPngHandlerPrivate;
class QAPngHandler : public QImageIOHandler
{
public:
    QAPngHandler();
    virtual ~QAPngHandler();
    virtual bool canRead() const;
    virtual QByteArray name() const;
    virtual bool read(QImage* image);
    virtual int currentImageNumber() const;
    virtual int imageCount() const;
    virtual int loopCount() const;
    virtual int nextImageDelay() const;
    static bool canRead(QIODevice* device);
    virtual QVariant option(ImageOption option) const;
    virtual void setOption(ImageOption option, const QVariant& value);
    virtual bool supportsOption(ImageOption option) const;
private:
    QAPngHandlerPrivate* const d;
};

class QAPngPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "apng.json")
public:
    virtual Capabilities capabilities(QIODevice* device, const QByteArray& format) const;
    virtual QImageIOHandler* create(QIODevice* device, const QByteArray& format = QByteArray()) const;
};

#endif // QAPNGHANDLER_H
