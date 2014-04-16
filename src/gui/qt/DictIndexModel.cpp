#include "DictIndexModel.h"
//#include <QDebug>

DictIndexModel::DictIndexModel():m_list(NULL)
{
    //QStringList list;
    //setStringList(list);
}

DictIndexModel::~DictIndexModel()
{
    m_list = NULL;
}

int DictIndexModel::rowCount(const QModelIndex &parent) const
{
    if(m_list == NULL)
        return 0;
    return m_list->size();
}

QVariant DictIndexModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || m_list == NULL)
        return QVariant();

    if (index.row() >= m_list->size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        iIndexItem* item = (*m_list)[index.row()];
        QString strIndex = QString::fromWCharArray(item->index, item->inxlen);
        //qDebug() << strIndex;
        return strIndex;
     } else {
        return QVariant();
    }
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

void DictIndexModel::onUpdataList(void *v)
{
    beginResetModel();
    m_list = (IndexList*)v;
    endResetModel();
}
