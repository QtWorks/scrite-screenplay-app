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

#ifndef NOTE_H
#define NOTE_H

#include <QColor>
#include <QObject>
#include <QVariant>

#include "qobjectserializer.h"

class Scene;
class Structure;
class Character;

class Note : public QObject, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)

public:
    Q_INVOKABLE Note(QObject *parent=nullptr);
    ~Note();
    Q_SIGNAL void aboutToDelete(Note *ptr);

    Q_PROPERTY(Structure* structure READ structure CONSTANT STORED false)
    Structure *structure() const { return m_structure; }

    Q_PROPERTY(Character* character READ character CONSTANT STORED false)
    Character *character() const { return m_character; }

    enum Type
    {
        TextNote, // content = "the note text", filePath = ""
        UrlNote,  // content = "<the url>", filePath = ""
        PhotoNote, // content = "the description", filePath = <path to photo in DFS>
        AudioNote, // content = "the description", filePath = <path to audio in DFS>
        VideoNote, // content = "the description", filePath = <path to video in DFS>
        FileNote // content = "the description", filePath = <path to file in DFS>
    };
    Q_ENUM(Type)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)
    void setType(Type val);
    Type type() const { return m_type; }
    Q_SIGNAL void typeChanged();

    Q_PROPERTY(QString heading READ heading WRITE setHeading NOTIFY headingChanged)
    void setHeading(const QString &val);
    QString heading() const { return m_heading; }
    Q_SIGNAL void headingChanged();

    Q_PROPERTY(QString content READ content WRITE setContent NOTIFY contentChanged)
    void setContent(const QString &val);
    QString content() const { return m_content; }
    Q_SIGNAL void contentChanged();

    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged STORED false)
    void setFilePath(const QString &val);
    QString filePath() const { return m_filePath; }
    Q_SIGNAL void filePathChanged();

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    void setColor(const QColor &val);
    QColor color() const { return m_color; }
    Q_SIGNAL void colorChanged();

    Q_PROPERTY(bool fetchingUrlInfo READ isFetchingUrlInfo NOTIFY fetchingUrlInfoChanged)
    bool isFetchingUrlInfo() const { return m_fetchingUrlInfo; }
    Q_SIGNAL void fetchingUrlInfoChanged();

    Q_PROPERTY(QJsonObject urlInfo READ urlInfo NOTIFY urlInfoChanged)
    QJsonObject urlInfo() const { return m_urlInfo; }
    Q_SIGNAL void urlInfoChanged();

    // QObjectSerializer::Interface interface
    void serializeToJson(QJsonObject &json) const;
    void deserializeFromJson(const QJsonObject &json);

    Q_SIGNAL void noteChanged();

protected:
    void setUrlInfo(const QJsonObject &val);
    bool event(QEvent *event);
    void fetchUrlInfo();
    void setFetchingUrlInfo(bool val);

private:
    QColor m_color = QColor(Qt::white);
    Type m_type = TextNote;
    QString m_heading;
    QString m_content;
    QString m_filePath;
    QJsonObject m_urlInfo;
    bool m_fetchingUrlInfo = false;
    Structure *m_structure = nullptr;
    Character *m_character = nullptr;
};

#endif // NOTE_H
