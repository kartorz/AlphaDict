#ifndef _ALDICTDOCUMENT_H_
#define _ALDICTDOCUMENT_H_
#include "kary_tree/kary_tree2.hpp"
#include "alphadict.h"
#include "aldict_inner.h"
#include "dict/iDict.h"
#include "MutexLock.h"

using namespace ktree;
using namespace std;

class AldictDocument
{
public:
    AldictDocument();
    ~AldictDocument();

    bool loadDict(const std::string& path);
    bool lookup(const string& word, vector<struct aldict_dataitem>& items, IndexList& candidate);
    struct aldict_dataitem dataitem(address_t addr);

    void writeToXml(const string& path);
    int  getIndexList(IndexList& indexList, int start, int end, const string& startwith="");
    bool support(const string& dictpath);

    struct aldict_header m_header;
private:
    struct IndexStat {
        int start;
        int end;
        int number;
    };

    struct LookupStat {
        string advance;
        tree_node<aldict_charindex>::treeNodePtr currentNode;
        vector<address_t> locs;
    };

    void loadIndexTree(tree_node<aldict_charindex>::treeNodePtr parent,
                       void *chrblock, address_t size);

    bool loadIndex(u4char_t *str, int inx, struct IndexStat *stat,
                   tree_node<aldict_charindex>::treeNodePtr parent,
                   IndexList& indexList);

    bool readHeader();
    void readChrIndex();

    bool lookup(char *strkey, tree_node<aldict_charindex>::treeNodePtr parent,
               struct LookupStat& lookupStat);
    bool lookup(char *strkey, address_t off, int len, struct LookupStat& lookupStat);
    void lookupCandidate(tree_node<aldict_charindex>::treeNodePtr parent,
                         string& header, IndexList& candidate);
    int  bsearch(tree_node<aldict_charindex>::treeNodePtr parent,
                 u4char_t key, int min, int max);

    void* getBlock(int blk);
    
    tree_node<aldict_charindex>::treeNodePtr
    findTreeNode(char *strkey, tree_node<aldict_charindex>::treeNodePtr parent, int* remain);

    kary_tree2<aldict_charindex> *m_indexTree;
    address_t m_chrIndexLoc;
    address_t m_strIndexLoc;
    address_t m_dataLoc;
    FILE *m_dictFile;
    IndexList m_indexList;
    int m_indexStart;
    int m_indexEnd;
    int m_indexNumber;

    std::map<int, void*> m_blkCache;
    MutexCriticalSection m_cs;
};

#endif
