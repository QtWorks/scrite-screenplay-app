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

#ifndef OBJECTLISTPROPERTYMODEL_H
#define OBJECTLISTPROPERTYMODEL_H

#include <QList>
#include <QAbstractListModel>

template <class T>
class ObjectListPropertyModel : public QAbstractListModel
{
public:
    ObjectListPropertyModel(QObject *parent=nullptr)
        : QAbstractListModel(parent) { }
    ~ObjectListPropertyModel() { }

    operator QList<T> () { return m_list; }
    QList<T>& list() { return m_list; }
    const QList<T> &list() const { return m_list; }

    bool empty() const { return m_list.empty(); }
    bool isEmpty() const { return m_list.isEmpty(); }

    void append(T ptr) {
        this->insert(-1, ptr);
    }

    void prepend(T ptr) {
        this->beginRemoveRows(QModelIndex(), 0, 0);
        m_list.prepend(ptr);
        this->endRemoveRows();
    }

    int indexOf(T ptr) const { return m_list.indexOf(ptr); }

    void removeAt(int row) {
        if(row < 0 || row >= m_list.size())
            return;

        this->beginRemoveRows(QModelIndex(), row, row);
        m_list.removeAt(row);
        this->endRemoveRows();
    }

    void insert(int row, T ptr) {
        int iidx = row < 0 || row >= m_list.size() ? m_list.size() : row;
        this->beginInsertRows(QModelIndex(), iidx, iidx);
        m_list.insert(iidx, ptr);
        this->endInsertRows();
    }

    void move(int fromRow, int toRow) {
        if(fromRow == toRow)
            return;

        if(fromRow < 0 || fromRow >= m_list.size())
            return;

        if(toRow < 0 || toRow >= m_list.size())
            return;

        this->beginMoveRows(QModelIndex(), fromRow, fromRow, QModelIndex(), toRow < fromRow ? toRow : toRow+1);
        m_list.move(fromRow, toRow);
        this->endMoveRows();
    }

    int size() const { return m_list.size(); }
    T at(int row) const { return m_list.at(row); }

    T first() const { return m_list.isEmpty() ? nullptr : m_list.first(); }
    T takeFirst() {
        T ptr = this->first();
        if(ptr == nullptr)
            return ptr;
        this->removeAt(0);
        return ptr;
    }

    T last() const { return m_list.isEmpty() ? nullptr : m_list.last(); }
    T takeLast() const {
        T ptr = this->first();
        if(ptr == nullptr)
            return ptr;
        this->removeAt(m_list.size()-1);
        return ptr;
    }

    T takeAt(int row) {
        T ptr = this->at(row);
        if(ptr == nullptr)
            return ptr;
        this->removeAt(row);
        return ptr;
    }

    // QAbstractItemModel interface
    enum { ObjectItemRole = Qt::UserRole+1, ModelDataRole };
    int rowCount(const QModelIndex &parent) const {
        return parent.isValid() ? 0 : m_list.size();
    }
    QVariant data(const QModelIndex &index, int role) const {
        if(role == ObjectItemRole || role == ModelDataRole) {
            QObject *ptr = index.row() < 0 || index.row() >= m_list.size() ? nullptr : m_list.at(index.row());
            return QVariant::fromValue<QObject*>(ptr);
        }
        return QVariant();
    }
    QHash<int, QByteArray> roleNames() const {
        QHash<int,QByteArray> roles;
        roles[ObjectItemRole] = QByteArrayLiteral("objectItem");
        roles[ModelDataRole] = QByteArrayLiteral("modelData");
        return roles;
    }

private:
    QList<T> m_list;
};

#endif // OBJECTLISTPROPERTYMODEL_H