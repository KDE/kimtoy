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

#ifndef KSSF_H
#define KSSF_H

#include <QTemporaryFile>
#include <KArchive>
#include <KZip>

class KSsf : public KZip
{
public:
    explicit KSsf(const QString& filename);

    explicit KSsf(QIODevice* dev);

    virtual ~KSsf();

protected:
    /// Reimplemented from KArchive
    virtual bool doWriteSymLink(const QString& name, const QString& target,
                                const QString& user, const QString& group,
                                mode_t perm, const QDateTime& atime, const QDateTime& mtime, const QDateTime& ctime);

    /// Reimplemented from KArchive
    virtual bool doWriteDir(const QString& name, const QString& user, const QString& group,
                            mode_t perm, const QDateTime& atime, const QDateTime& mtime, const QDateTime& ctime);

    /// Reimplemented from KArchive
    virtual bool doPrepareWriting(const QString& name, const QString& user,
                                  const QString& group, qint64 size, mode_t perm,
                                  const QDateTime& atime, const QDateTime& mtime, const QDateTime& ctime);

    /// Reimplemented from KArchive
    virtual bool doFinishWriting(qint64 size);

    virtual bool openArchive(QIODevice::OpenMode mode);
    virtual bool closeArchive();

protected:
    virtual void virtual_hook(int id, void* data);
private:
    bool isZip;
    QTemporaryFile* tmpFile;
};

#endif // KSSF_H
