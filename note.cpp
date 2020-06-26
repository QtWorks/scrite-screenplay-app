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

#include "note.h"
#include "scene.h"
#include "structure.h"
#include "networkaccess.h"
#include "scritedocument.h"
#include "documentfilesystem.h"

Note::Note(QObject *parent)
     : QObject(parent),
       m_structure(qobject_cast<Structure*>(parent)),
       m_character(qobject_cast<Character*>(parent))
{
    connect(this, &Note::typeChanged, this, &Note::noteChanged);
    connect(this, &Note::urlInfoChanged, this, &Note::noteChanged);
    connect(this, &Note::headingChanged, this, &Note::noteChanged);
    connect(this, &Note::contentChanged, this, &Note::noteChanged);
    connect(this, &Note::filePathChanged, this, &Note::noteChanged);
}

Note::~Note()
{

}

void Note::setType(Note::Type val)
{
    if(m_type == val)
        return;

    m_type = val;
    emit typeChanged();

    this->fetchUrlInfo();
}

void Note::setHeading(const QString &val)
{
    if(m_heading == val)
        return;

    m_heading = val;
    emit headingChanged();
}

void Note::setContent(const QString &val)
{
    if(m_content == val)
        return;

    m_content = val;
    emit contentChanged();

    this->fetchUrlInfo();
}

void Note::setFilePath(const QString &val)
{
    if(m_filePath == val)
        return;

    DocumentFileSystem *dfs = ScriteDocument::instance()->fileSystem();
    if(dfs->contains(m_filePath))
        return;

    if(!m_filePath.isEmpty())
        dfs->remove(m_filePath);

    m_filePath = dfs->add(val, "note");
    emit filePathChanged();
}

void Note::setColor(const QColor &val)
{
    if(m_color == val)
        return;

    m_color = val;
    emit colorChanged();
}

void Note::setUrlInfo(const QJsonObject &val)
{
    if(m_urlInfo == val)
        return;

    m_urlInfo = val;
    emit urlInfoChanged();

    this->setFetchingUrlInfo(false);
}

void Note::serializeToJson(QJsonObject &json) const
{
    json.insert("filePath", m_filePath);

    if(m_type == UrlNote)
        json.insert("urlInfo", m_urlInfo);
}

void Note::deserializeFromJson(const QJsonObject &json)
{
    const QString fPath = json.value("filePath").toString();
    if(fPath != m_filePath)
    {
        m_filePath = fPath;
        emit filePathChanged();
    }

    if(m_type == UrlNote)
    {
        const QJsonObject ui = json.value("urlInfo").toObject();
        this->setUrlInfo(ui);
    }
}

bool Note::event(QEvent *event)
{
    if(event->type() == QEvent::ParentChange)
    {
        m_structure = qobject_cast<Structure*>(this->parent());
        m_character = qobject_cast<Character*>(this->parent());
    }

    return QObject::event(event);
}

void Note::fetchUrlInfo()
{
    if(m_type == UrlNote && !m_content.isEmpty())
    {
        QUrl url(m_content);
        if(!url.isValid())
            return;

        this->setFetchingUrlInfo(true);
        FetchOpenGraphAttributes *fetchAttribs = new FetchOpenGraphAttributes(url, this);
        fetchAttribs->setAutoDelete(true);
        connect(fetchAttribs, &FetchOpenGraphAttributes::attributesFetched, this, &Note::setUrlInfo);
    }
    else
        this->setUrlInfo(QJsonObject());
}

void Note::setFetchingUrlInfo(bool val)
{
    if(m_fetchingUrlInfo == val)
        return;

    m_fetchingUrlInfo = val;
    emit fetchingUrlInfoChanged();
}

