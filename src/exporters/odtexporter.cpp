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

#include "odtexporter.h"

#include <QFileInfo>
#include <QPdfWriter>
#include <QTextDocumentWriter>

OdtExporter::OdtExporter(QObject *parent)
            :AbstractTextDocumentExporter(parent)
{

}

OdtExporter::~OdtExporter()
{

}

void OdtExporter::setIncludeSceneNumbers(bool val)
{
    if(m_includeSceneNumbers == val)
        return;

    m_includeSceneNumbers = val;
    emit includeSceneNumbersChanged();
}

bool OdtExporter::doExport(QIODevice *device)
{
    const qreal pageWidth = 0; // pdfWriter.width();
    QTextDocument textDocument;
    this->AbstractTextDocumentExporter::generate(&textDocument, pageWidth);

    QTextDocumentWriter writer;
    writer.setFormat("ODF");
    writer.setDevice(device);
    writer.write(&textDocument);

    return true;
}

QString OdtExporter::polishFileName(const QString &fileName) const
{
    QFileInfo fi(fileName);
    if( fi.suffix().toLower() != "odt" )
        return fileName + ".odt";
    return fileName;
}
