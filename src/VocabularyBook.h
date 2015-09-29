#ifndef _VOCABULARYBOOK_H_
#define _VOCABULARYBOOK_H_
#include "tinyxml2/tinyxml2.h"

#include <string>
#include <list>
#include "iDB.h"

using namespace std;
using namespace tinyxml2;

struct VBookItem 
{
	string word;
    string expl;
    int count;
    string date;
};

class VocabularyBook
{
public:
    VocabularyBook();
    VocabularyBook(const string&  bookfile);
    bool load(const string& bookfile);

    bool add(const string& word);
    bool add(const string& word, const string& expl);
    void remove(const int row);
    void clear();
    int size() const;
    string getWord(const int row) const;
    string getExpl(const int row) const;
    VBookItem getItem(const int row) const;

private:
    static const string s_tblWordColumnName[];
    static const string s_tblWordColumnType[];
    static const int s_tblWordRowLen;
    static const string s_tblWordName;

    void updateCount(const string& word, int count);
    int getCount(const string& word);

    const int m_total  = 5000;

    bool wordExist(const string& word);

    int m_quantity;
    bool loadDB(const string& bookfile);
    bool loadXml(const string& bookfile);

    mutable list<struct VBookItem> m_wdlist;
    string m_bookpath;

    iDB* m_db;
};

#endif
