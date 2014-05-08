#ifndef _ALDICTDOCUMENT_H_
#define _ALDICTDOCUMENT_H_
#include "kary_tree/kary_tree.hpp"
#include "alphadict.h"
#include "aldict_inner.h"
#include "dict/iDict.h"

using namespace ktree;
using namespace std;

class AldictDocument
{
public:
	 AldictDocument();
	~AldictDocument();

	bool loadDict(const std::string& path);
	address_t  lookup(const string& word, struct aldict_dataitem* item);
    struct aldict_dataitem dataitem(address_t addr);

	void writeToXml(const string& path);
	int  getIndexList(IndexList& indexList, int start, int end);
    bool support(const string& dictname);

	struct aldict_header m_header;
private:
	void loadIndexTree(tree_node<aldict_charindex>::treeNodePtr parent,
                       void *chrblock, address_t size);

    bool loadIndex(wchar_t *str, int inx, IndexList& indexList, 
                   tree_node<aldict_charindex>::treeNodePtr parent);

	bool readHeader();
	void readChrIndex();
	address_t lookup(char *strkey, tree_node<aldict_charindex>::treeNodePtr parent);
	address_t lookup(char *strkey, address_t off, int len);
    void* getBlock(int blk);

	kary_tree<aldict_charindex> *m_indexTree;
	address_t m_chrIndexLoc;
	address_t m_strIndexLoc;
	address_t m_dataLoc;
	FILE *m_dictFile;
	IndexList m_indexList;
    int m_indexStart;
    int m_indexEnd;
    int m_indexNumber;

    std::map<int, void*> m_cache;
};

#endif
