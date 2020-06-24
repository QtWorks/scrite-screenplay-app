/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "documentfilesystem.h"

#include <QDataStream>
#include <QDir>
#include <QTemporaryDir>

struct DocumentFileSystemData
{
    QScopedPointer<QTemporaryDir> folder;
    QByteArray header;
    QList<DocumentFile*> files;

    void pack(QDataStream &ds, const QString &path);
};

void DocumentFileSystemData::pack(QDataStream &ds, const QString &path)
{
    const QFileInfo fi(path);
    if( fi.isDir() )
    {
        const QFileInfoList fiList = QDir(path).entryInfoList(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot, QDir::Name|QDir::DirsLast);
        Q_FOREACH(QFileInfo fi2, fiList)
            this->pack(ds, fi2.absoluteFilePath());
    }
    else
    {
        const QString filePath = fi.absoluteFilePath();

        QFile file(filePath);
        const QByteArray fileContents = file.open(QFile::ReadOnly) ? file.readAll() : QByteArray();

        QDir fsDir(this->folder->path());
        ds << fsDir.relativeFilePath(filePath) << fileContents;
    }
}

Q_GLOBAL_STATIC(QByteArray, DocumentFileSystemMaker)

void DocumentFileSystem::setMarker(const QByteArray &marker)
{
    if(::DocumentFileSystemMaker->isEmpty())
        *::DocumentFileSystemMaker = marker;
}

DocumentFileSystem::DocumentFileSystem(QObject *parent)
    : QObject(parent),
      d(new DocumentFileSystemData)
{
    this->reset();
}

DocumentFileSystem::~DocumentFileSystem()
{
    delete d;
}

void DocumentFileSystem::reset()
{
    d->header.clear();

    while(!d->files.isEmpty())
    {
        DocumentFile *file = d->files.first();
        file->close();
    }

    d->folder.reset(new QTemporaryDir);
}

bool DocumentFileSystem::load(const QString &fileName)
{
    this->reset();

    QFile file(fileName);
    if(!file.open(QFile::ReadOnly))
        return false;

    const int markerLength = ::DocumentFileSystemMaker->length();
    const QByteArray marker = file.read(markerLength);
    if(marker != *::DocumentFileSystemMaker)
        return false;

    QDataStream ds(&file);
    return this->unpack(ds);
}

bool DocumentFileSystem::save(const QString &fileName)
{
    QFile file(fileName);
    if( !file.open(QFile::WriteOnly) )
        return false;

    file.write(*::DocumentFileSystemMaker);

    QDataStream ds(&file);
    if( !this->pack(ds) )
    {
        file.close();
        QFile::remove(fileName);
        return false;
    }

    file.close();
    return true;
}

void DocumentFileSystem::setHeader(const QByteArray &header)
{
    d->header = header;
}

QByteArray DocumentFileSystem::header() const
{
    return d->header;
}

QFile *DocumentFileSystem::open(const QString &path, QFile::OpenMode mode)
{
    const QString completePath = d->folder->filePath(path);
    if( !QFile::exists(completePath) && mode == QIODevice::ReadOnly )
        return nullptr;

    DocumentFile *file = new DocumentFile(completePath, this);
    if( !file->open(mode) )
    {
        delete file;
        return nullptr;
    }

    return  file;
}

QByteArray DocumentFileSystem::read(const QString &path)
{
    QByteArray ret;

    const QString completePath = d->folder->filePath(path);
    if( !QFile::exists(completePath) )
        return ret;

    DocumentFile file(completePath, this);
    if( !file.open(QFile::ReadOnly) )
        return ret;

    ret = file.readAll();
    file.close();

    return ret;
}

bool DocumentFileSystem::write(const QString &path, const QByteArray &bytes)
{
    const QString completePath = d->folder->filePath(path);
    const QFileInfo fi(completePath);

    if( !fi.exists() )
    {
        if( !QDir().mkpath(fi.absolutePath()) )
            return false;
    }

    DocumentFile file(completePath, this);
    if( !file.open(QFile::WriteOnly) )
        return false;

    file.write(bytes);
    return true;
}

bool DocumentFileSystem::exists(const QString &path) const
{
    const QString completePath = d->folder->filePath(path);
    return QFile::exists(completePath);
}

QFileInfo DocumentFileSystem::fileInfo(const QString &path) const
{
    const QString completePath = d->folder->filePath(path);
    return QFileInfo(completePath);
}

bool DocumentFileSystem::pack(QDataStream &ds)
{
    const QByteArray compressedHeader = d->header.isEmpty() ? d->header : qCompress(d->header);

    ds << compressedHeader;
    d->pack(ds, d->folder->path());

    return true;
}

bool DocumentFileSystem::unpack(QDataStream &ds)
{
    QByteArray compressedHeader;
    ds >> compressedHeader;

    d->header = compressedHeader.isEmpty() ? compressedHeader : qUncompress(compressedHeader);

    const QDir folderPath(d->folder->path());

    while(!ds.atEnd())
    {
        QString relativeFilePath;
        QByteArray fileContents;
        ds >> relativeFilePath >> fileContents;

        const QString absoluteFilePath = folderPath.absoluteFilePath(relativeFilePath);
        const QFileInfo fi(absoluteFilePath);

        if( !QDir().mkpath(fi.absolutePath()) )
            return false;

        QFile file(absoluteFilePath);
        if( !file.open(QFile::WriteOnly) )
            return false;

        file.write(fileContents);
        file.close();
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

DocumentFile::DocumentFile(const QString &filePath, DocumentFileSystem *parent)
    : QFile(filePath, parent),
      m_fileSystem(parent)
{
    if(m_fileSystem != nullptr)
    {
        m_fileSystem->d->files.append(this);
        connect(this, &QIODevice::aboutToClose, this, &DocumentFile::onAboutToClose);
    }
}

DocumentFile::~DocumentFile()
{
    if(m_fileSystem != nullptr)
    {
        m_fileSystem->d->files.removeOne(this);
        m_fileSystem = nullptr;
    }
}

void DocumentFile::onAboutToClose()
{
    if(m_fileSystem != nullptr)
    {
        m_fileSystem->d->files.removeOne(this);
        m_fileSystem = nullptr;
    }
}

