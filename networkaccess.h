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

#ifndef NETWORKACCESS_H
#define NETWORKACCESS_H

#include "errorreport.h"

#include <QJsonObject>
#include <QQmlParserStatus>
#include <QNetworkAccessManager>

class NetworkAccess : public QObject
{
    Q_OBJECT

public:
    static NetworkAccess &instance();
    ~NetworkAccess();

    Q_PROPERTY(QNetworkAccessManager* manager READ manager CONSTANT)
    QNetworkAccessManager *manager() const { return m_nam; }

private:
    NetworkAccess(QObject *parent=nullptr);
    void onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

private:
    ErrorReport *m_errorReport = nullptr;
    QNetworkAccessManager *m_nam = nullptr;
};

struct OpenGraphAttributes
{
    QUrl url;
    QUrl image;
    QString type;
    QString title;
    QString description;
};

class FetchOpenGraphAttributes : public QObject
{
    Q_OBJECT

public:
    FetchOpenGraphAttributes(const QUrl &url, QObject *parent=nullptr);
    ~FetchOpenGraphAttributes();

    void setAutoDelete(bool val);
    bool isAutoDelete() const { return m_autoDelete; }

    Q_SIGNAL void attributesFetched(const QJsonObject &attribs);

private:
    void onWebPageLoadFinished(bool ok);
    void useAttributes(const QJsonObject &attribs);

private:
    QUrl m_url;
    bool m_autoDelete = true;
    bool m_attribsFetched = false;
    QJsonObject m_defaultAttribs;
};

#endif // NETWORKACCESS_H
