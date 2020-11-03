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

#include "spellcheckservice.h"
#include "timeprofiler.h"
#include "scritedocument.h"
#include "garbagecollector.h"

#include <QFuture>
#include <QJsonObject>
#include <QTimerEvent>
#include <QFutureWatcher>
#include <QThreadStorage>
#include <QtConcurrentRun>
#include <QRandomGenerator>
#include <QCoreApplication>

#include "3rdparty/sonnet/sonnet/src/core/speller.h"
#include "3rdparty/sonnet/sonnet/src/core/loader_p.h"
#include "3rdparty/sonnet/sonnet/src/core/textbreaks_p.h"
#include "3rdparty/sonnet/sonnet/src/core/guesslanguage.h"

class SpellCheckServiceResult
{
    char padding[4];

public:
    SpellCheckServiceResult() { padding[0] = 0; }

    int timestamp = -1;
    QString text;
    QList<TextFragment> misspelledFragments;
};

class EnglishLanguageSpeller : public Sonnet::Speller
{
public:
#ifdef Q_OS_MAC
    EnglishLanguageSpeller() : Sonnet::Speller("en") { }
#else
#ifdef Q_OS_WIN
    EnglishLanguageSpeller() : Sonnet::Speller() { }
#else
    EnglishLanguageSpeller() : Sonnet::Speller("en_US") { }
#endif
#endif
    ~EnglishLanguageSpeller() { }
};

struct SpellCheckServiceRequest
{
    QString text;
    int timestamp;
    QStringList characterNames;
    QStringList ignoreList;
};

void InitializeSpellCheckThread()
{
    Sonnet::Loader::openLoader();
}

SpellCheckServiceResult CheckSpellings(const SpellCheckServiceRequest &request)
{
    SpellCheckServiceResult result;
    result.timestamp = request.timestamp;
    result.text = request.text;

    if(request.text.isEmpty())
        return result; // Should never happen

    /*
     * Much of the code here is inspired from Sonnet::Highlighter::highlightBlock()
     * Obviously their implementation is 'smarter' because their implementation works
     * directly on the view, our's works on the model.
     *
     * Sonnet::Highlighter is intended to be used on a QTextEdit or QPlainTextEdit
     * both of which are widgets (which means views). The highlighter directly works
     * on QTextBlocks within a QTextDocument, which is great.
     *
     * We cannot do that in Scrite because we open a TextArea for each scene separately.
     * And TextAreas get created and destroyed frequently depending on the scroll position
     * of contentView in ScreenplayEditor. This means that each time a scene scrolls into
     * visibility, it will have to reinitialize the entire underlying data structure related
     * spell-check. That is an expensive thing to do.
     *
     * In Scrite, we simply associate a SpellCheck instance with each SceneElement and reuse
     * its results across all views hooked to the Scene. Additionally, we use SpellCheck on
     * Note and StructureElement also. This fits into the whole model-view thinking that
     * QML apps are required to leverage.
     *
     * There is obviously room for improvement in this implementation. We could cache
     * spell-check done in the previous round and only check for delta changes in here.
     * But for now, we simply take a brute-force approach. But this function could be a
     * good place for us to accept community contribution.
     */

    const Sonnet::TextBreaks::Positions wordPositions = Sonnet::TextBreaks::wordBreaks(request.text);
    if(wordPositions.isEmpty() || Sonnet::Loader::openLoader() == nullptr)
        return result;

    EnglishLanguageSpeller speller;
    Q_FOREACH(Sonnet::TextBreaks::Position wordPosition, wordPositions)
    {
        const QString word = request.text.mid(wordPosition.start, wordPosition.length);
        if(word.isEmpty())
            continue; // not sure why this would happen, but just keeping safe.

#ifndef Q_OS_MAC
        // We have to do this on Windows and Linux, otherwise all non-English words will
        // be flagged as spelling mistakes. What would be ideal is to check if the entire
        // word only has non-latin letters, but this is good enough.
        if(word.at(0).script() != QChar::Script_Latin)
            continue;
#endif

        if(Sonnet::Loader::openLoader() == nullptr)
        {
            result.misspelledFragments.clear();
            break;
        }

        const bool misspelled = speller.isMisspelled(word);
        if(misspelled)
        {
            if(request.ignoreList.contains(word))
                continue;

            if(request.characterNames.contains(word, Qt::CaseInsensitive))
                continue;

            if(word.endsWith("\'s", Qt::CaseInsensitive))
            {
                if(request.characterNames.contains(word.leftRef(word.length()-2), Qt::CaseInsensitive))
                    continue;
            }

            TextFragment fragment(wordPosition.start, wordPosition.length, speller.suggest(word));
            if(fragment.isValid())
                result.misspelledFragments << fragment;
        }
    }

    return result;
}

bool AddToDictionary(const QString &word)
{
    /**
     * It is assumed that word contains a single word. We won't bother checking for that.
     */
    EnglishLanguageSpeller speller;
    return speller.addToPersonal(word);
}

QStringList GetSpellingSuggestions(const QString &word)
{
    /**
     * It is assumed that word contains a single word. We won't bother checking for that.
     */
    EnglishLanguageSpeller speller;
    return speller.suggest(word);
}

static QThreadPool &SpellCheckServiceThreadPool()
{
    /**
     * We make use of QtConcurrent::run() method to schedule the following methods on a
     * background thread, so that they dont block the UI.
     * - InitializeSpellCheckThread
     * - CheckSpellings
     * - AddToDictionary
     * - GetSpellingSuggestions
     *
     * We know for a fact that CheckSpellings() does indeed block the UI thread because
     * spelling lookup is a slow process. The default behaviour of SpellCheckService is
     * to looup spellings asynchronously and in the background. So, scheduling these
     * functions in a background thread works for us.
     *
     * Moreover we need exactly one thread for this purpose. While a QThreadPool can
     * allocate multiple threads for us, we need only one. And one that thread is created
     * it should NEVER EVER terminate until the program finishes.
     */
    static bool initialized = false;
    static QThreadPool threadPool;
    if(!initialized)
    {
        threadPool.setExpiryTimeout(-1);
        threadPool.setMaxThreadCount(1);
        QFuture<void> future = QtConcurrent::run(&threadPool, InitializeSpellCheckThread);
        future.waitForFinished();
        initialized = true;
    }

    return threadPool;
}

SpellCheckService::SpellCheckService(QObject *parent)
    : QObject(parent),
      m_textTracker(&m_textModifiable)
{

}

SpellCheckService::~SpellCheckService()
{

}

void SpellCheckService::setText(const QString &val)
{
    if(m_text == val)
        return;

    m_text = val;
    m_textModifiable.markAsModified();

    emit textChanged();

    this->doUpdate();
}

void SpellCheckService::setMethod(SpellCheckService::Method val)
{
    if(m_method == val)
        return;

    m_method = val;
    emit methodChanged();
}

void SpellCheckService::setAsynchronous(bool val)
{
    if(m_asynchronous == val)
        return;

    m_asynchronous = val;
    emit asynchronousChanged();
}

void SpellCheckService::scheduleUpdate()
{
    m_textModifiable.markAsModified();
    m_updateTimer.start(500, this);
}

void SpellCheckService::update()
{
    if(!m_textTracker.isModified())
        return;

    this->setMisspelledFragments(QList<TextFragment>());

    if(m_text.isEmpty())
        return;

    emit started();

    SpellCheckServiceRequest request;
    request.text = m_text;
    request.timestamp = m_textModifiable.modificationTime();
    request.characterNames = ScriteDocument::instance()->structure()->characterNames();
    request.ignoreList = ScriteDocument::instance()->spellCheckIgnoreList();

    request.characterNames << QStringLiteral("Rajkumar");

    QFutureWatcher<SpellCheckServiceResult> *watcher = new QFutureWatcher<SpellCheckServiceResult>(this);
    connect(watcher, SIGNAL(finished()), this, SLOT(spellCheckComplete()), Qt::QueuedConnection);

    QThreadPool &threadPool = SpellCheckServiceThreadPool();
    QFuture<SpellCheckServiceResult> future = QtConcurrent::run(&threadPool, CheckSpellings, request);
    watcher->setFuture(future);
}

QStringList SpellCheckService::suggestions(const QString &word)
{
    QThreadPool &threadPool = SpellCheckServiceThreadPool();
    QFuture<QStringList> future = QtConcurrent::run(&threadPool, GetSpellingSuggestions, word);
    future.waitForFinished();
    return future.result();
}

bool SpellCheckService::addToDictionary(const QString &word)
{
    QThreadPool &threadPool = SpellCheckServiceThreadPool();
    QFuture<bool> future = QtConcurrent::run(&threadPool, AddToDictionary, word);
    future.waitForFinished();
    return future.result();
}

void SpellCheckService::classBegin()
{

}

void SpellCheckService::componentComplete()
{
    this->doUpdate();
}

void SpellCheckService::setMisspelledFragments(const QList<TextFragment> &val)
{
    if(m_misspelledFragments == val)
        return;

    m_misspelledFragments = val;

    QJsonArray json;
    Q_FOREACH(TextFragment textFrag, m_misspelledFragments)
    {
        QJsonObject item;
        item.insert("start", textFrag.start());
        item.insert("length", textFrag.length());
        item.insert("isValid", textFrag.isValid());
        item.insert("end", textFrag.end());
        item.insert("text", m_text.mid(textFrag.start(), textFrag.length()));
        json.append(item);
    }
    m_misspelledFragmentsJson = json;

    this->markAsModified();

    emit misspelledFragmentsChanged();
}

void SpellCheckService::doUpdate()
{
    if(m_method == Automatic)
    {
        if(m_asynchronous)
            m_updateTimer.start(500, this);
        else
            this->update();
    }
}

void SpellCheckService::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_updateTimer.timerId())
    {
        m_updateTimer.stop();
        this->update();
    }
}

void SpellCheckService::spellCheckComplete()
{
    QFutureWatcher<SpellCheckServiceResult> *watcher = dynamic_cast< QFutureWatcher<SpellCheckServiceResult> *>(this->sender());
    if(watcher == nullptr)
        return;

    GarbageCollector::instance()->add(watcher);

    const SpellCheckServiceResult result = watcher->result();
    if(m_textModifiable.isModified(result.timestamp))
        return;

    this->acceptResult(result);
}

void SpellCheckService::acceptResult(const SpellCheckServiceResult &result)
{
    this->setMisspelledFragments(result.misspelledFragments);
    emit finished();
}
