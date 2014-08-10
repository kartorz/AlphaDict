/* For saving memory:
 *     - load char index partly, the others load dynamically into a cache tree 
 *       that should can load a whole subtree at least.
 *     
 * 
 */
# ifdef WIN32
#include <Windows.h>
# endif
#include <wchar.h>

#include "AldictDocument.h"
#include "tinyxml2/tinyxml2.h"
#include "Log.h"
#include "CharUtil.h"
#include "Util.h"

#define CACHE_SLICE_MAX  400  /* 100k */

#define MEM_CHARINX_MAX  20*1024*1024 /*M*/

#define INDEXARRY_LEN_MAX  256

AldictDocument::AldictDocument():m_indexTree(NULL)
{
}

AldictDocument::~AldictDocument()
{
    if (m_dictFile)
	    fclose(m_dictFile);
    std::map<int, void*>::iterator iter;
    for (iter=m_blkCache.begin(); iter!=m_blkCache.end(); iter++) {
        free(iter->second);
    }
    
    for (int i=0; i<m_indexList.size(); i++) {
        delete m_indexList[i];
    }
    if (m_indexTree)
        delete m_indexTree;
}

bool AldictDocument::loadDict(const std::string& dictpath)
{
#ifdef WIN32
   wchar_t *wdictname = CharUtil::utf8srtowcs(dictpath.c_str());
   if (wdictname != NULL) {
       m_dictFile = _wfopen(wdictname, L"rb");
       free(wdictname);
   }
#else
    m_dictFile = fopen(dictpath.c_str(),"rb");
#endif
	if (m_dictFile == NULL) {
	    g_log.e("Can't open dict file:(%s)", dictpath.c_str());
		return false;
	}
	g_log.i("read meta data of(%s)\n", dictpath.c_str());
	if (readHeader()) {
	    readChrIndex();	
	    return true;
    }
    return false;
}

bool AldictDocument::readHeader()
{
	util::ReadFile read;
	size_t size = read(m_dictFile, &m_header, sizeof(struct aldict_header));
    if (size < sizeof(struct aldict_header))
        return false;

    if (m_header.magic[0] == ALD_MAGIC_L && m_header.magic[1] == ALD_MAGIC_H) {
	    m_chrIndexLoc = m_header.loc_chrindex[0];
	    m_strIndexLoc = ald_read_u32(m_header.loc_strindex);
	    m_dataLoc = ald_read_u32(m_header.loc_data);
        return true;
    }
    g_log.i("not a alphadict's dictionary\n");
    return false;
}

void AldictDocument::readChrIndex()
{
	util::ReadFile read;
	util::Malloc maclloc_t;
	address_t len = (m_strIndexLoc - m_chrIndexLoc)*ALD_BLOCK;
	void* chrblock = maclloc_t(len);

    g_log.d("{readChrIndex}, charindex size == (%d)bytes \n", len);
	fseek(m_dictFile, (m_chrIndexLoc-1)*ALD_BLOCK, SEEK_SET);
	read(m_dictFile, chrblock, len);

	struct aldict_charindex rootIndex;
	rootIndex = *((struct aldict_charindex*)chrblock);
    m_indexTree = new kary_tree2<aldict_charindex>(rootIndex);
    if (true/*len <= MEM_CHARINX_MAX*/) {
	    loadIndexTree(m_indexTree->root(), chrblock, len);
    } else {
        // Todo: Loading a part of index character.
    }
}

void AldictDocument::loadIndexTree(tree_node<aldict_charindex>::treeNodePtr parent,
                                   void *chrblock, address_t blksize)
{		
	struct aldict_charindex& parInx = parent->value();
	address_t loc = ald_read_u32(parInx.location);
    u16 len = ald_read_u16(parInx.len_content);

    if (len > 5000) {
        g_log.w("{loadIndexTree} more than 5000 child need to be loaded, someting wrong?\n");
    }

	//g_log.d("{loadIndexTree} parent loc: (%u-->0x%x), len:(%d)\n", loc, loc, len);
	if ((loc & 0x80000000) == 0 && len > 0) { /* non-leaf */
        if (loc + (len-1) * sizeof(struct aldict_charindex) > blksize) {
            g_log.e("{loadIndexTree} (loc(%u) --> len(%u) )over char index aread\n", loc, len);
            return;
        }

	    for (u32 i=0; i<len; i++) {
		    struct aldict_charindex chrInx;
            address_t off = loc + i*sizeof(struct aldict_charindex);
			memcpy(&chrInx,(u8 *)chrblock + off,sizeof(struct aldict_charindex));
			parent->insert(chrInx);
            // Recursion
			loadIndexTree((*parent)[i], chrblock, blksize);
		}
	}
}

bool AldictDocument::lookup(const string& word, vector<struct aldict_dataitem>& items, IndexList& candidate)
{
    struct LookupStat lookupStat;
    lookupStat.advance = word;

    if (lookup((char*)word.c_str(), m_indexTree->root(), lookupStat)) {
        for (int i=0; i<lookupStat.locs.size(); i++) {
            struct aldict_dataitem result;
            result = dataitem(lookupStat.locs[i]);
            items.push_back(result);
            //addToInxCache(word, loc);
        }
        return true;
    }

    lookupCandidate(lookupStat.currentNode, lookupStat.advance, candidate);
    return false;
}

int AldictDocument::bsearch(tree_node<aldict_charindex>::treeNodePtr parent,
                            u4char_t key, int min, int max)
{
    int mid = (min + max) / 2;
    u32 chr = ald_read_u32(parent->child(mid)->value().wchr);
    //printf("bsearch mid(%d), min(%d), max(%d): %d-->%d\n", mid, min, max, chr, key);
    if (min == max) {
        if (chr == key)
            return min;
        return -1;
    }
	if (chr == key)
        return mid;
    if (chr < key)
	    return bsearch(parent, key, mid+1, max);
    
    if (mid > min)
        return bsearch(parent, key, min, mid-1);
    else
        return -1;
}

/* Look up in char index tree,
 * If we can't find the 'target', return a candidate string.
 */

bool AldictDocument::lookup(char *strkey, tree_node<aldict_charindex>::treeNodePtr parent, struct LookupStat& lookupStat)
{
    int remain = strlen(strkey); /* for lookup candidate, mbrtowc_r will remove some chars */
    /* 'strkey' will be modified, some chars will be removed */
    const u4char_t key = CharUtil::utf8byteToUCS4Char((const char**)&strkey);
    //printf("lookup, %lc\n", key);
    int cid = bsearch(parent, key, 0, parent->children().size()-1);
	if (cid != -1) {
	    if (strlen(strkey) > 0) {
	        if (parent->child(cid)->children().size() > 0) {
                    return lookup(strkey, (*parent)[cid], lookupStat);
		} else {
                    struct aldict_charindex chrInx = parent->child(cid)->value();
		    address_t loc = ald_read_u32(chrInx.location);
		    int len = ald_read_u16(chrInx.len_content);
		    if ((loc & F_LOCSTRINX) == F_LOCSTRINX) {
	                if (lookup(strkey, loc & (~F_LOCSTRINX), len, lookupStat))
                            return true;

                        int len = lookupStat.advance.length() - strlen(strkey); /* it is not the same as 'remain' */
                        lookupStat.advance = lookupStat.advance.substr(0, len);
                        lookupStat.currentNode = (*parent)[cid];
                        return false;
		    } else /* leaf node without a string index */  {
                        int len = lookupStat.advance.length() - strlen(strkey);
                        lookupStat.advance = lookupStat.advance.substr(0, len);
                        lookupStat.currentNode = NULL;
                        return false;
                    }
	        }
	} else { /* advance to the end of strkey */
            int csize = parent->child(cid)->children().size();
		    if (csize > 0) {
                /* Maybe, there are same indexs with different explantion. God, I hate this.*/
                for (int i=0; i< csize; i++) {
			        struct aldict_charindex chrInx = parent->child(cid)->child(i)->value();
				    if (ald_read_u32(chrInx.wchr) == 0) {
				        lookupStat.locs.push_back(ald_read_u32(chrInx.location));
                    } else {
                        break;
                    }
                }

				if (lookupStat.locs.size() == 0) {
                    lookupStat.currentNode = (*parent)[cid];
                    return false;
			    }
                return true;
			} else {
                struct aldict_charindex chrInx = parent->child(cid)->value();
				address_t loc = ald_read_u32(chrInx.location);
			    if ((loc & F_LOCSTRINX) == 0) {
				    lookupStat.locs.push_back(loc);
                    return true;
				} else {
                    lookupStat.currentNode = (*parent)[cid];
				    return false;
                }
			}
		}
	}

    int total = lookupStat.advance.length();
    lookupStat.advance = lookupStat.advance.substr(0, total-remain); // get the common string.
    lookupStat.currentNode = parent;
	return false;
}

/* Look up in string index area */
bool AldictDocument::lookup(char* strkey, address_t off, int len, struct LookupStat& lookupStat)
{
    //printf("lookup2, %s\n", strkey);
    bool result = false;
	int block_nr = off/ALD_BLOCK + m_strIndexLoc;
	off = off%ALD_BLOCK;
    u8 *buf = (u8 *)getBlock(block_nr) + off; /* Load index has checked if NULL. */

	int len_key = strlen(strkey);
	for (int nr = 0; nr < len; nr++) {
		struct aldict_stringindex *pStrInx = ( struct aldict_stringindex *) buf;
		if (pStrInx->len_str[0] == 0) {
			// Read next block
			buf = (u8 *)getBlock(++block_nr);            
			pStrInx = ( struct aldict_stringindex *) buf;
		}
        
		if (len_key == pStrInx->len_str[0]) {
		    char *strinx = (char *)(pStrInx->str);
			bool found = true;
			// compasion from tail to head.
		    for (int i=len_key; i>0; i--) {
				if (strinx[i-1] != strkey[i-1]) {
				    found = false;
					break;
				}
			}
			// found.
			if (found == true) {
				lookupStat.locs.push_back(ald_read_u32(pStrInx->location));
                result = true;
                //printf("lookup multi-result %u\n", ald_read_u32(pStrInx->location));
			} else if (result){
                /* the dumplicate indexes saved together */
                //printf("lookup same result done\n");
                return true;
            }
	    }
		buf += 5 + pStrInx->len_str[0];
	}
	return result;
}

void AldictDocument::lookupCandidate(tree_node<aldict_charindex>::treeNodePtr parent,
                                     string& header, IndexList& candidate)
{
    if (parent != NULL) {
        u4char_t index[INDEXARRY_LEN_MAX];
    	memset(index, L'\0', INDEXARRY_LEN_MAX);
        struct IndexStat stat;
        stat.start = 0;
        stat.end = 5;
        stat.number = 0; /* start from 0 */
        //IndexList indexList;
        //string header = candidate;
        //candidate.clear();
        loadIndex(index, 0, &stat, parent, candidate);
        for (int i=0; i<candidate.size(); i++) {
            candidate[i]->index = header + candidate[i]->index;
            //candidate = candidate + index + '\n';
            //addToInxCache(index, candidate[i]->addr);
        }
    }
}

struct aldict_dataitem AldictDocument::dataitem(address_t loc)
{	
    struct aldict_dataitem dataItem;
	memset(&dataItem, 0, sizeof(struct aldict_dataitem));
	if (loc != ALD_INVALID_ADDR) {
		util::ReadFile read;
    	fseek(m_dictFile, (m_dataLoc-1)*ALD_BLOCK+loc, SEEK_SET);

		u8 *buf = (u8 *)read(m_dictFile, 1);
		dataItem.len_word = buf[0];
		dataItem.ptr_word = (u8 *)malloc(dataItem.len_word);
		buf = (u8 *)read(m_dictFile, dataItem.len_word);
		memcpy(dataItem.ptr_word, buf, dataItem.len_word);

		buf = (u8 *)read(m_dictFile, 1);
		dataItem.len_phon = buf[0];
		if (dataItem.len_phon > 0) {
			dataItem.ptr_phon = (u8 *)malloc(dataItem.len_phon);
			buf = (u8 *)read(m_dictFile, dataItem.len_phon);
			memcpy(dataItem.ptr_phon, buf, dataItem.len_phon);
		}
		buf = (u8 *)read(m_dictFile, 2);
		dataItem.len_expl = ald_read_u16(buf);
		if (dataItem.len_expl > 0) {
			dataItem.ptr_expl = (u8 *)malloc(dataItem.len_expl);
			buf = (u8 *)read(m_dictFile, dataItem.len_expl);
			memcpy(dataItem.ptr_expl, buf, dataItem.len_expl);
		}
	}
	return dataItem;
}

int AldictDocument::getIndexList(IndexList& indexList, int start, int end, const string& startwith)
{
    if (!m_indexTree)
        return 0;

    u4char_t index[INDEXARRY_LEN_MAX];
    memset(index, L'\0', INDEXARRY_LEN_MAX);
    int index_star = 0;
    struct IndexStat stat;
    stat.start = start;
    stat.end = end;
    stat.number = 0; /* start from 0 */
    
    tree_node<aldict_charindex>::treeNodePtr root = m_indexTree->root();
    if (startwith != "") {
        string key = startwith;
        int remain;
        root = findTreeNode((char*)key.c_str(), root, &remain);
        string prefix = startwith.substr(0, startwith.length()-remain);
        //printf("%s, %s, %d\n", startwith.c_str(), prefix.c_str(), remain);
        char* pkey = (char *)(prefix.c_str());
        size_t u4len;
        u4char_t* u4str = CharUtil::utf8StrToUcs4Str(pkey, &u4len);
        if (u4len > 0) {
            memcpy(index, u4str, sizeof(u4char_t)*u4len);
            index_star = u4len;
        }        
        free((void *)u4str);
    }

    loadIndex(index, index_star, &stat, root, indexList);
    return stat.number - stat.start; /* indexListSize use this feature.*/
}

/* @return : false -> abort */
bool AldictDocument::loadIndex(u4char_t *str, int inx, struct IndexStat *stat,
                               tree_node<aldict_charindex>::treeNodePtr parent,
                               IndexList& indexList)
{
    int children_size = parent->children().size();
	if (children_size > 0) {
	    for (int i=0; i<children_size; i++) {
	        struct aldict_charindex chrInx = parent->child(i)->value();           
            if (inx < INDEXARRY_LEN_MAX -1) {
                str[inx] = ald_read_u32(chrInx.wchr);
                if (loadIndex(str, inx+1, stat, parent->child(i), indexList) == false)
                    return false;
            } else {
                g_log.d("{loadIndex} index is too long, should't happen\n");
                return false;
            }
        }
        return true;
	}

    // Leaf node
    string strparent;
    {
        str[inx] = L'\0';
        char* pinx = CharUtil::ucs4StrToUTF8Str(str);
        //char* pinx = Util::wcsrtombs_r(str);
        if (pinx == NULL) {
            g_log.e("{loadIndex} invalid wcstring\n");
            return false;
        }
        strparent = string(pinx);
        //printf("strparent %s\n", strparent.c_str());
        free(pinx);
    }

    struct aldict_charindex chrInx = parent->value();
	address_t loc = ald_read_u32(chrInx.location);
	int length = ald_read_u16(chrInx.len_content);
	if ((loc & F_LOCSTRINX) == 0) {
        if (stat->number >= stat->start) {
            if (stat->number < stat->end) {
                iIndexItem* item = new iIndexItem();
                item->index = strparent;
                item->addr = loc;
                indexList.push_back(item);
            } else {
                return false;
            }
        }
        ++stat->number;
	} else {
        loc = loc & (~F_LOCSTRINX);
        int bk_off = loc/ALD_BLOCK;
        int addr_off = loc%ALD_BLOCK;
        int block_nr = m_strIndexLoc+bk_off;
        u8 *buf_start, *buf_end;
        if (bk_off > m_dataLoc - m_strIndexLoc) {
             g_log.e("{loadIndex} a invalid addr (%x\n", loc);
             return false;
        }

        if ((buf_start = (u8 *)getBlock(block_nr)) == NULL)
            return false;
        buf_end = buf_start + ALD_BLOCK;

        buf_start += addr_off; // move buf_start
        
		for (int item = 0; item < length; item++) {
            struct aldict_stringindex *pStrInx;
            if (buf_end <= buf_start || buf_end - buf_start < sizeof(struct aldict_stringindex))
                goto READ_NEXT_BLOCK;
           
	        pStrInx = (struct aldict_stringindex *) buf_start;
            if (pStrInx->len_str[0] != 0)
                 goto LOADING;

        READ_NEXT_BLOCK:
		    // Read next block
            ++block_nr;
            if ((buf_start = (u8 *)getBlock(block_nr)) == NULL)
                return false;
            buf_end = buf_start + ALD_BLOCK;
            pStrInx = (struct aldict_stringindex *) buf_start;

            // Check pStrInx.
            if (pStrInx->len_str[0] == 0) {
                g_log.e("{loadIndex} read a invalid data area \n");
                return false;
	        }

       LOADING:
            if (stat->number >= stat->start) {
                if (stat->number < stat->end) {
                    string strinx = string((char*)(pStrInx->str), pStrInx->len_str[0]);
                    //printf("strinx %s, %d\n", strinx.c_str(), pStrInx->len_str[0]);
                    iIndexItem* item = new iIndexItem();
                    item->index = strparent + strinx;
                    item->addr = ald_read_u32(pStrInx->location);
                    indexList.push_back(item);
                } else {
                    return false;
                }
            }
            ++stat->number;
            buf_start += 5 + pStrInx->len_str[0];
		}
	}
    return true;
}

tree_node<aldict_charindex>::treeNodePtr
AldictDocument::findTreeNode(char *strkey, tree_node<aldict_charindex>::treeNodePtr parent, int* remain)
{
    *remain = strlen(strkey);
    if (*strkey != '\0') {
        /* 'strkey' will be modified, some chars will be removed */
	const u4char_t key = CharUtil::utf8byteToUCS4Char((const char**)&strkey);

        if (parent->children().size() > 0) {
            int i = bsearch(parent, key, 0, parent->children().size()-1);
            if (i != -1) {
	        	struct aldict_charindex chrInx = parent->child(i)->value();
	        	u4char_t chr = ald_read_u32(chrInx.wchr);
	        	if (chr == key) {
                    return findTreeNode(strkey, (*parent)[i], remain);
	        	}
	        }
        }
    }
	return parent;
}

void* AldictDocument::getBlock(int blk)
{
    map<int, void*>::iterator iter = m_blkCache.find(blk);
    if(iter == m_blkCache.end()) {
        MutexLock lock(m_cs);
        util::ReadFile read;
        void *ptr = malloc(ALD_BLOCK);
        memset(ptr, 0, ALD_BLOCK);
        if (ptr != NULL) {
            if (fseek(m_dictFile, (blk-1)*ALD_BLOCK, SEEK_SET) == 0) {
                read(m_dictFile, ptr, ALD_BLOCK);
                if(m_blkCache.size() > CACHE_SLICE_MAX) {
                    free(m_blkCache.begin()->second);
                    m_blkCache.erase(m_blkCache.begin());
                }
                m_blkCache[blk] = ptr;
                return ptr;
            }
            free(ptr);
            return NULL;
        }
        g_log.e("getBlock can't malloc(ALD_BLOCK)\n");
        return NULL;
    } else {
       return iter->second;
    }
}

void AldictDocument::writeToXml(const std::string& path)
{

}

bool AldictDocument::support(const string& dictname)
{
#ifdef WIN32
   wchar_t *wdictname = CharUtil::utf8srtowcs(dictname.c_str());
   if (wdictname != NULL)
       m_dictFile = _wfopen(wdictname, L"rb"); 
#else
   m_dictFile = fopen(dictname.c_str(),"rb");
#endif
	if (m_dictFile == NULL) {
	    g_log.e("Can't open dict file:(%s)", dictname.c_str());
		return false;
	}

	if (readHeader())
	    return true;
    else
        return false;
}
