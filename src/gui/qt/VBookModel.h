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
    void remove(const int row);
    void clear();
    QString expl(const int row) const;
    
    QString curExamExpl()  const;
    QString nextExamExpl();
    QString preExamExpl();
    bool   testInput(const QString& input, int& score);

private:
    mutable VocabularyBook* m_vocabularyBook;
    int examIndex;
};

#endif
