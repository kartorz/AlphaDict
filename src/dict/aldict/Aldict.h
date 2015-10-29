#ifndef _ALDICT_H_
#define _ALDICT_H_

#include "iDict.h"
#include "AldictDocument.h"

using namespace std;

class Aldict: public iDict
{
public:
    Aldict(const string& dictpath);
    Aldict();
    ~Aldict();
    virtual bool lookup(const string& word, DictItemList& itemList);
    virtual int indexListSize();
    virtual int getIndexList(IndexList& indexList, int start, int end, const string& startwith="");
    virtual iDictItem onClick(int row, iIndexItem* item);
    virtual string identifier();
    virtual bool support(const string& dictpath);
    virtual bool load(const string& dictpath);
    virtual void getLanguage(string& from, string& to);
    virtual void summary(string& text);

private:
    bool lookupCache(const string& key, DictItemList& itemList);
    void addToDictCache(const string& key, const DictItemList& itemList);
    void addToIndexCache(const string key, const iIndexItem& item);
    void dataItemTodictItem(const struct aldict_dataitem& d, iDictItem& i);

    AldictDocument m_doc;
    std::map<string, DictItemList> m_dictCache;
    std::map<string, iIndexItem> m_indexCache;

    MutexCriticalSection m_cs;
    bool m_bLoad;
};
#endif
