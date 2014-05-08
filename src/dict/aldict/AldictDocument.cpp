/* For saving memory:
 *     - load char index partly, the others load dynamically into a cache tree 
 *       that should can load a whole subtree at least.
 *     
 * 
 */

#include "AldictDocument.h"
#include "tinyxml2/tinyxml2.h"
#include "Log.h"
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
    for (iter=m_cache.begin(); iter!=m_cache.end(); iter++) {
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
    m_dictFile = fopen(dictpath.c_str(),"rb");
	if (m_dictFile == NULL) {
	    g_log.e("Can't open dict file:(%s)", dictpath.c_str());
		return false;
	}
	g_log.i("read meta data of Alpha Dict\n");
	if (readHeader()) {
	    readChrIndex();	
	    return true;
    }
    return false;
}

bool AldictDocument::readHeader()
{
	ReadFile read;
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
	ReadFile read;
	Malloc maclloc_t;
	address_t len = (m_strIndexLoc - m_chrIndexLoc)*ALD_BLOCK;
	void* chrblock = maclloc_t(len);

    g_log.d("{readChrIndex}, charindex size == (%d)bytes \n", len);
	fseek(m_dictFile, (m_chrIndexLoc-1)*ALD_BLOCK, SEEK_SET);
	read(m_dictFile, chrblock, len);

	struct aldict_charindex rootIndex;
	rootIndex = *((struct aldict_charindex*)chrblock);
    m_indexTree = new kary_tree<aldict_charindex>(rootIndex);
    if (len <= MEM_CHARINX_MAX) {
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

    if (len > 1000) {
        g_log.w("{loadIndexTree} more than 1000 child need to be loaded, someting wrong?\n");
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

address_t AldictDocument::lookup(const string& word, struct aldict_dataitem* item)
{
	address_t loc = lookup((char*)word.c_str(), m_indexTree->root());
	//g_log.d("AldictDocument::lookup word:(%d)-->(%x) \n", loc, loc);
	//free(wstr);
    if (loc != ALD_INVALID_ADDR)
        *item = dataitem(loc);
    return loc;
}

/* Look up in char index tree,
 * If we can't find the 'target',.. 
 */

address_t AldictDocument::lookup(char *strkey, tree_node<aldict_charindex>::treeNodePtr parent)
{
	const wchar_t key = Util::mbrtowc_r(&strkey); /* "strkey" will be modified */

	g_log.d("AldictDocument::lookup key:(%u)-->(%x) \n", key, key);
	for (int i=0; i<parent->children().size(); i++) {
		struct aldict_charindex chrInx = parent->child(i)->value();
		wchar_t chr = ald_read_u32(chrInx.wchr);
		if (chr == key) {
			if (strlen(strkey) > 0) {
				if (parent->child(i)->children().size() > 0) {
					return lookup(strkey, (*parent)[i]);
			    } else {
					address_t loc = ald_read_u32(chrInx.location);
					int len = ald_read_u16(chrInx.len_content);
					if ((loc & F_LOCSTRINX) == F_LOCSTRINX) {
					    return lookup(strkey, loc & (~F_LOCSTRINX), len);
					} else {
						return ALD_INVALID_ADDR;
					}
			    }
			} else {
			    if (parent->child(i)->children().size() > 0) {
				    struct aldict_charindex chrInx = parent->child(i)->child(0)->value();
					if (ald_read_u32(chrInx.wchr) == 0) {
						return ald_read_u32(chrInx.location);
					} else {
						return ALD_INVALID_ADDR;
					}
				} else {
					address_t loc = ald_read_u32(chrInx.location);
				    if ((loc & F_LOCSTRINX) == 0) {
					    return loc; // Find!
					}
					return ALD_INVALID_ADDR;
				}
			}
		}
	}
	return ALD_INVALID_ADDR;
}

/* Look up in string index area */
address_t AldictDocument::lookup(char* strkey, address_t off, int len)
{
	int block_nr = off/ALD_BLOCK + m_strIndexLoc;
	off = off%ALD_BLOCK;
    u8 *buf = (u8 *)getBlock(block_nr) + off; /* Load index has checked if NULL. */

	int len_key = strlen(strkey);
	for (int item = 0; item < len; item++) {
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
				return ald_read_u32(pStrInx->location);
			}
	    }		
		buf += 5 + pStrInx->len_str[0];
	}
	return ALD_INVALID_ADDR;
}

struct aldict_dataitem AldictDocument::dataitem(address_t loc)
{	
    struct aldict_dataitem dataItem;
	memset(&dataItem, 0, sizeof(struct aldict_dataitem));
	if (loc != ALD_INVALID_ADDR) {
		ReadFile read;
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

int AldictDocument::getIndexList(IndexList& indexList, int start, int end)
{
    if (!m_indexTree)
        return 0;

    wchar_t index[INDEXARRY_LEN_MAX];
	memset(index, L'\0', INDEXARRY_LEN_MAX);
    m_indexStart = start;
    m_indexEnd = end;
    m_indexNumber = 0; /* start from 0 */

    loadIndex(index, 0, indexList, m_indexTree->root());
    return m_indexNumber - m_indexStart;
}

/* @return : true -> continue; false -> stop */
bool AldictDocument::loadIndex(wchar_t *str, int inx, IndexList& indexList,
                               tree_node<aldict_charindex>::treeNodePtr parent)
{
    int children_size = parent->children().size();
	if (children_size > 0) {
	    for (int i=0; i<children_size; i++) {
	        struct aldict_charindex chrInx = parent->child(i)->value();           
            if (inx < INDEXARRY_LEN_MAX -1) {
                str[inx] = ald_read_u32(chrInx.wchr);
                if (loadIndex(str, inx+1, indexList, parent->child(i)) == false)
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
        char* pinx = Util::wcsrtombs_r(str);
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
        if (m_indexNumber >= m_indexStart) {
            if (m_indexNumber < m_indexEnd) {
                iIndexItem* item = new iIndexItem();
                item->index = strparent;
                item->addr = loc;
                indexList.push_back(item);
            } else {
                return false;
            }
        }
        ++m_indexNumber;
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
            if (m_indexNumber >= m_indexStart) {
                if (m_indexNumber < m_indexEnd) {
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
            ++m_indexNumber;
            buf_start += 5 + pStrInx->len_str[0];
		}
	}
    return true;
}

void* AldictDocument::getBlock(int blk)
{
    map<int, void*>::iterator iter = m_cache.find(blk);
    if(iter == m_cache.end()) {
        ReadFile read;
        void *ptr = malloc(ALD_BLOCK);
        memset(ptr, 0, ALD_BLOCK);
        if (ptr != NULL) {
            if (fseek(m_dictFile, (blk-1)*ALD_BLOCK, SEEK_SET) == 0) {
                read(m_dictFile, ptr, ALD_BLOCK);
                if(m_cache.size() > CACHE_SLICE_MAX) {
                    free(m_cache.begin()->second);
                    m_cache.erase(m_cache.begin());
                }
                m_cache[blk] = ptr;
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
   m_dictFile = fopen(dictname.c_str(),"rb");
	if (m_dictFile == NULL) {
	    g_log.e("Can't open dict file:(%s)", dictname.c_str());
		return false;
	}

	if (readHeader())
	    return true;
    else
        return false;
}
