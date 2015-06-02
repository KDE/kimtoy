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

#include "kssf.h"

#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QFile>

#include <QtCrypto>

#include <KArchive>
#include <KZip>
#include <KTemporaryFile>

KSsf::KSsf(const QString& filename)
        : KZip(filename)
{
    isZip = true;
    tmpFile = 0;
}

KSsf::KSsf(QIODevice* dev)
        : KZip(dev)
{
    isZip = true;
    tmpFile = 0;
}

KSsf::~KSsf()
{
    if(isOpen())
        close();

    delete tmpFile;
}

bool KSsf::doWriteSymLink(const QString &name, const QString &target,
                          const QString &user, const QString &group,
                          mode_t perm, time_t atime, time_t mtime, time_t ctime)
{
    Q_UNUSED(name)
    Q_UNUSED(target)
    Q_UNUSED(user)
    Q_UNUSED(group)
    Q_UNUSED(perm)
    Q_UNUSED(atime)
    Q_UNUSED(mtime)
    Q_UNUSED(ctime)
    return false;
}

bool KSsf::doWriteDir(const QString& name, const QString& user, const QString& group,
                      mode_t perm, time_t atime, time_t mtime, time_t ctime)
{
    Q_UNUSED(name)
    Q_UNUSED(user)
    Q_UNUSED(group)
    Q_UNUSED(perm)
    Q_UNUSED(atime)
    Q_UNUSED(mtime)
    Q_UNUSED(ctime)
    return false;
}

bool KSsf::doPrepareWriting(const QString& name, const QString& user,
                            const QString& group, qint64 size, mode_t perm,
                            time_t atime, time_t mtime, time_t ctime)
{
    Q_UNUSED(name)
    Q_UNUSED(user)
    Q_UNUSED(group)
    Q_UNUSED(size)
    Q_UNUSED(perm)
    Q_UNUSED(atime)
    Q_UNUSED(mtime)
    Q_UNUSED(ctime)
    return false;
}

bool KSsf::doFinishWriting(qint64 size)
{
    Q_UNUSED(size)
    return false;
}

bool KSsf::openArchive(QIODevice::OpenMode mode)
{
    if (mode != QIODevice::ReadOnly) {
        qWarning() << "Unsupported mode " << mode;
        return false;
    }

    QIODevice* dev = device();
    if (!dev)
        return false;

    QByteArray magic = dev->read(8);
    if (magic == QByteArray::fromHex("536B696E03000000")) {
        qWarning() << "detected encrypted ssf archive";
        isZip = false;
    }

    if (isZip)
        return KZip::openArchive(mode);

    QCA::Initializer init;

    // decrypt aes-cbc
    if (!QCA::isSupported("aes256-cbc")) {
        qWarning() << "AES256-CBC not supported";
        return false;
    }

    QByteArray aeskey = QByteArray::fromHex("5236461AD38503669045162879033623DDBE6F03FF04E3CAD57FFCA350E49ED9");
    QByteArray aesiv = QByteArray::fromHex("E07AAD35E090AA038A51FD05DF8C5D0F");

    QCA::SymmetricKey key(aeskey);
    QCA::InitializationVector iv(aesiv);

    QCA::Cipher cipher("aes256", QCA::Cipher::CBC);
    cipher.setup(QCA::Decode, key, iv);

    QByteArray encdata = dev->readAll();
    QByteArray decdata = cipher.process(encdata).toByteArray();

    if (!cipher.ok()) {
        qWarning() << "decrypt failed";
        return false;
    }

    qWarning() << "decrypt success";

    // convert header byte order
    QDataStream dds(&decdata, QIODevice::ReadWrite);
    dds.setByteOrder(QDataStream::LittleEndian);
    quint32 plainlen;
    dds >> plainlen;
    dds.device()->reset();
    dds.setByteOrder(QDataStream::BigEndian);
    dds << plainlen;

    // zlib uncompress
    QByteArray plaindata = qUncompress(decdata);

    if (plaindata.isEmpty()) {
        qWarning() << "uncompress failed";
        return false;
    }

    qWarning() << "uncompress success";

    // create plain temp file
    tmpFile = new KTemporaryFile();
    tmpFile->setPrefix("kssf-");
    tmpFile->setSuffix(".plain");
    tmpFile->open();

    tmpFile->write(plaindata);

    qWarning() << "creating tempfile:" << tmpFile->fileName();

    setDevice(tmpFile);

    // read offset table
    QDataStream ds(plaindata);
    ds.setByteOrder(QDataStream::LittleEndian);

    quint32 size;
    ds >> size;
    quint32 offsettablesize;
    ds >> offsettablesize;

    QVector<quint32> offsets;
    int offsetcount = offsettablesize / sizeof(quint32);
    for (int i = 0; i < offsetcount; i++)
    {
        quint32 offset;
        ds >> offset;
        offsets.append(offset);
    }

    // read chunk
    foreach (quint32 offset, offsets)
    {
        ds.device()->seek(offset);

        quint32 namelen;
        ds >> namelen;

        // utf-16 string
        char* utf16str = new char[namelen];
        ds.readRawData(utf16str, namelen);
        QString filename = QString::fromUtf16((quint16*)utf16str, namelen / 2);
        delete[] utf16str;

//         qWarning() << filename;

        quint32 contentlen;
        ds >> contentlen;

        quint32 contentoffset = ds.device()->pos();

        int mode = 644;
        int date = 0;
        KArchiveEntry* entry = new KArchiveFile(this, filename, mode, date,
                                                /*uid*/ QString(), /*gid*/ QString(), /*symlink*/ QString(),
                                                contentoffset, contentlen);
        rootDir()->addEntry(entry);
    }

    return true;
}

bool KSsf::closeArchive()
{
    if (isZip)
        return KZip::closeArchive();

    tmpFile->close();

    return true;
}

void KSsf::virtual_hook(int id, void* data)
{
    KZip::virtual_hook(id, data);
}

// int main(int argc, char** argv)
// {
//     QString file = "b.ssf";
//
//     KSsf ssf(file);
//     if (!ssf.open(QIODevice::ReadOnly)) {
//         return -1;
//     }
//
//     const KArchiveEntry* entry = ssf.directory()->entry("skin.ini");
//     const KArchiveFile* skinini = static_cast<const KArchiveFile*>(entry);
//
//     if (!skinini) {
//         entry = ssf.directory()->entry("Skin.ini");
//         skinini = static_cast<const KArchiveFile*>(entry);
//         if (!skinini)
//             return -1;
//     }
//
//     qWarning() << "open skin.ini success";
//
//     return 0;
// }
