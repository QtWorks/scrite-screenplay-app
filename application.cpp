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

#include "application.h"
#include "undoredo.h"
#include "autoupdate.h"
#include "simpletimer.h"

#include <QDir>
#include <QUuid>
#include <QtDebug>
#include <QWindow>
#include <QPointer>
#include <QProcess>
#include <QSettings>
#include <QFileInfo>
#include <QKeyEvent>
#include <QDateTime>
#include <QMetaEnum>
#include <QQuickItem>
#include <QJsonArray>
#include <QMessageBox>
#include <QJsonObject>
#include <QColorDialog>
#include <QNetworkReply>
#include <QFontDatabase>
#include <QStandardPaths>

#define ENABLE_SCRIPT_HOTKEY

bool QtApplicationEventNotificationCallback(void **cbdata);

Application *Application::instance()
{
    return qobject_cast<Application*>(qApp);
}

Application::Application(int &argc, char **argv, const QVersionNumber &version)
    : QtApplicationClass(argc, argv),
      m_versionNumber(version)
{
    connect(m_undoGroup, &QUndoGroup::canUndoChanged, this, &Application::canUndoChanged);
    connect(m_undoGroup, &QUndoGroup::canRedoChanged, this, &Application::canRedoChanged);
    connect(m_undoGroup, &QUndoGroup::undoTextChanged, this, &Application::undoTextChanged);
    connect(m_undoGroup, &QUndoGroup::redoTextChanged, this, &Application::redoTextChanged);
    connect(this, &QGuiApplication::fontChanged, this, &Application::applicationFontChanged);

    this->setWindowIcon( QIcon(":/images/appicon.png") );
    this->setBaseWindowTitle(Application::applicationName() + " " + Application::applicationVersion());

    const QString settingsFile = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absoluteFilePath("settings.ini");
    m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
    this->installationId();
    this->installationTimestamp();
    m_settings->setValue("Installation/launchCount", this->launchCounter()+1);

#ifndef QT_NO_DEBUG
    QInternal::registerCallback(QInternal::EventNotifyCallback, QtApplicationEventNotificationCallback);
#endif

    TransliterationEngine::instance(this);
}

Application::~Application()
{
#ifndef QT_NO_DEBUG
    QInternal::unregisterCallback(QInternal::EventNotifyCallback, QtApplicationEventNotificationCallback);
#endif
}

QString Application::installationId() const
{
    QString clientID = m_settings->value("Installation/ClientID").toString();
    if(clientID.isEmpty())
    {
        clientID = QUuid::createUuid().toString();
        m_settings->setValue("Installation/ClientID", clientID);
    }

    return clientID;
}

QDateTime Application::installationTimestamp() const
{
    QString installTimestampStr = m_settings->value("Installation/timestamp").toString();
    QDateTime installTimestamp = QDateTime::fromString(installTimestampStr);
    if(installTimestampStr.isEmpty() || !installTimestamp.isValid())
    {
        installTimestamp = QDateTime::currentDateTime();
        installTimestampStr = installTimestamp.toString();
        m_settings->setValue("Installation/timestamp", installTimestampStr);
    }

    return installTimestamp;
}

int Application::launchCounter() const
{
    return m_settings->value("Installation/launchCount", 0).toInt();
}

#ifdef Q_OS_MAC
Application::Platform Application::platform() const
{
    return Application::MacOS;
}
#else
#ifdef Q_OS_WIN
Application::Platform Application::platform() const
{
    return Application::WindowsDesktop;
}
#else
Application::Platform Application::platform() const
{
    return Application::LinuxDesktop;
}
#endif
#endif

QString Application::controlKey() const
{
    return this->platform() == Application::MacOS ? "⌘" : "Ctrl";
}

QString Application::altKey() const
{
    return this->platform() == Application::MacOS ? "⌥" : "Alt";
}

QString Application::polishShortcutTextForDisplay(const QString &text) const
{
    QString text2 = text.trimmed();
    text2.replace("Ctrl", this->controlKey(), Qt::CaseInsensitive);
    text2.replace("Alt", this->altKey(), Qt::CaseInsensitive);
    return text2;
}

void Application::setBaseWindowTitle(const QString &val)
{
    if(m_baseWindowTitle == val)
        return;

    m_baseWindowTitle = val;
    emit baseWindowTitleChanged();
}

QString Application::typeName(QObject *object) const
{
    if(object == nullptr)
        return QString();

    return QString::fromLatin1(object->metaObject()->className());
}

bool Application::verifyType(QObject *object, const QString &name) const
{
    return object && object->inherits(qPrintable(name));
}

UndoStack *Application::findUndoStack(const QString &objectName) const
{
    const QList<QUndoStack*> stacks = m_undoGroup->stacks();
    Q_FOREACH(QUndoStack *stack, stacks)
    {
        if(stack->objectName() == objectName)
        {
            UndoStack *ret = qobject_cast<UndoStack*>(stack);
            return ret;
        }
    }

    return nullptr;
}

QJsonObject Application::systemFontInfo() const
{
    QFontDatabase fontdb;

    QJsonObject ret;
    ret.insert("families", QJsonArray::fromStringList(fontdb.families()));

    QJsonArray sizes;
    QList<int> stdSizes = fontdb.standardSizes();
    Q_FOREACH(int stdSize, stdSizes)
        sizes.append( QJsonValue(stdSize) );
    ret.insert("standardSizes", sizes);

    return ret;
}

QColor Application::pickColor(const QColor &initial) const
{
    QColorDialog::ColorDialogOptions options =
            QColorDialog::ShowAlphaChannel|QColorDialog::DontUseNativeDialog;
    return QColorDialog::getColor(initial, nullptr, "Select Color", options);
}

QRectF Application::textBoundingRect(const QString &text, const QFont &font) const
{
    return QFontMetricsF(font).boundingRect(text);
}

void Application::revealFileOnDesktop(const QString &pathIn)
{
    m_errorReport->clear();

    // The implementation of this function is inspired from QtCreator's
    // implementation of FileUtils::showInGraphicalShell() method
    const QFileInfo fileInfo(pathIn);

    // Mac, Windows support folder or file.
    if (this->platform() == WindowsDesktop)
    {
        const QString explorer = QStandardPaths::findExecutable("explorer.exe");
        if (explorer.isEmpty())
        {
            m_errorReport->setErrorMessage("Could not find explorer.exe in path to launch Windows Explorer.");
            return;
        }

        QStringList param;
        if (!fileInfo.isDir())
            param += QLatin1String("/select,");
        param += QDir::toNativeSeparators(fileInfo.canonicalFilePath());
        QProcess::startDetached(explorer, param);
    }
    else if (this->platform() == MacOS)
    {
        QStringList scriptArgs;
        scriptArgs << QLatin1String("-e")
                   << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                                         .arg(fileInfo.canonicalFilePath());
        QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
        scriptArgs.clear();
        scriptArgs << QLatin1String("-e")
                   << QLatin1String("tell application \"Finder\" to activate");
        QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
    }
    else
    {
#if 0 // TODO
        // we cannot select a file here, because no file browser really supports it...
        const QString folder = fileInfo.isDir() ? fileInfo.absoluteFilePath() : fileInfo.filePath();
        const QString app = UnixUtils::fileBrowser(ICore::settings());
        QProcess browserProc;
        const QString browserArgs = UnixUtils::substituteFileBrowserParameters(app, folder);
        bool success = browserProc.startDetached(browserArgs);
        const QString error = QString::fromLocal8Bit(browserProc.readAllStandardError());
        success = success && error.isEmpty();
        if (!success)
            showGraphicalShellError(parent, app, error);
#endif
    }
}

QJsonArray enumerationModel(const QMetaObject *metaObject, const QString &enumName)
{
    QJsonArray ret;

    if( metaObject == nullptr || enumName.isEmpty() )
        return ret;

    const int enumIndex = metaObject->indexOfEnumerator( qPrintable(enumName) );
    if( enumIndex < 0 )
        return ret;

    const QMetaEnum enumInfo = metaObject->enumerator(enumIndex);
    if( !enumInfo.isValid() )
        return ret;

    for(int i=0; i<enumInfo.keyCount(); i++)
    {
        QJsonObject item;
        item.insert("key", QString::fromLatin1(enumInfo.key(i)));
        item.insert("value", enumInfo.value(i));
        ret.append(item);
    }

    return ret;
}

QJsonArray Application::enumerationModel(QObject *object, const QString &enumName) const
{
    const QMetaObject *mo = object ? object->metaObject() : nullptr;
    return ::enumerationModel(mo, enumName);
}

QJsonArray Application::enumerationModelForType(const QString &typeName, const QString &enumName) const
{
    const int typeId = QMetaType::type(qPrintable(typeName+"*"));
    const QMetaObject *mo = typeId == QMetaType::UnknownType ? nullptr : QMetaType::metaObjectForType(typeId);
    return ::enumerationModel(mo, enumName);
}

QJsonObject Application::fileInfo(const QString &path) const
{
    QFileInfo fi(path);
    QJsonObject ret;
    ret.insert("exists", fi.exists());
    if(!fi.exists())
        return ret;

    ret.insert("baseName", fi.baseName());
    ret.insert("absoluteFilePath", fi.absoluteFilePath());
    ret.insert("absolutePath", fi.absolutePath());
    ret.insert("suffix", fi.suffix());
    ret.insert("fileName", fi.fileName());
    return ret;
}

QString Application::settingsFilePath() const
{
    return m_settings->fileName();
}

QPointF Application::cursorPosition() const
{
    return QCursor::pos();
}

QPointF Application::mapGlobalPositionToItem(QQuickItem *item, const QPointF &pos) const
{
    if(item == nullptr)
        return pos;

    return item->mapFromGlobal(pos);
}

class ExecLater : public QObject
{
public:
    ExecLater(int howMuchLater, const QJSValue &function, const QJSValueList &arg, QObject *parent=nullptr);
    ~ExecLater();

    void timerEvent(QTimerEvent *event);

private:
    SimpleTimer m_timer;
    QJSValue m_function;
    QJSValueList m_arguments;
};

ExecLater::ExecLater(int howMuchLater, const QJSValue &function, const QJSValueList &args, QObject *parent)
    : QObject(parent), m_timer("ExecLater.m_timer"), m_function(function), m_arguments(args)
{
    howMuchLater = qBound(0, howMuchLater, 60*60*1000);
    m_timer.start(howMuchLater, this);
}

ExecLater::~ExecLater()
{
    m_timer.stop();
}

void ExecLater::timerEvent(QTimerEvent *event)
{
    if(m_timer.timerId() == event->timerId())
    {
        m_timer.stop();
        if(m_function.isCallable())
            m_function.call(m_arguments);
        GarbageCollector::instance()->add(this);
    }
}

void Application::execLater(QObject *context, int howMuchLater, const QJSValue &function, const QJSValueList &args)
{
    QObject *parent = context ? context : this;
    new ExecLater(howMuchLater, function, args, parent);
}

AutoUpdate *Application::autoUpdate() const
{
    return AutoUpdate::instance();
}

QJsonObject Application::objectConfigurationFormInfo(const QObject *object, const QMetaObject *from=nullptr) const
{
    QJsonObject ret;
    if(object == nullptr)
            return ret;

    if(from == nullptr)
        from = object->metaObject();

    const QMetaObject *mo = object->metaObject();
    auto queryClassInfo = [mo](const char *key) {
        const int ciIndex = mo->indexOfClassInfo(key);
        if(ciIndex < 0)
            return QString();
        const QMetaClassInfo ci = mo->classInfo(ciIndex);
        return QString::fromLatin1(ci.value());
    };

    auto queryPropertyInfo = [queryClassInfo](const QMetaProperty &prop, const char *key) {
        const QString ciKey = QString::fromLatin1(prop.name()) + "_" + QString::fromLatin1(key);
        return queryClassInfo(qPrintable(ciKey));
    };

    ret.insert("title", queryClassInfo("Title"));

    QJsonArray fields;
    for(int i=from->propertyOffset(); i<mo->propertyCount(); i++)
    {
        const QMetaProperty prop = mo->property(i);
        if(!prop.isWritable() || !prop.isStored())
            continue;

        QJsonObject field;
        field.insert("name", QString::fromLatin1(prop.name()));
        field.insert("label", queryPropertyInfo(prop, "FieldLabel"));
        field.insert("note", queryPropertyInfo(prop, "FieldNote"));
        field.insert("editor", queryPropertyInfo(prop, "FieldEditor"));
        field.insert("min", queryPropertyInfo(prop, "FieldMinValue"));
        field.insert("max", queryPropertyInfo(prop, "FieldMaxValue"));
        field.insert("ideal", queryPropertyInfo(prop, "FieldDefaultValue"));

        const QString fieldEnum = queryPropertyInfo(prop, "FieldEnum");
        if( !fieldEnum.isEmpty() )
        {
            const int enumIndex = mo->indexOfEnumerator(qPrintable(fieldEnum));
            const QMetaEnum enumerator = mo->enumerator(enumIndex);

            QJsonArray choices;
            for(int j=0; j<enumerator.keyCount(); j++)
            {
                QJsonObject choice;
                choice.insert("key", QString::fromLatin1(enumerator.key(j)));
                choice.insert("value", enumerator.value(j));

                const QByteArray ciKey = QByteArray(enumerator.name()) + "_" + QByteArray(enumerator.key(j));
                const QString text = queryClassInfo(ciKey);
                if(!text.isEmpty())
                    choice.insert("key", text);

                choices.append(choice);
            }

            field.insert("choices", choices);
        }

        fields.append(field);
    }

    ret.insert("fields", fields);

    return ret;
}

bool QtApplicationEventNotificationCallback(void **cbdata)
{
#ifndef QT_NO_DEBUG
    QObject *object = reinterpret_cast<QObject*>(cbdata[0]);
    QEvent *event = reinterpret_cast<QEvent*>(cbdata[1]);
    bool *result = reinterpret_cast<bool*>(cbdata[2]);

    const bool ret = Application::instance()->notifyInternal(object, event);

    if(result)
        *result |= ret;

    return ret;
#else
    Q_UNUSED(cbdata)
    return false;
#endif
}

bool Application::notify(QObject *object, QEvent *event)
{
    // Note that notifyInternal() will be called first before we get here.
    if(event->type() == QEvent::DeferredDelete)
        return QtApplicationClass::notify(object, event);

    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        if(ke->modifiers() & Qt::ControlModifier && ke->key() == Qt::Key_M)
        {
            emit minimizeWindowRequest();
            return true;
        }

        if(ke->modifiers() == Qt::ControlModifier && ke->key() == Qt::Key_Z)
        {
            m_undoGroup->undo();
            return true;
        }

        if( (ke->modifiers() == Qt::ControlModifier && ke->key() == Qt::Key_Y)
#ifdef Q_OS_MAC
           || (ke->modifiers()&Qt::ControlModifier && ke->modifiers()&Qt::ShiftModifier && ke->key() == Qt::Key_Z)
#endif
                )
        {
            m_undoGroup->redo();
            return true;
        }

        if( ke->modifiers()&Qt::ControlModifier && ke->modifiers()&Qt::ShiftModifier && ke->key() == Qt::Key_T )
        {
            if(this->loadScript())
                return true;
        }
    }

    const bool ret = QtApplicationClass::notify(object, event);

    // The only reason we reimplement the notify() method is because we sometimes want to
    // handle an event AFTER it is handled by the target object.

    if(event->type() == QEvent::ChildAdded)
    {
        QChildEvent *childEvent = reinterpret_cast<QChildEvent*>(event);
        QObject *childObject = childEvent->child();

        if(!childObject->isWidgetType() && !childObject->isWindowType())
        {
            /**
             * For whatever reason, ParentChange event is only sent
             * if the child is a widget or window or declarative-item.
             * I was not aware of this up until now. Classes like
             * StructureElement, SceneElement etc assume that ParentChange
             * event will be sent when they are inserted into the document
             * object tree, so that they can evaluate a pointer to the
             * parent object in the tree. Since these classes are subclassed
             * from QObject, we will need the following lines to explicitly
             * despatch ParentChange events.
             */
            QEvent parentChangeEvent(QEvent::ParentChange);
            QtApplicationClass::notify(childObject, &parentChangeEvent);
        }
    }

    return ret;
}

bool Application::notifyInternal(QObject *object, QEvent *event)
{
#ifndef QT_NO_DEBUG
    static QMap<QObject*,QString> objectNameMap;
    auto evaluateObjectName = [](QObject *object, QMap<QObject*,QString> &from) {
        QString objectName = from.value(object);
        if(objectName.isEmpty()) {
            QQuickItem *item = qobject_cast<QQuickItem*>(object);
            QObject* parent = item && item->parentItem() ? item->parentItem() : object->parent();
            QString parentName = parent ? from.value(parent) : "No Parent";
            if(parentName.isEmpty()) {
                parentName = QString("%1 [%2] (%3)")
                                    .arg(parent->metaObject()->className())
                                    .arg((unsigned long)((void*)parent),0,16)
                                    .arg(parent->objectName());
            }
            objectName = QString("%1 [%2] (%3) under %4")
                    .arg(object->metaObject()->className())
                    .arg((unsigned long)((void*)object),0,16)
                    .arg(object->objectName())
                    .arg(parentName);
            from[object] = objectName;
        }
        return objectName;
    };

    if(event->type() == QEvent::DeferredDelete)
    {
        const QString objectName = evaluateObjectName(object, objectNameMap);
        qDebug() << "DeferredDelete: " << objectName;
    }
    else if(event->type() == QEvent::Timer)
    {
        const QString objectName = evaluateObjectName(object, objectNameMap);
        QTimerEvent *te = static_cast<QTimerEvent*>(event);
        SimpleTimer *timer = SimpleTimer::get(te->timerId());
        qDebug() << "TimerEventDespatch: " << te->timerId() << " on " << objectName << " is " << (timer ? qPrintable(timer->name()) : "Qt Timer.");
    }
#else
    Q_UNUSED(object)
    Q_UNUSED(event)
#endif

    return false;
}

QColor Application::pickStandardColor(int counter) const
{
    const QVector<QColor> colors = this->standardColors();
    if(colors.isEmpty())
        return QColor("white");

    QColor ret = colors.at( counter%colors.size() );
    return ret;
}

QColor Application::textColorFor(const QColor &bgColor) const
{
    // https://stackoverflow.com/questions/1855884/determine-font-color-based-on-background-color/1855903#1855903
    const qreal luma = ((0.299 * bgColor.redF()) + (0.587 * bgColor.greenF()) + (0.114 * bgColor.blueF()));
    return luma > 0.5 ? Qt::black : Qt::white;
}

QRectF Application::boundingRect(const QString &text, const QFont &font) const
{
    const QFontMetricsF fm(font);
    return fm.boundingRect(text);
}

QRectF Application::intersectedRectangle(const QRectF &of, const QRectF &with) const
{
    return of.intersected(with);
}

bool Application::doRectanglesIntersect(const QRectF &r1, const QRectF &r2) const
{
    return r1.intersects(r2);
}

QSizeF Application::scaledSize(const QSizeF &of, const QSizeF &into) const
{
    return of.scaled(into, Qt::KeepAspectRatio);
}

QRectF Application::uniteRectangles(const QRectF &r1, const QRectF &r2) const
{
    return r1.united(r2);
}

QRectF Application::adjustRectangle(const QRectF &rect, qreal left, qreal top, qreal right, qreal bottom) const
{
    return rect.adjusted(left, top, right, bottom);
}

bool Application::isRectangleInRectangle(const QRectF &bigRect, const QRectF &smallRect) const
{
    return bigRect.contains(smallRect);
}

QPointF Application::translationRequiredToBringRectangleInRectangle(const QRectF &bigRect, const QRectF &smallRect) const
{
    QPointF ret(0, 0);

    if(!bigRect.contains(smallRect))
    {
        if(smallRect.left() < bigRect.left())
            ret.setX( bigRect.left()-smallRect.left() );
        else if(smallRect.right() > bigRect.right())
            ret.setX( -(smallRect.right()-bigRect.right()) );

        if(smallRect.top() < bigRect.top())
            ret.setY( bigRect.top()-smallRect.top() );
        else if(smallRect.bottom() > bigRect.bottom())
            ret.setY( -(smallRect.bottom()-bigRect.bottom()) );
    }

    return ret;
}

QString Application::fileContents(const QString &fileName) const
{
    QFile file(fileName);
    if( !file.open(QFile::ReadOnly) )
        return QString();

    return QString::fromLatin1(file.readAll());
}

QScreen *Application::windowScreen(QObject *window) const
{
    QWindow *qwindow = qobject_cast<QWindow*>(window);
    if(qwindow)
        return qwindow->screen();

    QWidget *qwidget = qobject_cast<QWidget*>(window);
    if(qwidget)
        return qwidget->window()->windowHandle()->screen();

    return nullptr;
}

void Application::initializeStandardColors(QQmlEngine *)
{
    if(!m_standardColors.isEmpty())
        return;

    const QVector<QColor> colors = this->standardColors();
    for(int i=0; i<colors.size(); i++)
        m_standardColors << QVariant::fromValue<QColor>(colors.at(i));

    emit standardColorsChanged();
}

QVector<QColor> Application::standardColors(const QVersionNumber &version)
{
    // Up-until version 0.2.17 Beta
    if( !version.isNull() && version <= QVersionNumber(0,2,17) )
        return QVector<QColor>() <<
            QColor("blue") << QColor("magenta") << QColor("darkgreen") <<
            QColor("purple") << QColor("yellow") << QColor("orange") <<
            QColor("red") << QColor("brown") << QColor("gray") << QColor("white");

    // New set of colors
    return QVector<QColor>() <<
        QColor("#2196f3") << QColor("#e91e63") << QColor("#009688") <<
        QColor("#9c27b0") << QColor("#ffeb3b") << QColor("#ff9800") <<
        QColor("#f44336") << QColor("#795548") << QColor("#9e9e9e") <<
        QColor("#fafafa") << QColor("#3f51b5") << QColor("#cddc39");
}

#ifdef ENABLE_SCRIPT_HOTKEY

#include <QJSEngine>
#include <QFileDialog>
#include "scritedocument.h"

bool Application::loadScript()
{
    QMessageBox::StandardButton answer = QMessageBox::question(nullptr, "Warning", "Executing scripts on a scrite project is an experimental feature. Are you sure you want to use it?", QMessageBox::Yes|QMessageBox::No);
    if(answer == QMessageBox::No)
        return true;

    ScriteDocument *document = ScriteDocument::instance();

    QString scriptPath = QDir::homePath();
    if( !document->fileName().isEmpty() )
    {
        QFileInfo fi(document->fileName());
        scriptPath = fi.absolutePath();
    }

    const QString caption("Select a JavaScript file to load");
    const QString filter("JavaScript File (*.js)");
    const QString scriptFile = QFileDialog::getOpenFileName(nullptr, caption, scriptPath, filter);
    if(scriptFile.isEmpty())
        return true;

    auto loadProgram = [](const QString &fileName) {
        QFile file(fileName);
        if(!file.open(QFile::ReadOnly))
            return QString();
        return QString::fromLatin1(file.readAll());
    };
    const QString program = loadProgram(scriptFile);
    if(program.isEmpty())
    {
        QMessageBox::information(nullptr, "Script", "No code was found in the selected file.");
        return true;
    }

    QJSEngine jsEngine;
    QJSValue globalObject = jsEngine.globalObject();
    globalObject.setProperty("document", jsEngine.newQObject(document));
    const QJSValue result = jsEngine.evaluate(program, scriptFile);
    if(result.isError())
    {
        const QString msg = "Uncaught exception at line " +
                result.property("lineNumber").toString() + ": " +
                result.toString();
        QMessageBox::warning(nullptr, "Script", msg);
    }

    return true;
}

#else
bool Application::loadScript() { return false; }
#endif
