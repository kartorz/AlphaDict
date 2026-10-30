#ifndef _DICTINDEXMODEL_H_
#define _DICTINDEXMODEL_H_
#include <QtCore/QAbstractListModel>

#include "dict/iDict.h"
#include "MutexLock.h"
#include "dict/aldict/aldict_inner.h"

class DictIndexModel: public QAbstractListModel
{
     Q_OBJECT

public:
    DictIndexModel();
    virtual ~DictIndexModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual QHash<int, QByteArray> roleNames() const;
    iIndexItem* item(int row);

    /* Thread-safe detached copy of row, owned by the caller. */
    iIndexItem* itemClone(int row);

public slots:
    void onResetIndexList(string startwith);
    void onResetIndexList();   /* meta-callable overload, see QtMessager */
    QModelIndex updateIndexList(int pg);

private:
    /* Release every entry and empty the container. */
    void clearIndexList();

    /* Value member: the container itself used to be heap allocated and was
     * never freed, leaking its element buffer on every run. */
    IndexList  m_indexList;
    int m_indexStart;
	int m_indexEnd;
    string m_indexPrefix;
    MutexCriticalSection m_cs;
};

#endif
