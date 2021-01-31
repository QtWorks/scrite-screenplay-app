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

#ifndef SCREENPLAY_H
#define SCREENPLAY_H

#include "scene.h"
#include "modifiable.h"
#include "execlatertimer.h"
#include "qobjectproperty.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QQmlListProperty>

class Screenplay;
class ScriteDocument;

class ScreenplayElement : public QObject, public Modifiable
{
    Q_OBJECT

public:
    Q_INVOKABLE ScreenplayElement(QObject *parent=nullptr);
    ~ScreenplayElement();
    Q_SIGNAL void aboutToDelete(ScreenplayElement *element);

    enum ElementType
    {
        SceneElementType,
        BreakElementType
    };
    Q_ENUM(ElementType)
    Q_PROPERTY(ElementType elementType READ elementType WRITE setElementType NOTIFY elementTypeChanged)
    void setElementType(ElementType val);
    ElementType elementType() const { return m_elementType; }
    Q_SIGNAL void elementTypeChanged();

    Q_PROPERTY(int breakType READ breakType WRITE setBreakType NOTIFY breakTypeChanged)
    void setBreakType(int val);
    int breakType() const { return m_breakType; }
    Q_SIGNAL void breakTypeChanged();

    Q_PROPERTY(QString breakTitle READ breakTitle WRITE setBreakTitle NOTIFY breakTitleChanged)
    void setBreakTitle(const QString &val);
    QString breakTitle() const { return m_breakTitle.isEmpty() ? this->sceneID() : m_breakTitle; }
    Q_SIGNAL void breakTitleChanged();

    Q_PROPERTY(Screenplay* screenplay READ screenplay WRITE setScreenplay NOTIFY screenplayChanged STORED false RESET resetScreenplay)
    void setScreenplay(Screenplay *val);
    Screenplay* screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    Q_PROPERTY(QString sceneID READ sceneID WRITE setSceneFromID NOTIFY sceneChanged)
    void setSceneFromID(const QString &val);
    QString sceneID() const;

    Q_PROPERTY(int sceneNumber READ sceneNumber NOTIFY sceneNumberChanged)
    int sceneNumber() const { return m_customSceneNumber < 0 ? m_sceneNumber : m_customSceneNumber; }
    Q_SIGNAL void sceneNumberChanged();

    Q_PROPERTY(QString userSceneNumber READ userSceneNumber WRITE setUserSceneNumber NOTIFY userSceneNumberChanged)
    void setUserSceneNumber(const QString &val);
    QString userSceneNumber() const { return m_userSceneNumber; }
    Q_SIGNAL void userSceneNumberChanged();

    Q_PROPERTY(bool hasUserSceneNumber READ hasUserSceneNumber NOTIFY userSceneNumberChanged)
    bool hasUserSceneNumber() const { return !m_userSceneNumber.isEmpty(); }

    Q_PROPERTY(QString resolvedSceneNumber READ resolvedSceneNumber NOTIFY resolvedSceneNumberChanged)
    QString resolvedSceneNumber() const;
    Q_SIGNAL void resolvedSceneNumberChanged();

    Q_PROPERTY(Scene* scene READ scene NOTIFY sceneChanged STORED false RESET resetScene)
    void setScene(Scene *val);
    Scene* scene() const { return m_scene; }
    Q_SIGNAL void sceneChanged();

    Q_PROPERTY(bool expanded READ isExpanded WRITE setExpanded NOTIFY expandedChanged)
    void setExpanded(bool val);
    bool isExpanded() const { return m_expanded; }
    Q_SIGNAL void expandedChanged();

    Q_PROPERTY(QJsonValue userData READ userData WRITE setUserData NOTIFY userDataChanged STORED false)
    void setUserData(const QJsonValue &val);
    QJsonValue userData() const { return m_userData; }
    Q_SIGNAL void userDataChanged();

    Q_PROPERTY(QJsonValue editorHints READ editorHints WRITE setEditorHints NOTIFY editorHintsChanged)
    void setEditorHints(const QJsonValue &val);
    QJsonValue editorHints() const { return m_editorHints; }
    Q_SIGNAL void editorHintsChanged();

    Q_PROPERTY(bool selected READ isSelected WRITE setSelected NOTIFY selectedChanged STORED false)
    void setSelected(bool val);
    bool isSelected() const { return m_selected; }
    Q_SIGNAL void selectedChanged();

    Q_INVOKABLE void toggleSelection() { this->setSelected(!m_selected); }

    Q_SIGNAL void elementChanged();

    Q_SIGNAL void sceneAboutToReset();
    Q_SIGNAL void sceneReset(int elementIndex);
    Q_SIGNAL void evaluateSceneNumberRequest();
    Q_SIGNAL void sceneTypeChanged();

protected:
    bool event(QEvent *event);
    void evaluateSceneNumber(int &number);
    void resetScene();
    void resetScreenplay();

private:
    friend class Screenplay;
    bool m_expanded = true;
    int m_breakType = -1;
    bool m_selected = false;
    int m_sceneNumber = -1;
    QString m_sceneID;
    QString m_breakTitle;
    QJsonValue m_userData;
    int m_customSceneNumber = -1;
    bool m_elementTypeIsSet = false;
    QJsonValue m_editorHints;
    QString m_userSceneNumber;
    ElementType m_elementType = SceneElementType;
    QObjectProperty<Scene> m_scene;
    QObjectProperty<Screenplay> m_screenplay;
};

class Screenplay : public QAbstractListModel, public Modifiable, public QObjectSerializer::Interface
{
    Q_OBJECT
    Q_INTERFACES(QObjectSerializer::Interface)

public:
    Screenplay(QObject *parent=nullptr);
    ~Screenplay();
    Q_SIGNAL void aboutToDelete(Screenplay *ptr);

    Q_PROPERTY(ScriteDocument* scriteDocument READ scriteDocument CONSTANT STORED false)
    ScriteDocument* scriteDocument() const { return m_scriteDocument; }

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    void setTitle(const QString &val);
    QString title() const { return m_title; }
    Q_SIGNAL void titleChanged();

    Q_PROPERTY(QString subtitle READ subtitle WRITE setSubtitle NOTIFY subtitleChanged)
    void setSubtitle(const QString &val);
    QString subtitle() const { return m_subtitle; }
    Q_SIGNAL void subtitleChanged();

    Q_PROPERTY(QString logline READ logline WRITE setLogline NOTIFY loglineChanged)
    void setLogline(const QString &val);
    QString logline() const { return m_logline; }
    Q_SIGNAL void loglineChanged();

    Q_PROPERTY(QString basedOn READ basedOn WRITE setBasedOn NOTIFY basedOnChanged)
    void setBasedOn(const QString &val);
    QString basedOn() const { return m_basedOn; }
    Q_SIGNAL void basedOnChanged();

    Q_PROPERTY(QString author READ author WRITE setAuthor NOTIFY authorChanged)
    void setAuthor(const QString &val);
    QString author() const { return m_author; }
    Q_SIGNAL void authorChanged();

    Q_PROPERTY(QString contact READ contact WRITE setContact NOTIFY contactChanged)
    void setContact(const QString &val);
    QString contact() const { return m_contact; }
    Q_SIGNAL void contactChanged();

    Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)
    void setAddress(QString val);
    QString address() const { return m_address; }
    Q_SIGNAL void addressChanged();

    Q_PROPERTY(QString phoneNumber READ phoneNumber WRITE setPhoneNumber NOTIFY phoneNumberChanged)
    void setPhoneNumber(QString val);
    QString phoneNumber() const { return m_phoneNumber; }
    Q_SIGNAL void phoneNumberChanged();

    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)
    void setEmail(QString val);
    QString email() const { return m_email; }
    Q_SIGNAL void emailChanged();

    Q_PROPERTY(QString website READ website WRITE setWebsite NOTIFY websiteChanged)
    void setWebsite(QString val);
    QString website() const { return m_website; }
    Q_SIGNAL void websiteChanged();

    Q_PROPERTY(QString version READ version WRITE setVersion NOTIFY versionChanged)
    void setVersion(const QString &val);
    QString version() const { return m_version; }
    Q_SIGNAL void versionChanged();

    Q_PROPERTY(QString coverPagePhoto READ coverPagePhoto NOTIFY coverPagePhotoChanged STORED false)
    Q_INVOKABLE void setCoverPagePhoto(const QString &val);
    Q_INVOKABLE void clearCoverPagePhoto();
    QString coverPagePhoto() const { return m_coverPagePhoto; }
    Q_SIGNAL void coverPagePhotoChanged();

    enum CoverPagePhotoSize { SmallCoverPhoto, MediumCoverPhoto, LargeCoverPhoto };
    Q_ENUM(CoverPagePhotoSize)
    Q_PROPERTY(CoverPagePhotoSize coverPagePhotoSize READ coverPagePhotoSize WRITE setCoverPagePhotoSize NOTIFY coverPagePhotoSizeChanged)
    void setCoverPagePhotoSize(CoverPagePhotoSize val);
    CoverPagePhotoSize coverPagePhotoSize() const { return m_coverPagePhotoSize; }
    Q_SIGNAL void coverPagePhotoSizeChanged();

    Q_PROPERTY(bool titlePageIsCentered READ isTitlePageIsCentered WRITE setTitlePageIsCentered NOTIFY titlePageIsCenteredChanged)
    void setTitlePageIsCentered(bool val);
    bool isTitlePageIsCentered() const { return m_titlePageIsCentered; }
    Q_SIGNAL void titlePageIsCenteredChanged();

    Q_PROPERTY(bool hasTitlePageAttributes READ hasTitlePageAttributes NOTIFY hasTitlePageAttributesChanged)
    bool hasTitlePageAttributes() const { return m_hasTitlePageAttributes; }
    Q_SIGNAL void hasTitlePageAttributesChanged();

    Q_PROPERTY(bool hasNonStandardScenes READ hasNonStandardScenes NOTIFY hasNonStandardScenesChanged)
    bool hasNonStandardScenes() const { return m_hasNonStandardScenes; }
    Q_SIGNAL void hasNonStandardScenesChanged();

    Q_PROPERTY(QQmlListProperty<ScreenplayElement> elements READ elements NOTIFY elementsChanged)
    QQmlListProperty<ScreenplayElement> elements();
    Q_INVOKABLE void addElement(ScreenplayElement *ptr);
    Q_INVOKABLE void addScene(Scene *scene);
    Q_INVOKABLE void insertElementAt(ScreenplayElement *ptr, int index);
    Q_INVOKABLE void removeElement(ScreenplayElement *ptr);
    Q_INVOKABLE void moveElement(ScreenplayElement *ptr, int toRow);
    Q_INVOKABLE void moveSelectedElements(int toRow);
    Q_INVOKABLE void removeSelectedElements();
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE ScreenplayElement *elementAt(int index) const;
    Q_PROPERTY(int elementCount READ elementCount NOTIFY elementCountChanged)
    int elementCount() const;
    Q_INVOKABLE void clearElements();
    Q_SIGNAL void elementCountChanged();
    Q_SIGNAL void elementsChanged();
    Q_SIGNAL void elementInserted(ScreenplayElement *ptr, int index);
    Q_SIGNAL void elementRemoved(ScreenplayElement *ptr, int index);
    Q_SIGNAL void elementMoved(ScreenplayElement *ptr, int from, int to);

    Q_INVOKABLE ScreenplayElement *splitElement(ScreenplayElement *ptr, SceneElement *element, int textPosition);
    Q_INVOKABLE ScreenplayElement *mergeElementWithPrevious(ScreenplayElement *ptr);

    Q_INVOKABLE void removeSceneElements(Scene *scene);
    Q_INVOKABLE int firstIndexOfScene(Scene *scene) const;
    Q_INVOKABLE int indexOfElement(ScreenplayElement *element) const;
    Q_INVOKABLE QList<int> sceneElementIndexes(Scene *scene, int max=-1) const;
    QList<ScreenplayElement*> sceneElements(Scene *scene, int max=-1) const;
    Q_INVOKABLE int firstSceneIndex() const;
    Q_INVOKABLE int lastSceneIndex() const;

    QList<ScreenplayElement*> getElements() const { return m_elements; }
    bool setElements(const QList<ScreenplayElement*> &list);

    enum BreakType
    {
        Act,
        Chapter,
        Interval
    };
    Q_ENUM(BreakType)
    Q_INVOKABLE void addBreakElement(BreakType type);
    Q_INVOKABLE void addBreakElementI(int type) { this->addBreakElement(BreakType(type)); }
    Q_INVOKABLE void insertBreakElement(BreakType type, int index);
    Q_INVOKABLE void insertBreakElementI(int type, int index) { this->insertBreakElement(BreakType(type), index); }
    Q_SIGNAL void breakTitleChanged();

    Q_SIGNAL void screenplayChanged();

    Q_PROPERTY(int currentElementIndex READ currentElementIndex WRITE setCurrentElementIndex NOTIFY currentElementIndexChanged)
    void setCurrentElementIndex(int val);
    int currentElementIndex() const { return m_currentElementIndex; }
    Q_SIGNAL void currentElementIndexChanged(int val);

    Q_INVOKABLE int previousSceneElementIndex();
    Q_INVOKABLE int nextSceneElementIndex();

    Q_PROPERTY(Scene* activeScene READ activeScene WRITE setActiveScene NOTIFY activeSceneChanged STORED false RESET resetActiveScene)
    void setActiveScene(Scene* val);
    Scene* activeScene() const { return m_activeScene; }
    Q_SIGNAL void activeSceneChanged();

    Q_SIGNAL void sceneReset(int sceneIndex, int sceneElementIndex);

    Q_INVOKABLE QJsonArray search(const QString &text, int flags=0) const;
    Q_INVOKABLE int replace(const QString &text, const QString &replacementText, int flags=0);

    // QObjectSerializer::Interface interface
    void serializeToJson(QJsonObject &) const;
    void deserializeFromJson(const QJsonObject &);
    bool canSetPropertyFromObjectList(const QString &propName) const;
    void setPropertyFromObjectList(const QString &propName, const QList<QObject*> &objects);

    // QAbstractItemModel interface
    enum Roles { IdRole = Qt::UserRole, ScreenplayElementRole, ScreenplayElementTypeRole, BreakTypeRole, SceneRole, RowNumberRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int,QByteArray> roleNames() const;

protected:
    bool event(QEvent *event);
    void timerEvent(QTimerEvent *te);
    void resetActiveScene();
    void onSceneReset(int elementIndex);
    void evaluateSceneNumbers();
    void evaluateSceneNumbersLater();
    void validateCurrentElementIndex();
    void setHasNonStandardScenes(bool val);
    void setHasTitlePageAttributes(bool val);
    void evaluateHasTitlePageAttributes();
    QList<ScreenplayElement*> takeSelectedElements();

private:
    QString m_title;
    QString m_email;
    QString m_author;
    QString m_basedOn;
    QString m_logline;
    QString m_contact;
    QString m_version;
    QString m_website;
    QString m_address;
    QString m_subtitle;
    QString m_phoneNumber;
    QString m_coverPagePhoto;
    bool m_titlePageIsCentered = true;
    bool m_hasTitlePageAttributes = false;
    ScriteDocument *m_scriteDocument = nullptr;
    CoverPagePhotoSize m_coverPagePhotoSize = LargeCoverPhoto;

    static void staticAppendElement(QQmlListProperty<ScreenplayElement> *list, ScreenplayElement *ptr);
    static void staticClearElements(QQmlListProperty<ScreenplayElement> *list);
    static ScreenplayElement* staticElementAt(QQmlListProperty<ScreenplayElement> *list, int index);
    static int staticElementCount(QQmlListProperty<ScreenplayElement> *list);
    QList<ScreenplayElement *> m_elements; // We dont use ObjectListPropertyModel<ScreenplayElement*> for this because
                                           // the Screenplay class is already a list model of screenplay elements.
    int m_currentElementIndex = -1;
    QObjectProperty<Scene> m_activeScene;
    bool m_hasNonStandardScenes = false;

    ExecLaterTimer m_sceneNumberEvaluationTimer;
};

#endif // SCREENPLAY_H
