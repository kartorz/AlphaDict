#ifndef _IDICT_H_
#define _IDICT_H_
#include <map>
#include <string>
#include <vector>
#include <stdlib.h>

#include "type.h"

using namespace std;

class iIndexItem {
public:
    iIndexItem():index(NULL),inxlen(0),addr(0),opaque(NULL)
	{
	}
    ~iIndexItem()
    {
	    free(index);
		free(opaque);
	}
	wchar_t *index;
	int inxlen;
    address_t addr;
	void *opaque;
};

typedef vector<class iIndexItem*> IndexList;

class iDictItem
{
public:
	iDictItem(std::string identi=""):phonetic(""),expl(""),identify(identi),addr(0)
    {
    }
    ~iDictItem(){}

    std::string identify;
	std::string phonetic; /* utf-8 bytes */
	std::string expl;     /* utf-8 bytes */
	address_t addr;
};

class iDict {
public:
	virtual iDictItem lookup(const string& word) = 0;

     /* Derived class should maintains(new and delete) this returning pointers. */
	virtual IndexList* getIndexList() = 0;
	virtual iDictItem  onClick(int index, iIndexItem* item) = 0;
};

#endif
