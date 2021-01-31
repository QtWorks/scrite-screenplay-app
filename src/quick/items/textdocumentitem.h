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

#ifndef TEXTDOCUMENTITEM_H
#define TEXTDOCUMENTITEM_H

#include <QQuickItem>
#include <QTextDocument>

class TextDocumentViewportItem;
class TextDocumentItem : public QQuickItem
{
    Q_OBJECT

public:
    TextDocumentItem(QQuickItem *parent=nullptr);
    ~TextDocumentItem();

    Q_PROPERTY(QTextDocument* document READ document WRITE setDocument NOTIFY documentChanged)
    void setDocument(QTextDocument* val);
    QTextDocument* document() const { return m_document; }
    Q_SIGNAL void documentChanged();

    Q_PROPERTY(qreal documentScale READ documentScale WRITE setDocumentScale NOTIFY documentScaleChanged)
    void setDocumentScale(qreal val);
    qreal documentScale() const { return m_documentScale; }
    Q_SIGNAL void documentScaleChanged();

    Q_PROPERTY(QQuickItem* flickable READ flickable WRITE setFlickable NOTIFY flickableChanged)
    void setFlickable(QQuickItem* val);
    QQuickItem* flickable() const { return m_flickable; }
    Q_SIGNAL void flickableChanged();

private:
    void updateViewport();
    void onDocumentChanged();

private:
    qreal m_documentScale = 1.0;
    QQuickItem* m_flickable = nullptr;
    QTextDocument* m_document = nullptr;
    QTimer *m_viewportUpdateHandler = nullptr;
    QTimer *m_documentChangeHandler = nullptr;
    TextDocumentViewportItem *m_viewportItem = nullptr;
};

#endif // TEXTDOCUMENTITEM_H
