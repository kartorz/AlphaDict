#include "DictIndexModel.h"
#include "DictManager.h"
#include "MessageQueue.h"

#include <stdlib.h>

#define INDEXLIST_SIZE_MAX  10000
#define OVERLAP_NR 100

#define STRING_MORE_B1  "<--------"
#define STRING_MORE_B2  "<<--------"

#define STRING_MORE_E1  "-------->"
#define STRING_MORE_E2  "-------->>"

DictIndexModel::DictIndexModel():m_indexStart(0),m_indexEnd(0)
{
    //QStringList list;
    //setStringList(list);
    m_indexList = new IndexList(); /* avoid to check NULL pointer */
}

DictIndexModel::~DictIndexModel()
{
    for (int i=0; i<m_indexList->size(); i++)
        delete (*m_indexList)[i];
    m_indexList->clear();
}

int DictIndexModel::rowCount(const QModelIndex &parent) const
{
    //MutexLock lock(m_cs);
    return m_indexList->size();
}

QVariant DictIndexModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_indexList->size())
        return QVariant();

    if (role == Qt::DisplayRole) {       
        //printf("data:%d\n", index.row());
        //MutexLock lock(m_cs);
        iIndexItem* item = (*m_indexList)[index.row()];
        //printf("data index:%s\n", item->index.c_str());
        QString strIndex;
        if (item != NULL)
            strIndex = QString::fromUtf8(item->index.c_str());
        return strIndex;
     } else {
        return QVariant();
    }
}

iIndexItem* DictIndexModel::item(int row)
{
    MutexLock lock(m_cs);
    return (*m_indexList)[row];
}

QVariant DictIndexModel::headerData(int section, Qt::Orientation orientation, int role) const
{
     if (role != Qt::DisplayRole)
         return QVariant();

     if (orientation == Qt::Horizontal)
         return QString("Column %1").arg(section);
     else
         return QString("Row %1").arg(section);
}

void DictIndexModel::onResetIndexList()
{
    MutexLock lock(m_cs);
    beginResetModel();
    for (int i=0; i<m_indexList->size(); i++)
        delete (*m_indexList)[i];
    m_indexList->clear();

    int ret = DictManager::getReference().getIndexList(*m_indexList, 0, INDEXLIST_SIZE_MAX);
    if (ret != INDEXLIST_SIZE_MAX)
	    m_indexEnd = ret;
	else 
	    m_indexEnd = DictManager::getReference().indexListSize();
    m_indexStart = 0;
    endResetModel();
}

QModelIndex DictIndexModel::updateIndexList(int pg)
{
    MutexLock lock(m_cs);
    QModelIndex result = QAbstractItemModel::createIndex(-1, 0);
    //printf("onUpdateIndexList: %d, %d\n", curitem, m_indexList->size());
    beginResetModel();
    
    if (pg < 0) {
	    if (m_indexStart == 0)
		    return result;
        IndexList* indexList = new IndexList();
        int start = m_indexStart + pg*INDEXLIST_SIZE_MAX;
		start = start < 0 ? 0 : start;
        DictManager::getReference().getIndexList(*indexList, start, start+INDEXLIST_SIZE_MAX);

		if (pg == -1) {
            for (int i=0; i<OVERLAP_NR; i++)
                indexList->push_back((*m_indexList)[i]);
			for (int i=OVERLAP_NR; i<m_indexList->size(); i++)
                delete (*m_indexList)[i];
		} else {
            for (int i=0; i<m_indexList->size(); i++)
                delete (*m_indexList)[i];
		}

        m_indexList->clear();
        delete m_indexList;
        m_indexList = indexList;
        m_indexStart = start;
    } else {
	    if (m_indexEnd && (m_indexEnd <= m_indexStart + m_indexList->size()))
		    return result;
        IndexList* indexList = new IndexList();
		int del_end = 0;
		if (pg == 1) {
            for (int i=INDEXLIST_SIZE_MAX - OVERLAP_NR; i<m_indexList->size()-1; i++) {
                indexList->push_back((*m_indexList)[i]);
            }
			del_end = INDEXLIST_SIZE_MAX - OVERLAP_NR;
		} else {

		    del_end = m_indexList->size();
		}

        int start = m_indexStart + pg*INDEXLIST_SIZE_MAX;
        int ret = DictManager::getReference().getIndexList(*indexList, start, start + INDEXLIST_SIZE_MAX);
        if (ret == INDEXLIST_SIZE_MAX) {
             // put 'more' item at the end.
            m_indexStart = start;
			for (int i=0; i<del_end; i++)
			    delete (*m_indexList)[i];
		 	m_indexList->clear();
            delete m_indexList;
            m_indexList = indexList;
			result = QAbstractItemModel::createIndex(OVERLAP_NR, 0);
        } else if (ret > 0){
		    for (int i=0; i<ret; i++)
                m_indexList->push_back((*indexList)[i]);
			if (pg != 1)
			    m_indexStart = start;
			//else append to end.
			m_indexEnd = start + ret;
	    }
    }
    endResetModel();
    return result;
}

