#ifndef _VOCABULARYBOOK_H_
#define _VOCABULARYBOOK_H_
#include "tinyxml2/tinyxml2.h"

#include <string>
#include <list>

using namespace std;
using namespace tinyxml2;

class VocabularyBook
{
public:
    VocabularyBook();
    VocabularyBook(const string&  bookfile);
    bool load(const string& bookfile);
    bool add(const string& word);
    void remove(const int row);
    void clear();
    int size() const;
    string getWord(const int row) const;
    string getExpl(const int row) const;

private:
    struct VBItem {
        string word;
        string expl;
    };

    mutable list<struct VBItem> m_wdlist;
    tinyxml2::XMLDocument m_doc;
    string m_bookpath;
};

#endif
