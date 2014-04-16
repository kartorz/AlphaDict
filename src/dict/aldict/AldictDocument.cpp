#include "AldictDocument.h"
#include "tinyxml2/tinyxml2.h"
#include "Log.h"
#include "Util.h"

#define CACHE_SLICE_MAX  400  /* 100k */

AldictDocument::AldictDocument() 
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
}

bool AldictDocument::loadDict(const std::string& dictpath)
{
    m_dictFile = fopen(dictpath.c_str(),"rb");
	if (m_dictFile == NULL) {
	    g_log.e("Can't open dict file:(%s)", dictpath.c_str());
		return false;
	}

	g_log.i("read meta data of Alpha Dict\n");
	readHeader();
	readChrIndex();	
	return true;
}

void AldictDocument::readHeader()
{
	ReadFile read;
	read(m_dictFile, &m_header, sizeof(struct aldict_header));
	m_chrIndexLoc = ald_read_u32(m_header.loc_chrindex);
	m_strIndexLoc = ald_read_u32(m_header.loc_strindex);
	m_dataLoc = ald_read_u32(m_header.loc_data);
}

void AldictDocument::readChrIndex()
{
	ReadFile read;
	Malloc maclloc_t;
	int len = (m_strIndexLoc - m_chrIndexLoc)*ALD_BLOCK;
	void* chrblock = maclloc_t(len);//malloc(len);//maclloc_t(len);

	fseek(m_dictFile, (m_chrIndexLoc-1)*ALD_BLOCK, SEEK_SET);
	read(m_dictFile, chrblock, len);

	struct aldict_charindex rootIndex;
	//memcpy((void *)(&rootIndex), chrblock, sizeof(struct aldict_charindex));
	rootIndex = *((struct aldict_charindex*)chrblock);

	m_indexTree = new kary_tree<aldict_charindex>(rootIndex);
	loadIndexTree(m_indexTree->root(), chrblock);
}

void AldictDocument::loadIndexTree(
		tree_node<aldict_charindex>::treeNodePtr parent, void *chrblock)
{		
	struct aldict_charindex& chrInx = parent->value();
	address_t loc = ald_read_u32(chrInx.location);
	int len = ald_read_u16(chrInx.len_content);
	g_log.d("loadIndexTree loc:(%u-->0x%x), len:(%d)\n", loc, loc, len);
	//printf("loadIndexTree loc:(%u-->0x%x), len:(%d)\n", loc, loc, len);
	if ((loc & 0x80000000) == 0 && len > 0) { /* non-leaf */
		LOOP(len) {
			//struct aldict_charindex chrInx = *(
		   // 	(struct aldict_charindex*)((u8 *)chrblock + loc + i*sizeof(struct aldict_charindex)));
		    struct aldict_charindex chrInx;
			memcpy(&chrInx,
				   (u8 *)chrblock + loc + i*sizeof(struct aldict_charindex),
				   sizeof(struct aldict_charindex));
			parent->insert(chrInx);
			loadIndexTree((*parent)[i], chrblock);
		}
	}
}

address_t AldictDocument::lookup(const string& word, struct aldict_dataitem* item)
{	
	wchar_t *wstr = Util::mbstowcs(word.c_str());
	address_t loc = lookup(wstr, m_indexTree->root());
	g_log.d("AldictDocument::lookup word:(%d)-->(%x) \n", loc, loc);
	free(wstr);
    *item = dataitem(loc);
    return loc;
}

/* Look up in char index tree */
address_t AldictDocument::lookup(wchar_t *wstr, tree_node<aldict_charindex>::treeNodePtr parent)
{
	const wchar_t key = wstr[0];
	g_log.d("AldictDocument::lookup key:(%u)-->(%x) \n", key, key);
	printf("AldictDocument::lookup key:(%u)-->(%x) \n", key, key);
	for (int i=0; i<parent->children().size(); i++) {
		struct aldict_charindex chrInx = parent->child(i)->value();
		wchar_t chr = ald_read_u32(chrInx.wchr);
		if (chr == key) {
			if (wcslen(wstr) > 1) {
				wstr += 1; // next wchar
				if (parent->child(i)->children().size() > 0) {
					return lookup(wstr, (*parent)[i]);
			    } else {
					address_t loc = ald_read_u32(chrInx.location);
					int len = ald_read_u16(chrInx.len_content);
					if (loc & F_LOCSTRINX == F_LOCSTRINX) {
					    return lookup(wstr, loc & (~F_LOCSTRINX), len);
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
				    if (loc & F_LOCSTRINX == 0) {
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
address_t AldictDocument::lookup(wchar_t* key, address_t off, int len)
{
	int bk_off = off/ALD_BLOCK;
	off = off%ALD_BLOCK;
    u8 *buf = (u8 *)getBlock(m_strIndexLoc+bk_off) + off;
	int len_key = wcslen(key);
	for (int item = 0; item < len; item++) {
		aldict_stringindex *pStrInx = (aldict_stringindex *) buf;
		if (pStrInx->len_str[0] == 0) {
			// Read next block
			int n_bnr = ALD_BLOCK_NR(ftello(m_dictFile)) + 1;            
			buf = (u8 *)getBlock(n_bnr - 1);
			pStrInx = (aldict_stringindex *) buf;
		}

		if (len_key == pStrInx->len_str[0]) {
		    wchar_t *wstr = (wchar_t*)(pStrInx->str);
			bool found = true;
			// compasion from tail to head.
		    for (int i=len_key; i>0; i--) {
		    	wchar_t tail_wchr = ald_read_u32((u8 *)(wstr+i-1));
				wchar_t tail_key = key[i-1];
				if (tail_wchr != tail_key) {
				    found = false;
					break;
				}
			}
			// found.
			if (found == true) {
				return ald_read_u32(pStrInx->location);
			}
	    }		
		buf += 5 + 4*pStrInx->len_str[0]; // wchar_t
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

IndexList* AldictDocument::getIndexList()
{
    wchar_t index[256*4];
	memset(index, 0, 256*4);
    m_indexList.clear();
    loadIndex(index, 0, m_indexTree->root());
    return &m_indexList;
}

void AldictDocument::loadIndex(wchar_t *str, int inx,
							   tree_node<aldict_charindex>::treeNodePtr parent)
{
    int children_size = parent->children().size();
	if (children_size > 0) {
	    for (int i=0; i<children_size; i++) {
	        struct aldict_charindex chrInx = parent->child(i)->value();
	    	str[inx] = ald_read_u32(chrInx.wchr);
	    	loadIndex(str, inx+1,  parent->child(i));
        }
	} else {
	    struct aldict_charindex chrInx = parent->value();
		address_t loc = ald_read_u32(chrInx.location);
		int len = ald_read_u16(chrInx.len_content);
		if ((loc & F_LOCSTRINX) == 0) {
            iIndexItem* item = new iIndexItem();
            item->index = (wchar_t*)malloc(4*inx);
            wcsncpy(item->index,  str, inx);
            item->inxlen = inx;
            item->addr = loc;
            m_indexList.push_back(item);
		} else {
            loc = loc & (~F_LOCSTRINX);
            int bk_off = loc/ALD_BLOCK;
            int off = loc%ALD_BLOCK;
	        u8 *buf = (u8 *)getBlock(m_strIndexLoc + bk_off) + off;
            
			for (int item = 0; item < len; item++) {
		        aldict_stringindex *pStrInx = (aldict_stringindex *) buf;
		        if (pStrInx->len_str[0] == 0) {
			        // Read next block
                    int c_bnr = ALD_BLOCK_NR(ftello(m_dictFile));
			        int n_bnr = c_bnr + 1;
                    buf = (u8 *)getBlock(n_bnr - 1);
			        pStrInx = (aldict_stringindex *) buf;
		        }

				wchar_t *wstr = (wchar_t*)(pStrInx->str);
				wcsncpy(str+inx, (wchar_t*)(pStrInx->str), pStrInx->len_str[0]);
                inx += pStrInx->len_str[0];
                {
                    iIndexItem* item = new iIndexItem();
                    item->index = (wchar_t*)malloc(4*inx);
                    wcsncpy(item->index,  str, inx);
                    item->inxlen = inx;
                    item->addr = ald_read_u32(pStrInx->location);
                    m_indexList.push_back(item);
                }
                buf += 5 + 4*pStrInx->len_str[0]; // wchar_t
			}
		}
	}
}

void* AldictDocument::getBlock(int blk)
{
    map<int, void*>::iterator iter = m_cache.find(blk);
    if(iter != m_cache.end())
        return iter->second;
    else {
        ReadFile read;
        void *ptr = malloc(ALD_BLOCK);
        fseek(m_dictFile, (blk-1)*ALD_BLOCK, SEEK_SET);
        read(m_dictFile, ptr, ALD_BLOCK);

        if(m_cache.size() > CACHE_SLICE_MAX) {
            free(m_cache.end()->second);
            m_cache.erase(m_cache.end());
        }
        m_cache[blk] = ptr;
        return ptr;
    }
}

void AldictDocument::writeToXml(const std::string& path)
{
	
}

