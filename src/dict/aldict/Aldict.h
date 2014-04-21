#ifndef _ALDICT_H_
#define _ALDICT_H_

#include "iDict.h"
#include "AldictDocument.h"

using namespace std;

class Aldict: public iDict
{
public:
	Aldict(const string& dictname);
    Aldict() {}
    ~Aldict();
	virtual iDictItem lookup(const string& word);
	virtual IndexList* getIndexList();
	virtual iDictItem onClick(int index, iIndexItem* item);
    virtual const string identifier();
    virtual bool support(const string& dictname);
    virtual bool canLookup(const string& srcLan, const string& detLan);
    virtual bool load(const string& dictname);
private:
	AldictDocument m_doc;
};
#endif
