#ifndef _VBOOKMODEL_H_
#define _VBOOKMODEL_H_
#include <QtCore/QAbstractTableModel>

#include "MutexLock.h"
#include "VocabularyBook.h"

enum VBookMode {
    StudyMode = 0,
    ExamMode,
};

class VBookModel: public QAbstractTableModel
{
     Q_OBJECT

public:
    VBookModel(const string& vbookpath);
    virtual ~VBookModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;

    bool add(const QString& word);
    QModelIndex remove(const int row);
    void clear();
    QString expl(const int row) const;
    void setCurrentRow(int row) {m_currentRow = row;}
    int getCurrentRow() {return m_currentRow;}
    bool isLastRow() {return m_currentRow == m_vocabularyBook->size()-1;}
    QModelIndex curIndex() const;

    QModelIndex moveUp();
    QModelIndex moveDown();
    QModelIndex moveToFirst();

    //QString curExamExpl()  const;
    //bool nextExamExpl(QString& text);
    //bool preExamExpl(QString& text);
    bool exam(const QString& input, QString& score);
    int  study(const QString& input, int repeat);

    void setMode(enum VBookMode mode);
    enum VBookMode getMode() const {return m_mode;}

private:
    mutable VocabularyBook* m_vocabularyBook;
    
    enum VBookMode m_mode;
    vector<bool> m_examResult;
    int m_currentRow;
    int m_score;
    int m_studyCount;
};

#endif
