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

#include "networkaccess.h"
#include "garbagecollector.h"

#include <QFile>
#include <QNetworkReply>
#include <QWebEnginePage>
#include <QCoreApplication>

NetworkAccess &NetworkAccess::instance()
{
    static NetworkAccess *theInstance = new NetworkAccess(qApp);
    return *theInstance;
}

NetworkAccess::NetworkAccess(QObject *parent)
    : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
    connect(m_nam, &QNetworkAccessManager::sslErrors, this, &NetworkAccess::onSslErrors);
}

NetworkAccess::~NetworkAccess()
{

}

void NetworkAccess::onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    QStringList sslErrors;
    Q_FOREACH(QSslError error, errors)
        sslErrors << error.errorString();
    m_errorReport->setErrorMessage("SSL Error: " + sslErrors.join(", "));
    reply->abort();
}

///////////////////////////////////////////////////////////////////////////////

FetchOpenGraphAttributes::FetchOpenGraphAttributes(const QUrl &url, QObject *parent)
    : QObject(parent)
{
    QWebEnginePage *webPage = new QWebEnginePage(this);
    webPage->setUrl(url);
    connect(webPage, &QWebEnginePage::loadFinished, this, &FetchOpenGraphAttributes::onWebPageLoadFinished);
}

FetchOpenGraphAttributes::~FetchOpenGraphAttributes()
{

}

void FetchOpenGraphAttributes::setAutoDelete(bool val)
{
    m_autoDelete = val;
}

void FetchOpenGraphAttributes::onWebPageLoadFinished(bool)
{
    QWebEnginePage *webPage = qobject_cast<QWebEnginePage*>(this->sender());

    static QString jsCode;
    if(jsCode.isEmpty())
    {
        QFile jsFile(":/js/fetchogattribs.js");
        if(jsFile.open(QFile::ReadOnly))
            jsCode = jsFile.readAll();
    }

    m_defaultAttribs = QJsonObject();
    m_defaultAttribs.insert("url", webPage->url().toString());
    m_defaultAttribs.insert("type", "website");
    m_defaultAttribs.insert("title", webPage->title());

    if(jsCode.isEmpty())
    {
        this->useAttributes(m_defaultAttribs);
        return;
    }

    webPage->runJavaScript(jsCode, [this](const QVariant &result) {
        this->useAttributes(result.toJsonObject());
    });

    /**
      Ref: https://doc.qt.io/qt-5/qwebenginepage.html#runJavaScript-3

      Documentation for runJavaScript says

      Warning: We guarantee that the callback (resultCallback) is always called,
      but it might be done during page destruction. When QWebEnginePage is deleted,
      the callback is triggered with an invalid value and it is not safe to use
      the corresponding QWebEnginePage or QWebEngineView instance inside it.

      So, lets give the web-page a few seconds time to run the JavaScript code and
      give us the result. Otherwise, we can delete the web-page to incentivise
      QWebEnginePage to run the JS for us and return the result.
      */

    QTimer *timer = new QTimer(this);
    timer->setInterval(500);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, webPage, &QWebEnginePage::deleteLater);
    timer->start();

    /**
      By the time the web-page deletes itself, if we have not received the result
      then we can simply emit title and webpage attribus.
      */
    connect(webPage, &QWebEnginePage::destroyed, [this]() {
        this->useAttributes(m_defaultAttribs);
    });
}

void FetchOpenGraphAttributes::useAttributes(const QJsonObject &attribs)
{
    if(m_attribsFetched)
        return;

    m_attribsFetched = true;
    emit attributesFetched(attribs);

    qDebug() << "PA: " << attribs;

    if(m_autoDelete)
        GarbageCollector::instance()->add(this);
}

