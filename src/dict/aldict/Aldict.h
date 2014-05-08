#ifndef _ALDICT_H_
#define _ALDICT_H_

#include "iDict.h"
#include "AldictDocument.h"

using namespace std;

class Aldict: public iDict
{
public:
	Aldict(const string& dictname);
    Aldict();
    ~Aldict();
	virtual iDictItem lookup(const string& word);
	virtual int indexListSize();
	virtual int getIndexList(IndexList& indexList, int start, int end);
	virtual iDictItem onClick(int index, iIndexItem* item);
    virtual string identifier();
    virtual bool support(const string& dictname);
    virtual bool load(const string& dictname);
    virtual void getLanguage(string& from, string& to);
    virtual void summary(string& text);
private:
	AldictDocument m_doc;
    bool m_bLoad;
};
#endif
