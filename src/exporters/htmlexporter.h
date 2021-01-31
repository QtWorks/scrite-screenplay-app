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

#ifndef HTMLEXPORTER_H
#define HTMLEXPORTER_H

#include "abstractexporter.h"

class HtmlExporter : public AbstractExporter
{
    Q_OBJECT
    Q_CLASSINFO("Format", "Screenplay/HTML")
    Q_CLASSINFO("NameFilters", "HTML (*.html)")

public:
    Q_INVOKABLE HtmlExporter(QObject *parent=nullptr);
    ~HtmlExporter();

    Q_CLASSINFO("includeSceneNumbers_FieldLabel", "Include scene numbers in the generated HTML.")
    Q_CLASSINFO("includeSceneNumbers_FieldEditor", "CheckBox")
    Q_PROPERTY(bool includeSceneNumbers READ isIncludeSceneNumbers WRITE setIncludeSceneNumbers NOTIFY includeSceneNumbersChanged)
    void setIncludeSceneNumbers(bool val);
    bool isIncludeSceneNumbers() const { return m_includeSceneNumbers; }
    Q_SIGNAL void includeSceneNumbersChanged();

    Q_CLASSINFO("exportWithSceneColors_FieldLabel", "Export with scene colors")
    Q_CLASSINFO("exportWithSceneColors_FieldEditor", "CheckBox")
    Q_PROPERTY(bool exportWithSceneColors READ isExportWithSceneColors WRITE setExportWithSceneColors NOTIFY exportWithSceneColorsChanged)
    void setExportWithSceneColors(bool val);
    bool isExportWithSceneColors() const { return m_exportWithSceneColors; }
    Q_SIGNAL void exportWithSceneColorsChanged();

    bool canBundleFonts() const { return true; }
    bool requiresConfiguration() const { return true; }

protected:
    bool doExport(QIODevice *device); // AbstractExporter interface
    QString polishFileName(const QString &fileName) const; // AbstractDeviceIO interface

private:
    bool m_includeSceneNumbers = false;
    bool m_exportWithSceneColors = false;
};

#endif // HTMLEXPORTER_H
