#ifndef _DICTINDEXMODEL_H_
#define _DICTINDEXMODEL_H_

#include "dict/iDict.h"

#include <QAbstractListModel>

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
    iIndexItem* item(int row) { return (*m_list)[row]; }
private slots:
    void onUpdataList(void *v);

private:
    IndexList* m_list;
};

#endif
