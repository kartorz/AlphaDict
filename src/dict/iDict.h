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
	iDictItem(std::string identi=""):phonetic(""),expl(""),dictIdentifier(identi),addr(0)
    {
    }
    ~iDictItem(){}

    std::string dictIdentifier;
	std::string phonetic; /* utf-8 bytes */
	std::string expl;     /* utf-8 bytes */
	address_t addr;
};

class iDict {
public:
    /* Load a dictionary */
    virtual bool load(const string& dictname) = 0;

	virtual iDictItem lookup(const string& word) = 0;
     /* This calss should maintains(new and delete) this returning pointers. */
	virtual IndexList* getIndexList() = 0;
    
    /* For click on index item */
	virtual iDictItem  onClick(int index, iIndexItem* item) = 0;

    /* Identify the unique dict class name */
    virtual const std::string identifier() = 0;

    /* Check if a dictionary is supported by this dict class  */
    virtual bool support(const string& dictname) = 0;

    /* Check if this dictionary supports accroding lanauage */
    // @detLan == "any": means that we only need check srcLan.
    virtual bool canLookup(const string& srcLan, const string& detLan) = 0;
};

#endif
