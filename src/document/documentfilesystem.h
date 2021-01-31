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

#ifndef DOCUMENTFILESYSTEM_H
#define DOCUMENTFILESYSTEM_H

#include <QObject>

#include <QFile>
#include <QSize>
#include <QImage>
#include <QFileInfo>

class DocumentFile;

struct DocumentFileSystemData;
class DocumentFileSystem : public QObject
{
public:
    static void setMarker(const QByteArray &marker);

    DocumentFileSystem(QObject *parent=nullptr);
    ~DocumentFileSystem();

    enum Format { UnknownFormat, ScriteFormat, ZipFormat };

    void reset();
    bool load(const QString &fileName, Format *format=nullptr);
    bool save(const QString &fileName);

    void setHeader(const QByteArray &header);
    QByteArray header() const;

    QFile *open(const QString &path, QFile::OpenMode mode=QFile::ReadOnly);

    QByteArray read(const QString &path);
    bool write(const QString &path, const QByteArray &bytes);

    QString add(const QString &fileName, const QString &ns=QString());
    QString duplicate(const QString &fileName, const QString &ns=QString());
    bool remove(const QString &path);

    QString absolutePath(const QString &path, bool mkpath=false) const;
    QString relativePath(const QString &path) const;
    bool contains(const QString &path) const;

    bool exists(const QString &path) const;
    QFileInfo fileInfo(const QString &path) const;

    // API to add/replace/remove an external file into the DFS under a specific path/name
    QString addFile(const QString &srcFile, const QString &dstPath, bool replaceIfExists=true);
    QString addImage(const QString &srcFile, const QString &dstPath, const QSize &scaleTo=QSize(), bool replaceIfExists=true);
    QString addImage(const QImage &srcImage, const QString &dstPath, const QSize &scaleTo=QSize(), bool replaceIfExists=true);

private:
    bool pack(QDataStream &ds);
    bool unpack(QDataStream &ds);

private:
    friend class DocumentFile;
    DocumentFileSystemData *d;
};

class DocumentFile : public QFile
{
public:
    ~DocumentFile();

private:
    DocumentFile(const QString &filePath, DocumentFileSystem *parent=nullptr);
    void onAboutToClose();

private:
    friend class DocumentFileSystem;
    DocumentFileSystem *m_fileSystem = nullptr;
};

#endif // DOCUMENTFILESYSTEM_H
