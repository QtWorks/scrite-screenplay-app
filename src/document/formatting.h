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

#ifndef FORMATTING_H
#define FORMATTING_H

#include "scene.h"
#include "modifiable.h"
#include "execlatertimer.h"
#include "transliteration.h"
#include "qobjectproperty.h"

#include <QScreen>
#include <QPageLayout>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QPagedPaintDevice>
#include <QSyntaxHighlighter>
#include <QQuickTextDocument>

class SpellCheckService;
class ScriteDocument;
class ScreenplayFormat;

class SceneElementFormat : public QObject, public Modifiable
{
    Q_OBJECT

public:
    ~SceneElementFormat();

    Q_PROPERTY(ScreenplayFormat* format READ format CONSTANT STORED false)
    ScreenplayFormat* format() const { return m_format; }

    Q_PROPERTY(SceneElement::Type elementType READ elementType CONSTANT)
    SceneElement::Type elementType() const { return m_elementType; }
    Q_SIGNAL void elementTypeChanged();

    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    void setFont(const QFont &val);
    QFont &fontRef() { return m_font; }
    QFont font() const { return m_font; }
    Q_SIGNAL void fontChanged();

    Q_PROPERTY(QFont font2 READ font2 NOTIFY font2Changed)
    QFont font2() const;
    Q_SIGNAL void font2Changed();

    Q_INVOKABLE void setFontFamily(const QString &val);
    Q_INVOKABLE void setFontBold(bool val);
    Q_INVOKABLE void setFontItalics(bool val);
    Q_INVOKABLE void setFontUnderline(bool val);
    Q_INVOKABLE void setFontPointSize(int val);
    Q_INVOKABLE void setFontCapitalization(QFont::Capitalization caps);

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    void setTextColor(const QColor &val);
    QColor textColor() const { return m_textColor; }
    Q_SIGNAL void textColorChanged();

    Q_PROPERTY(Qt::Alignment textAlignment READ textAlignment WRITE setTextAlignment NOTIFY textAlignmentChanged)
    void setTextAlignment(Qt::Alignment val);
    Qt::Alignment textAlignment() const { return m_textAlignment; }
    Q_SIGNAL void textAlignmentChanged();

    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    void setBackgroundColor(const QColor &val);
    QColor backgroundColor() const { return m_backgroundColor; }
    Q_SIGNAL void backgroundColorChanged();

    Q_PROPERTY(qreal lineHeight READ lineHeight WRITE setLineHeight NOTIFY lineHeightChanged)
    void setLineHeight(qreal val);
    qreal lineHeight() const { return m_lineHeight; }
    Q_SIGNAL void lineHeightChanged();

    Q_PROPERTY(qreal lineSpacingBefore READ lineSpacingBefore WRITE setLineSpacingBefore NOTIFY lineSpacingBeforeChanged STORED false)
    void setLineSpacingBefore(qreal val);
    qreal lineSpacingBefore() const { return m_lineSpacingBefore; }
    Q_SIGNAL void lineSpacingBeforeChanged();

    Q_PROPERTY(qreal leftMargin READ leftMargin WRITE setLeftMargin NOTIFY leftMarginChanged STORED false)
    void setLeftMargin(qreal val);
    qreal leftMargin() const { return m_leftMargin; }
    Q_SIGNAL void leftMarginChanged();

    Q_PROPERTY(qreal rightMargin READ rightMargin WRITE setRightMargin NOTIFY rightMarginChanged STORED false)
    void setRightMargin(qreal val);
    qreal rightMargin() const { return m_rightMargin; }
    Q_SIGNAL void rightMarginChanged();

    // Must be manually kept in sync with TransliterationEngine::Language
    enum DefaultLanguage
    {
        Default,
        English,
        Bengali,
        Gujarati,
        Hindi,
        Kannada,
        Malayalam,
        Marathi,
        Oriya,
        Punjabi,
        Sanskrit,
        Tamil,
        Telugu
    };
    Q_ENUM(DefaultLanguage)
    Q_PROPERTY(DefaultLanguage defaultLanguage READ defaultLanguage WRITE setDefaultLanguage NOTIFY defaultLanguageChanged)
    void setDefaultLanguage(DefaultLanguage val);
    DefaultLanguage defaultLanguage() const { return m_defaultLanguage; }
    Q_SIGNAL void defaultLanguageChanged();

    Q_PROPERTY(int defaultLanguageInt READ defaultLanguageInt WRITE setDefaultLanguageInt NOTIFY defaultLanguageChanged)
    int defaultLanguageInt() const { return int(m_defaultLanguage); }
    void setDefaultLanguageInt(int val) { this->setDefaultLanguage(DefaultLanguage(val)); }

    Q_INVOKABLE void activateDefaultLanguage();

    QTextBlockFormat createBlockFormat(const qreal *pageWidth=nullptr) const;
    QTextCharFormat createCharFormat(const qreal *pageWidth=nullptr) const;

    Q_SIGNAL void elementFormatChanged();

    enum Properties
    {
        FontFamily,
        FontSize,
        FontStyle,
        LineHeight,
        LineSpacingBefore,
        TextAndBackgroundColors
    };
    Q_ENUM(Properties)
    Q_INVOKABLE void applyToAll(Properties properties);

    void resetToDefaults();

private:
    friend class ScreenplayFormat;
    SceneElementFormat(SceneElement::Type type=SceneElement::Action, ScreenplayFormat *parent=nullptr);

private:
    QFont m_font;
    qreal m_lineHeight = 1.0;
    qreal m_leftMargin = 0;
    qreal m_rightMargin = 0;
    qreal m_lineSpacingBefore = 0;
    QColor m_textColor = QColor(Qt::black);
    QColor m_backgroundColor = QColor(Qt::transparent);
    ScreenplayFormat *m_format = nullptr;
    Qt::Alignment m_textAlignment = Qt::AlignLeft;
    SceneElement::Type m_elementType = SceneElement::Action;
    DefaultLanguage m_defaultLanguage = Default;
};

class ScreenplayPageLayout : public QObject
{
    Q_OBJECT

public:
    ScreenplayPageLayout(ScreenplayFormat *parent=nullptr);
    ~ScreenplayPageLayout();

    Q_PROPERTY(ScreenplayFormat* format READ format CONSTANT)
    ScreenplayFormat *format() const { return m_format; }

    enum PaperSize { A4, Letter };
    Q_ENUM(PaperSize)
    Q_PROPERTY(PaperSize paperSize READ paperSize WRITE setPaperSize NOTIFY paperSizeChanged STORED false)
    void setPaperSize(PaperSize val);
    PaperSize paperSize() const { return m_paperSize; }
    Q_SIGNAL void paperSizeChanged();

    Q_PROPERTY(QMarginsF margins READ margins NOTIFY rectsChanged STORED false)
    QMarginsF margins() const { return m_margins; }
    Q_SIGNAL void marginsChanged();

    Q_PROPERTY(qreal leftMargin READ leftMargin NOTIFY rectsChanged STORED false)
    qreal leftMargin() const { return m_margins.left(); }

    Q_PROPERTY(qreal topMargin READ topMargin NOTIFY rectsChanged STORED false)
    qreal topMargin() const { return m_margins.top(); }

    Q_PROPERTY(qreal rightMargin READ rightMargin NOTIFY rectsChanged STORED false)
    qreal rightMargin() const { return m_margins.right(); }

    Q_PROPERTY(qreal bottomMargin READ bottomMargin NOTIFY rectsChanged STORED false)
    qreal bottomMargin() const { return m_margins.bottom(); }

    Q_PROPERTY(QRectF paperRect READ paperRect NOTIFY rectsChanged STORED false)
    QRectF paperRect() const { return m_paperRect; }
    Q_SIGNAL void paperRectChanged();

    Q_PROPERTY(qreal paperWidth READ paperWidth NOTIFY rectsChanged STORED false)
    qreal paperWidth() const { return m_paperRect.width(); }

    Q_PROPERTY(qreal pageWidth READ pageWidth NOTIFY rectsChanged STORED false)
    qreal pageWidth() const { return m_paperRect.width(); }

    Q_PROPERTY(QRectF paintRect READ paintRect NOTIFY rectsChanged STORED false)
    QRectF paintRect() const { return m_paintRect; }
    Q_SIGNAL void paintRectChanged();

    Q_PROPERTY(QRectF headerRect READ headerRect NOTIFY rectsChanged STORED false)
    QRectF headerRect() const { return m_headerRect; }
    Q_SIGNAL void headerRectChanged();

    Q_PROPERTY(QRectF footerRect READ footerRect NOTIFY rectsChanged STORED false)
    QRectF footerRect() const { return m_footerRect; }
    Q_SIGNAL void footerRectChanged();

    Q_PROPERTY(QRectF contentRect READ contentRect NOTIFY rectsChanged STORED false)
    QRectF contentRect() const { return m_paintRect; }

    Q_PROPERTY(qreal contentWidth READ contentWidth NOTIFY rectsChanged STORED false)
    qreal contentWidth() const { return m_paintRect.width(); }

    Q_PROPERTY(qreal defaultResolution READ defaultResolution NOTIFY defaultResolutionChanged)
    qreal defaultResolution() const { return m_defaultResolution; }
    Q_SIGNAL void defaultResolutionChanged();

    Q_PROPERTY(qreal customResolution READ customResolution WRITE setCustomResolution NOTIFY customResolutionChanged)
    void setCustomResolution(qreal val);
    qreal customResolution() const { return m_customResolution; }
    Q_SIGNAL void customResolutionChanged();

    Q_PROPERTY(qreal resolution READ resolution NOTIFY resolutionChanged)
    qreal resolution() const { return m_resolution; }
    Q_SIGNAL void resolutionChanged();

    void configure(QTextDocument *document) const;
    void configure(QPagedPaintDevice *printer) const;

signals:
    void rectsChanged();

private:
    void evaluateRects();
    void evaluateRectsLater();
    void timerEvent(QTimerEvent *event);

    void setResolution(qreal val);
    void setDefaultResolution(qreal val);
    void loadCustomResolutionFromSettings();

private:
    qreal m_resolution = 0;
    QRectF m_paintRect;
    QRectF m_paperRect;
    QMarginsF m_margins;
    QRectF m_headerRect;
    QRectF m_footerRect;
    PaperSize m_paperSize = Letter;
    char m_padding[4];
    QPageLayout m_pageLayout;
    qreal m_customResolution = 0;
    qreal m_defaultResolution = 0;
    ScreenplayFormat *m_format = nullptr;
    ExecLaterTimer m_evaluateRectsTimer;
};

class ScreenplayFormat : public QAbstractListModel, public Modifiable
{
    Q_OBJECT

public:
    ScreenplayFormat(QObject *parent=nullptr);
    ~ScreenplayFormat();

    Q_PROPERTY(ScriteDocument* scriteDocument READ scriteDocument CONSTANT STORED false)
    ScriteDocument* scriteDocument() const { return m_scriteDocument; }

    Q_PROPERTY(QScreen* screen READ screen WRITE setScreen NOTIFY screenChanged RESET resetScreen STORED false)
    void setScreen(QScreen* val);
    QScreen* screen() const { return m_screen; }
    Q_SIGNAL void screenChanged();

    Q_INVOKABLE void setSreeenFromWindow(QObject *windowObject);

    qreal screenDevicePixelRatio() const { return m_screen ? m_screen->devicePixelRatio() : 1.0; }

    Q_PROPERTY(qreal devicePixelRatio READ devicePixelRatio NOTIFY fontZoomLevelIndexChanged)
    qreal devicePixelRatio() const;

    Q_PROPERTY(ScreenplayPageLayout* pageLayout READ pageLayout CONSTANT STORED false)
    ScreenplayPageLayout* pageLayout() const { return m_pageLayout; }

    Q_PROPERTY(TransliterationEngine::Language defaultLanguage READ defaultLanguage WRITE setDefaultLanguage NOTIFY defaultLanguageChanged)
    void setDefaultLanguage(TransliterationEngine::Language val);
    TransliterationEngine::Language defaultLanguage() const { return m_defaultLanguage; }
    Q_SIGNAL void defaultLanguageChanged();

    Q_PROPERTY(int defaultLanguageInt READ defaultLanguageInt WRITE setDefaultLanguageInt NOTIFY defaultLanguageChanged)
    int defaultLanguageInt() const { return int(m_defaultLanguage); }
    void setDefaultLanguageInt(int val) { this->setDefaultLanguage(TransliterationEngine::Language(val)); }

    Q_PROPERTY(QFont defaultFont READ defaultFont WRITE setDefaultFont NOTIFY defaultFontChanged)
    void setDefaultFont(const QFont &val);
    QFont defaultFont() const { return m_defaultFont; }
    QFont &defaultFontRef() { return m_defaultFont; }
    Q_SIGNAL void defaultFontChanged();

    Q_PROPERTY(QFont defaultFont2 READ defaultFont2 NOTIFY fontPointSizeDeltaChanged)
    QFont defaultFont2() const;

    QFontMetrics defaultFontMetrics() const { return m_defaultFontMetrics; }
    QFontMetrics defaultFont2Metrics() const { return m_defaultFont2Metrics; }

    Q_PROPERTY(int fontPointSizeDelta READ fontPointSizeDelta NOTIFY fontPointSizeDeltaChanged)
    int fontPointSizeDelta() const { return m_fontPointSizeDelta; }
    Q_SIGNAL void fontPointSizeDeltaChanged();

    Q_PROPERTY(int fontZoomLevelIndex READ fontZoomLevelIndex WRITE setFontZoomLevelIndex NOTIFY fontZoomLevelIndexChanged STORED false)
    void setFontZoomLevelIndex(int val);
    int fontZoomLevelIndex() const { return m_fontZoomLevelIndex; }
    Q_SIGNAL void fontZoomLevelIndexChanged();

    Q_PROPERTY(QVariantList fontZoomLevels READ fontZoomLevels NOTIFY fontZoomLevelsChanged)
    QVariantList fontZoomLevels() const { return m_fontZoomLevels; }
    Q_SIGNAL void fontZoomLevelsChanged();

    Q_INVOKABLE SceneElementFormat *elementFormat(SceneElement::Type type) const;
    Q_INVOKABLE SceneElementFormat *elementFormat(int type) const;
    Q_SIGNAL void formatChanged();

    Q_PROPERTY(QQmlListProperty<SceneElementFormat> elementFormats READ elementFormats)
    QQmlListProperty<SceneElementFormat> elementFormats();

    void applyToAll(const SceneElementFormat *from, SceneElementFormat::Properties properties);

    enum Role { SceneElementFomat=Qt::UserRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void resetToDefaults();

    void useUserSpecifiedFonts();

private:
    void resetScreen();
    void evaluateFontPointSizeDelta();
    void evaluateFontZoomLevels();

private:
    char  m_padding[4];
    QFont m_defaultFont;
    qreal m_pageWidth = 750.0;
    int   m_fontPointSizeDelta = 0;
    int m_fontZoomLevelIndex = -1;
    QList<int> m_fontPointSizes;
    QVariantList m_fontZoomLevels;
    QObjectProperty<QScreen> m_screen;
    ScriteDocument *m_scriteDocument = nullptr;
    QFontMetrics m_defaultFontMetrics;
    QFontMetrics m_defaultFont2Metrics;
    QStringList m_suggestionsAtCursor;
    ScreenplayPageLayout* m_pageLayout = new ScreenplayPageLayout(this);
    TransliterationEngine::Language m_defaultLanguage = TransliterationEngine::English;

    static SceneElementFormat* staticElementFormatAt(QQmlListProperty<SceneElementFormat> *list, int index);
    static int staticElementFormatCount(QQmlListProperty<SceneElementFormat> *list);
    QList<SceneElementFormat*> m_elementFormats;
};

class SceneDocumentBinder : public QSyntaxHighlighter, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

public:
    SceneDocumentBinder(QObject *parent=nullptr);
    ~SceneDocumentBinder();

    Q_PROPERTY(ScreenplayFormat* screenplayFormat READ screenplayFormat WRITE setScreenplayFormat NOTIFY screenplayFormatChanged RESET resetScreenplayFormat)
    void setScreenplayFormat(ScreenplayFormat* val);
    ScreenplayFormat* screenplayFormat() const { return m_screenplayFormat; }
    Q_SIGNAL void screenplayFormatChanged();

    Q_PROPERTY(Scene* scene READ scene WRITE setScene NOTIFY sceneChanged RESET resetScene)
    void setScene(Scene* val);
    Scene* scene() const { return m_scene; }
    Q_SIGNAL void sceneChanged();

    Q_PROPERTY(QQuickTextDocument* textDocument READ textDocument WRITE setTextDocument NOTIFY textDocumentChanged RESET resetTextDocument)
    void setTextDocument(QQuickTextDocument* val);
    QQuickTextDocument* textDocument() const { return m_textDocument; }
    Q_SIGNAL void textDocumentChanged();

    Q_PROPERTY(bool spellCheckEnabled READ isSpellCheckEnabled WRITE setSpellCheckEnabled NOTIFY spellCheckEnabledChanged)
    void setSpellCheckEnabled(bool val);
    bool isSpellCheckEnabled() const { return m_spellCheckEnabled; }
    Q_SIGNAL void spellCheckEnabledChanged();

    Q_PROPERTY(bool liveSpellCheckEnabled READ isLiveSpellCheckEnabled WRITE setLiveSpellCheckEnabled NOTIFY liveSpellCheckEnabledChanged)
    void setLiveSpellCheckEnabled(bool val);
    bool isLiveSpellCheckEnabled() const { return m_liveSpellCheckEnabled; }
    Q_SIGNAL void liveSpellCheckEnabledChanged();

    Q_PROPERTY(qreal textWidth READ textWidth WRITE setTextWidth NOTIFY textWidthChanged)
    void setTextWidth(qreal val);
    qreal textWidth() const { return m_textWidth; }
    Q_SIGNAL void textWidthChanged();

    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    void setCursorPosition(int val);
    int cursorPosition() const { return m_cursorPosition; }
    Q_SIGNAL void cursorPositionChanged();

    Q_SIGNAL void requestCursorPosition(int position);

    Q_PROPERTY(QStringList characterNames READ characterNames WRITE setCharacterNames NOTIFY characterNamesChanged)
    void setCharacterNames(const QStringList &val);
    QStringList characterNames() const { return m_characterNames; }
    Q_SIGNAL void characterNamesChanged();

    Q_PROPERTY(SceneElement* currentElement READ currentElement NOTIFY currentElementChanged RESET resetCurrentElement)
    SceneElement* currentElement() const { return m_currentElement; }
    Q_SIGNAL void currentElementChanged();

    Q_PROPERTY(int currentElementCursorPosition READ currentElementCursorPosition NOTIFY cursorPositionChanged)
    int currentElementCursorPosition() const { return m_currentElementCursorPosition; }

    Q_PROPERTY(bool forceSyncDocument READ isForceSyncDocument WRITE setForceSyncDocument NOTIFY forceSyncDocumentChanged)
    void setForceSyncDocument(bool val);
    bool isForceSyncDocument() const { return m_forceSyncDocument; }
    Q_SIGNAL void forceSyncDocumentChanged();

    Q_PROPERTY(QString nextTabFormatAsString READ nextTabFormatAsString NOTIFY nextTabFormatChanged)
    QString nextTabFormatAsString() const;

    Q_PROPERTY(int nextTabFormat READ nextTabFormat NOTIFY nextTabFormatChanged)
    int nextTabFormat() const;
    Q_SIGNAL void nextTabFormatChanged();

    Q_INVOKABLE void tab();
    Q_INVOKABLE void backtab();
    Q_INVOKABLE bool canGoUp();
    Q_INVOKABLE bool canGoDown();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void reload();

    Q_INVOKABLE int lastCursorPosition() const;
    Q_INVOKABLE int cursorPositionAtBlock(int blockNumber) const;

    Q_PROPERTY(QStringList spellingSuggestions READ spellingSuggestions NOTIFY spellingSuggestionsChanged)
    QStringList spellingSuggestions() const {return m_spellingSuggestions; }
    Q_SIGNAL void spellingSuggestionsChanged();

    Q_PROPERTY(bool wordUnderCursorIsMisspelled READ isWordUnderCursorIsMisspelled NOTIFY wordUnderCursorIsMisspelledChanged)
    bool isWordUnderCursorIsMisspelled() const { return m_wordUnderCursorIsMisspelled; }
    Q_SIGNAL void wordUnderCursorIsMisspelledChanged();

    Q_INVOKABLE QStringList spellingSuggestionsForWordAt(int position) const;

    Q_INVOKABLE void replaceWordAt(int position, const QString &with);
    Q_INVOKABLE void replaceWordUnderCursor(const QString &with) {
        this->replaceWordAt(m_cursorPosition, with);
    }

    Q_INVOKABLE void addWordAtPositionToDictionary(int position);
    Q_INVOKABLE void addWordUnderCursorToDictionary() {
        this->addWordAtPositionToDictionary(m_cursorPosition);
    }

    Q_INVOKABLE void addWordAtPositionToIgnoreList(int position);
    Q_INVOKABLE void addWordUnderCursorToIgnoreList() {
        this->addWordAtPositionToIgnoreList(m_cursorPosition);
    }

    Q_PROPERTY(QStringList autoCompleteHints READ autoCompleteHints NOTIFY autoCompleteHintsChanged)
    QStringList autoCompleteHints() const { return m_autoCompleteHints; }
    Q_SIGNAL void autoCompleteHintsChanged();

    Q_PROPERTY(QString completionPrefix READ completionPrefix NOTIFY completionPrefixChanged)
    QString completionPrefix() const { return m_completionPrefix; }
    Q_SIGNAL void completionPrefixChanged();

    Q_PROPERTY(QFont currentFont READ currentFont NOTIFY currentFontChanged)
    QFont currentFont() const;
    Q_SIGNAL void currentFontChanged();

    Q_PROPERTY(int documentLoadCount READ documentLoadCount NOTIFY documentLoadCountChanged)
    int documentLoadCount() const { return m_documentLoadCount; }
    Q_SIGNAL void documentLoadCountChanged();

    Q_SIGNAL void documentInitialized();

    Q_INVOKABLE void copy(int fromPosition, int toPosition);
    Q_INVOKABLE bool paste(int fromPosition=-1);

    // QQmlParserStatus interface
    void classBegin();
    void componentComplete();

protected:
    // QSyntaxHighlighter interface
    void highlightBlock(const QString &text);

    // QObject interface
    void timerEvent(QTimerEvent *te);

private:
    void resetScene();
    void resetTextDocument();
    void resetScreenplayFormat();

    void initializeDocument();
    void initializeDocumentLater();
    void setDocumentLoadCount(int val);
    void setCurrentElement(SceneElement* val);
    void resetCurrentElement();
    void onSceneElementChanged(SceneElement *element, Scene::SceneElementChangeType type);
    Q_SLOT void onSpellCheckUpdated();
    void onContentsChange(int from, int charsRemoved, int charsAdded);
    void syncSceneFromDocument(int nrBlocks=-1);
    bool eventFilter(QObject *object, QEvent *event);

    void evaluateAutoCompleteHints();
    void setAutoCompleteHints(const QStringList &val);
    void setCompletionPrefix(const QString &val);
    void setSpellingSuggestions(const QStringList &val);
    void setWordUnderCursorIsMisspelled(bool val);

    void onSceneAboutToReset();
    void onSceneReset(int position);

    void rehighlightLater();
    void rehighlightBlockLater(const QTextBlock &block);

private:
    friend class SpellCheckService;
    qreal m_textWidth = 0;
    int m_cursorPosition = -1;
    int m_documentLoadCount = 0;
    bool m_sceneIsBeingReset = false;
    bool m_forceSyncDocument = false;
    bool m_spellCheckEnabled = true;
    QString m_completionPrefix;
    bool m_initializingDocument = false;
    QStringList m_characterNames;
    bool m_liveSpellCheckEnabled = true;
    QObjectProperty<Scene> m_scene;
    ExecLaterTimer m_rehighlightTimer;
    QStringList m_autoCompleteHints;
    QStringList m_spellingSuggestions;
    int m_currentElementCursorPosition = -1;
    bool m_wordUnderCursorIsMisspelled = false;
    ExecLaterTimer m_initializeDocumentTimer;
    QList<SceneElement::Type> m_tabHistory;
    QList<QTextBlock> m_rehighlightBlockQueue;
    QObjectProperty<SceneElement> m_currentElement;
    QObjectProperty<QQuickTextDocument> m_textDocument;
    QObjectProperty<ScreenplayFormat> m_screenplayFormat;
};

#endif // FORMATTING_H
