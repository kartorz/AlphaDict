#include "VBookModel.h"
#include <stdlib.h>

#define LIST_SIZE_MAX 1000

VBookModel::VBookModel(const string& vbookpath):m_currentRow(0),m_mode(StudyMode),m_score(0)
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

int VBookModel::columnCount(const QModelIndex &parent) const
{
    return m_vocabularyBook->colummCount();
}

QModelIndex VBookModel::curIndex() const
{
    return index(m_currentRow, 0);
}

QVariant VBookModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_vocabularyBook->size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        //printf("%d, %d\n", index.row(), index.column());
        //return QString::fromUtf8(m_vocabularyBook->getItem(index.row()).c_str());
        VBookItem item = m_vocabularyBook->getItem(index.row());

        switch (index.column()) {
            case 0:
                //printf("%s\n", item.word.c_str());
                if (m_mode == StudyMode || m_examResult[index.row()]) {
                    return QString::fromUtf8(item.word.c_str());
                }

                return QString("  -------------------- ");

            case 1: {
                int cmplxty = (item.complexity + item.count) / 3;
                if (cmplxty <= 0) 
                    cmplxty = 1;
                else if (cmplxty > 10)
                    cmplxty = 10;

                QString asterisk;
                for (int i = 0; i < cmplxty; i++)  asterisk += "*";
                return asterisk;
            }
            case 2:
                return QString::fromUtf8(item.date.c_str());
        }
        return QVariant();
     } else {
        return QVariant();
    }
}

QVariant VBookModel::headerData(int section, Qt::Orientation orientation, int role) const
{
     if (role != Qt::DisplayRole)
         return QVariant();

     if (orientation == Qt::Horizontal) {
         return QVariant();
         //return QString("Column %1").arg(section);
     }
     else
         return QString("%1").arg(section);
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

QModelIndex VBookModel::remove(const int row)
{
    beginResetModel();
    m_vocabularyBook->remove(row);
    endResetModel();
    int cur = row > m_vocabularyBook->size() -1 ? m_vocabularyBook->size() -1 : row;
    QModelIndex result = QAbstractItemModel::createIndex(cur, 0);
    return result;
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

void VBookModel::setMode(enum VBookMode mode)
{
    beginResetModel();

    m_mode = mode;
    if (mode == ExamMode) {
        m_examResult.assign(m_vocabularyBook->size(), false);
    } else {
        m_examResult.clear();
    }

    endResetModel();
}

QModelIndex VBookModel::moveUp()
{
    if (m_currentRow > 0) {
        --m_currentRow;
    }
    return curIndex();
}

QModelIndex VBookModel::moveDown()
{
    if (m_currentRow < m_vocabularyBook->size()-1) {
        ++m_currentRow;
    }
    return curIndex();
}

QModelIndex VBookModel::moveToFirst()
{
    m_currentRow = 0;
    return index(0, 0);
}

/*QString VBookModel::curExamExpl() const
{
    return expl(m_currentRow);
}*/

/*bool VBookModel::nextExamExpl(QString& text)
{
    if (m_examIndex < m_vocabularyBook->size()-1) {
        text = expl(++m_examIndex);
        return true;
    }
    return false;
}

bool VBookModel::preExamExpl(QString& text)
{
    if (m_examIndex > 0) {
        text =  expl(--m_examIndex);
        return true;
    }    
    return false;
}*/

bool VBookModel::study(const QString& input)
{
    string result = m_vocabularyBook->getWord(m_currentRow);
    if (result.compare(input.toUtf8().data()) == 0)
        return true;
    return false;
}

bool VBookModel::exam(const QString& input, QString& score)
{
    if (m_mode == ExamMode) {        
        VBookItem item = m_vocabularyBook->getItem(m_currentRow);
        string word = item.word;
        if (word.compare(input.toUtf8().data()) == 0) {
            if (m_examResult[m_currentRow] == false) {
                ++m_score;
                m_examResult[m_currentRow] = true;
                m_vocabularyBook->setComplexity(m_currentRow, --item.complexity);
            }
            score = QString("%1 / %2").arg(m_score).arg(m_vocabularyBook->size());
            return true;
        }
        
        m_vocabularyBook->setComplexity(m_currentRow, ++item.complexity);
        score = QString("%1 / %2").arg(m_score).arg(m_vocabularyBook->size());
    }
    return false;
}
