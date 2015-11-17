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

// ase decrypt
#include <openssl/aes.h>

#include <KArchive>
#include <KZip>

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
                          mode_t perm, const QDateTime& atime, const QDateTime& mtime, const QDateTime& ctime)
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
                      mode_t perm, const QDateTime& atime, const QDateTime& mtime, const QDateTime& ctime)
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
                            const QDateTime& atime, const QDateTime& mtime, const QDateTime& ctime)
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

    // decrypt aes-cbc
    AES_KEY dec_key;

    static const unsigned char aeskey[] =
    {
        0x52,0x36,0x46,0x1A,0xD3,0x85,0x03,0x66,
        0x90,0x45,0x16,0x28,0x79,0x03,0x36,0x23,
        0xDD,0xBE,0x6F,0x03,0xFF,0x04,0xE3,0xCA,
        0xD5,0x7F,0xFC,0xA3,0x50,0xE4,0x9E,0xD9
    };

    AES_set_decrypt_key(aeskey, 256, &dec_key);

    unsigned char iv[AES_BLOCK_SIZE] =
    {
        0xE0,0x7A,0xAD,0x35,0xE0,0x90,0xAA,0x03,
        0x8A,0x51,0xFD,0x05,0xDF,0x8C,0x5D,0x0F
    };

    QByteArray encdata = dev->readAll();
    QByteArray decdata = encdata;
    AES_cbc_encrypt((const unsigned char*)encdata.constData(), (unsigned char*)decdata.data(), encdata.size(), &dec_key, iv, AES_DECRYPT);

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
    tmpFile = new QTemporaryFile();
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
//         int date = 0;
        KArchiveEntry* entry = new KArchiveFile(this, filename, mode, QDateTime::currentDateTime(),
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
