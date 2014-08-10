#ifndef _VBOOKMODEL_H_
#define _VBOOKMODEL_H_
#include <QtCore/QAbstractListModel>

#include "MutexLock.h"
#include "VocabularyBook.h"

class VBookModel: public QAbstractListModel
{
     Q_OBJECT

public:
    VBookModel(const string& vbookpath);
    virtual ~VBookModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;

    bool add(const QString& word);
    QModelIndex remove(const int row);
    void clear();
    QString expl(const int row) const;
    void setCurrentRow(int row) {m_currentRow = row; m_examIndex = row;}

    QString curExamExpl()  const;
    bool nextExamExpl(QString& text);
    bool preExamExpl(QString& text);
    bool testInput(const QString& input, int& score);

private:
    mutable VocabularyBook* m_vocabularyBook;
    int m_examIndex;
    int m_currentRow;
};

#endif
