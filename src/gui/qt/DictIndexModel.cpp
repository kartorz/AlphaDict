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
}

DictIndexModel::~DictIndexModel()
{
    clearIndexList();
}

/* Drop every entry and release the storage.  The model owns the iIndexItem
 * objects that loadIndex()/getIndexList() hand over to it. */
void DictIndexModel::clearIndexList()
{
    for (size_t i=0; i<m_indexList.size(); i++)
        delete m_indexList[i];
    m_indexList.clear();
}

int DictIndexModel::rowCount(const QModelIndex &parent) const
{
    //MutexLock lock(m_cs);
    return m_indexList.size();
}

QVariant DictIndexModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_indexList.size())
        return QVariant();

    if (role == Qt::DisplayRole) {       
        //printf("data:%d\n", index.row());
        //MutexLock lock(m_cs);
        iIndexItem* item = m_indexList[index.row()];
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
    if (row < 0 || row >= (int)m_indexList.size())
        return NULL;
    return m_indexList[row];
}

iIndexItem* DictIndexModel::itemClone(int row)
{
    MutexLock lock(m_cs);
    if (row < 0 || row >= (int)m_indexList.size())
        return NULL;
    iIndexItem* src = m_indexList[row];
    return src != NULL ? src->clone() : NULL;
}

QHash<int, QByteArray> DictIndexModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    return roles;
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
    onResetIndexList(string(""));
}

void DictIndexModel::onResetIndexList(string startwith)
{
    MutexLock lock(m_cs);
    beginResetModel();
    clearIndexList();

    int ret = DictManager::getReference().getIndexList(m_indexList, 0, INDEXLIST_SIZE_MAX, startwith);
    if (ret != INDEXLIST_SIZE_MAX)
	    m_indexEnd = ret;
	else 
	    m_indexEnd = DictManager::getReference().indexListSize();
    m_indexStart = 0;
    m_indexPrefix = startwith;
    endResetModel();
}

QModelIndex DictIndexModel::updateIndexList(int pg)
{
    MutexLock lock(m_cs);
    QModelIndex result = QAbstractItemModel::createIndex(-1, 0);

    /* Refuse the no-op pages before the model is reset, otherwise
     * beginResetModel() is left unbalanced and Qt5 aborts the application. */
    if (pg < 0) {
        if (m_indexStart == 0)
            return result;
    } else if (m_indexEnd && (m_indexEnd <= m_indexStart + (int)m_indexList.size())) {
        return result;
    }

    beginResetModel();

    int start = m_indexStart + pg * INDEXLIST_SIZE_MAX;
    if (start < 0)
        start = 0;

    /* Entries carried over from the current page so the overlap stays visible.
     * Single-step paging keeps OVERLAP_NR of them; the double-arrow buttons
     * jump a whole page and keep nothing. */
    size_t keep = 0;
    if (pg == 1 || pg == -1)
        keep = m_indexList.size() < (size_t)OVERLAP_NR ? m_indexList.size()
                                                       : (size_t)OVERLAP_NR;

    if (pg < 0) {
        /* Backward paging rebuilds the page: the kept head of the page being
         * left is appended after the freshly loaded one, which keeps the rows
         * contiguous.  The list is replaced wholesale, so the entries below
         * the kept ones have to be released here. */
        IndexList indexList;
        DictManager::getReference().getIndexList(indexList, start,
                                                 start + INDEXLIST_SIZE_MAX, m_indexPrefix);
        for (size_t i = 0; i < keep; i++)
            indexList.push_back(m_indexList[i]);
        for (size_t i = keep; i < m_indexList.size(); i++)
            delete m_indexList[i];

        m_indexList = indexList;
        m_indexStart = start;
        endResetModel();
        return result;
    }

    /* Forward paging.  The kept tail goes in front of the freshly loaded page;
     * indexList[0..keep) are aliases of entries m_indexList still owns. */
    IndexList indexList;
    for (size_t i = m_indexList.size() - keep; i < m_indexList.size(); i++)
        indexList.push_back(m_indexList[i]);

    int ret = DictManager::getReference().getIndexList(indexList, start,
                                                       start + INDEXLIST_SIZE_MAX, m_indexPrefix);

    if (ret == INDEXLIST_SIZE_MAX) {
        /* Full page: replace the current one, keeping the carried tail. */
        for (size_t i = 0; i < m_indexList.size() - keep; i++)
            delete m_indexList[i];
        m_indexList = indexList;
        m_indexStart = start;
        result = QAbstractItemModel::createIndex(OVERLAP_NR, 0);
    } else if (ret > 0) {
        /* Short page at the end of the index: append the new entries to the
         * current page.  Only the freshly loaded part moves ownership. */
        for (size_t i = keep; i < indexList.size(); i++)
            m_indexList.push_back(indexList[i]);
        if (pg != 1)
            m_indexStart = start;
        m_indexEnd = start + ret;
    }
    /* ret <= 0: nothing was loaded, leave the current page untouched.
     * indexList only holds aliases that m_indexList still owns. */

    endResetModel();
    return result;
}
