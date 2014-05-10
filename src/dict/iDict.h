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
    iIndexItem():index(""),addr(0),opaque(NULL)
	{
	}
    ~iIndexItem()
    {
        if (opaque)
		    free(opaque);
	}
	string index; /* utf-8 bytes */
    address_t addr;
	void *opaque;
};

typedef vector<iIndexItem*> IndexList;

class iDictItem
{
public:
	iDictItem():phonetic(""),expl(""),dictname(""),opaque(NULL)
    {
    }
    ~iDictItem(){}

    std::string dictname;  /* utf-8 bytes */
	std::string phonetic; /* utf-8 bytes */
	std::string expl;     /* utf-8 bytes */
    void *opaque;
};

typedef vector<iDictItem> DictItemList;

class iDict {
public:
    /* Load a dictionary */
    virtual bool load(const string& dictname) = 0;

    /* Lookup a word or phrase */
	virtual bool lookup(const string& word, DictItemList& itemList) = 0;

	virtual int indexListSize() { return 0; }

    /* Including 'start' but not incluing 'end', start from 0 */
	virtual int getIndexList(IndexList & indexList, int start, int end, const string& startwith="") = 0;

    /* For click on index item */
	virtual iDictItem  onClick(int row, iIndexItem* item) = 0;

    /* Identify the unique dict class name */
    virtual string identifier() = 0;

    virtual void summary(string& text) { text = "no summary"; }

    /* Check if a dictionary is supported by this dict class  */
    virtual bool support(const string& dictname) = 0;

    /* 'any' means the dictionary don't specify it */
    virtual void getLanguage(string& from, string& to) { from = "any"; to = "any"; };
};

#endif
