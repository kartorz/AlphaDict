#include "VBookModel.h"
#include <stdlib.h>

#define LIST_SIZE_MAX 500

VBookModel::VBookModel(const string& vbookpath):examIndex(0)
{
    m_vocabularyBook = new VocabularyBook(vbookpath);
}

VBookModel::~VBookModel()
{
    delete m_vocabularyBook;
}

int VBookModel::rowCount(const QModelIndex &parent) const
{
    return m_vocabularyBook->size();
}

QVariant VBookModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_vocabularyBook->size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        return QString::fromUtf8(m_vocabularyBook->getWord(index.row()).c_str());
     } else {
        return QVariant();
    }
}

QVariant VBookModel::headerData(int section, Qt::Orientation orientation, int role) const
{
     if (role != Qt::DisplayRole)
         return QVariant();

     if (orientation == Qt::Horizontal)
         return QString("Column %1").arg(section);
     else
         return QString("Row %1").arg(section);
}

bool VBookModel::add(const QString& word)
{
    if (m_vocabularyBook->size() < LIST_SIZE_MAX) {
        beginResetModel();
        bool ret = m_vocabularyBook->add(std::string(word.toUtf8().data()));
        endResetModel();
        return ret;
    } else {
        return false;
    }
}

void VBookModel::remove(const int row)
{
    beginResetModel();
    m_vocabularyBook->remove(row);
    endResetModel();
}

void VBookModel::clear()
{
    beginResetModel();
    m_vocabularyBook->clear();
    endResetModel();
}

QString VBookModel::expl(const int row) const
{
    return QString::fromUtf8(m_vocabularyBook->getExpl(row).c_str());
}

QString VBookModel::curExamExpl() const
{
    return expl(examIndex);
}

QString VBookModel::nextExamExpl()
{
    if (examIndex < m_vocabularyBook->size()-1) {
        return expl(++examIndex);
    }
    return expl(examIndex);
}

QString VBookModel::preExamExpl()
{
    if (examIndex > 0) {
        return expl(--examIndex);
    }

    return expl(examIndex);
}

bool VBookModel::testInput(const QString& input, int& score)
{
    string result = m_vocabularyBook->getWord(examIndex);
    if (result.compare(input.toUtf8().data()) == 0)
        return true;
    return false;
}
