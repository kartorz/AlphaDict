#ifndef _ALDICT_H_
#define _ALDICT_H_

#include "iDict.h"
#include "AldictDocument.h"

using namespace std;

class Aldict: public iDict
{
public:
	Aldict(const string& dictpath);
    ~Aldict();
	virtual iDictItem lookup(const string& word);
	virtual IndexList* getIndexList();
	virtual iDictItem onClick(int index, iIndexItem* item);

private:
	AldictDocument m_doc;
	Aldict();
};
#endif
