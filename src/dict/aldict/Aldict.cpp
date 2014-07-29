# ifdef WIN32
#include <Windows.h>
# endif

#include <sstream>
#include "Aldict.h"

Aldict::Aldict():m_bLoad(false)
{
}

Aldict::Aldict(const string& dictname):m_bLoad(false)
{
	m_doc.loadDict(dictname);
}

bool Aldict::load(const string& dictname)
{
    m_bLoad = m_doc.loadDict(dictname);
    return m_bLoad;
}

bool Aldict::lookup(const string& word, DictItemList& itemList)
{
    MutexLock lock(m_cs);

    if (m_bLoad && !word.empty()) {
        if (lookupCache(word, itemList))
            return true;

        vector<struct aldict_dataitem> retItems;
        IndexList candidate;
        if (m_doc.lookup(word, retItems, candidate)) {
            for (int i=0; i<retItems.size(); i++) {
                iDictItem item;
                item.bfind = true;
                struct aldict_dataitem& d = retItems[i];
                //printf("Aldict:lookup (%s)\n", word.c_str());
                if (d.len_phon > 0) {
                    item.phonetic = string((const char*)d.ptr_phon, (int)d.len_phon);
                    free(d.ptr_phon);
                }
                if (d.len_expl > 0) {
                    item.expl = string((const char*)d.ptr_expl, (int)d.len_expl);
                    //printf("Aldict:lookup result: len_word: (%d,%s)\n",d.len_expl, d.ptr_expl);
                    free(d.ptr_expl);
                }
    	        free(d.ptr_word);

                itemList.push_back(item);
            }
            addToDictCache(word, itemList);
            return true;
        }else {
            iDictItem item;
            item.expl = "Do you want to lookup:\n\n";
            for (int i=0; i<candidate.size(); i++) {
                item.expl +=  candidate[i]->index;
                item.expl += '\n';

                addToIndexCache(candidate[i]->index, *candidate[i]);
            }
            itemList.push_back(item);            
            return false;
        }
    }
    iDictItem i;
    itemList.push_back(i);
	return false;
}

string Aldict::identifier()
{
    return "aldict";
}

int Aldict::getIndexList(IndexList& indexList, int start, int end, const string& startwith)
{
    if (m_bLoad) {
        int size = m_doc.getIndexList(indexList, start, end, startwith);
        return size > 0 ? size : 0;
    }
	return 0;
}

iDictItem Aldict::onClick(int row, iIndexItem* item)
{
    struct aldict_dataitem d = m_doc.dataitem(item->addr);
    iDictItem i;
    dataItemTodictItem(d, i);
    return i;
}

bool Aldict::support(const string& dictname)
{
    return m_doc.support(dictname);
}

void Aldict::getLanguage(string& from, string& to)
{
    string s((char *)(m_doc.m_header.src_lan));
    string d((char *)(m_doc.m_header.det_lan));
    
    from = s;
    to = d;
    //printf("{getLanguage} (%s, %s)\n", s.c_str(), d.c_str());    
}

int Aldict::indexListSize()
{
    int entries = ald_read_u32(m_doc.m_header.d_entries);
    if (entries == 0) {
        IndexList indexList;
        int ret = m_doc.getIndexList(indexList, 0x7FFFFFFF, 0x7FFFFFFF);
	    entries = 0x7FFFFFFF + ret;
    }
    return entries;
}

void Aldict::summary(string& text)
{
	stringstream stream;

    stream << (char *)(m_doc.m_header.d_identi);
    stream << "\n";

    stream <<  "version:    ";
    stream <<  ((int) m_doc.m_header.d_version[0]);
    stream << ".";
    stream <<  ((int) m_doc.m_header.d_version[1]);   
    stream << "\n";

    stream << "from \"";
    stream <<  ((char *) m_doc.m_header.src_lan);
    stream << "\"   to   \"";
    stream <<  ((char *) m_doc.m_header.det_lan);
    stream << "\"\n";

    stream << (char *)(m_doc.m_header.p_identi);
    stream << "\n";

    stream << ald_read_u16(m_doc.m_header.p_date);
    stream << "-";
    stream << ((int) m_doc.m_header.p_date[2]);
    stream << "-";
    stream << ((int) m_doc.m_header.p_date[3]);
    stream << "\n";

    stream << "entry:    ";
    stream << ald_read_u32(m_doc.m_header.d_entries);
	text.append(stream.str());
}

bool Aldict::lookupCache(const string& key, DictItemList& itemList)
{
    map<string, DictItemList>::iterator iter = m_dictCache.find(key);
    if(iter == m_dictCache.end()) {
        if ((m_doc.m_header.flags[0] & F_DUPLICATEINX) == 0) {
            map<string, iIndexItem>::iterator iter2 = m_indexCache.find(key);
            if (iter2 == m_indexCache.end())
                return false;
            iIndexItem indexItem = iter2->second;
            iDictItem i = onClick(-1, &indexItem);
            itemList.push_back(i);
            addToDictCache(key, itemList);
            return true;
        }
        return false;
    }

    itemList = iter->second;
    return true;
}

void Aldict::addToDictCache(const string& key, const DictItemList& itemList)
{
    #define DCACHE_SIZE_MAX 10
    map<string, DictItemList>::iterator iter = m_dictCache.find(key);
    if(iter == m_dictCache.end()) {
        if(m_dictCache.size() > DCACHE_SIZE_MAX) {
            m_dictCache.erase(m_dictCache.begin());
        }
        m_dictCache[key] = itemList;
    }
}

void Aldict::addToIndexCache(const string key, const iIndexItem& item)
{
    #define ICACHE_SIZE_MAX 20
    if ((m_doc.m_header.flags[0] & F_DUPLICATEINX) == 0) {
        map<string, iIndexItem>::iterator iter = m_indexCache.find(key);
        if (iter == m_indexCache.end()) {
            if(m_indexCache.size() > ICACHE_SIZE_MAX) {
                m_indexCache.erase(m_indexCache.begin());
            }
            m_indexCache[key] = item;
        }
    } // else if : There may be duplicate indexs, should lookup again. 
}

void Aldict::dataItemTodictItem(const struct aldict_dataitem& d, iDictItem& i)
{
    if (d.len_phon > 0) {
        i.phonetic = string((const char*)d.ptr_phon, (int)d.len_phon);
        free(d.ptr_phon);
    }
    if (d.len_expl > 0) {
        i.expl = string((const char*)d.ptr_expl, (int)d.len_expl);
        free(d.ptr_expl);
    }
	free(d.ptr_word);
}
