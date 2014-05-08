#include "Aldict.h"

#include <sstream>

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

iDictItem Aldict::lookup(const string& word)
{
	iDictItem i;
    if (m_bLoad) {
	    struct aldict_dataitem d;
        address_t loc = m_doc.lookup(word, &d);
        i.addr = loc;
        printf("Aldict:lookup (%s), %u\n", word.c_str(), loc);
        if (loc != ALD_INVALID_ADDR) {
            if (d.len_phon > 0) {
                i.phonetic = string((const char*)d.ptr_phon, (int)d.len_phon);
                free(d.ptr_phon);
            }
            if (d.len_expl > 0) {
                i.expl = string((const char*)d.ptr_expl, (int)d.len_expl);
                /*printf("Aldict:lookup result: len_word: (%d,%s)\n",
                       d.len_expl, d.ptr_expl);*/
            
                free(d.ptr_expl);
            }
	        free(d.ptr_word);
        }
    }
	return i;
}

string Aldict::identifier()
{
    return "aldict";
}

int Aldict::getIndexList(IndexList& indexList, int start, int end)
{
    if (m_bLoad)
        return m_doc.getIndexList(indexList, start, end);
	else
	    return 0;
}

iDictItem Aldict::onClick(int index, iIndexItem* item)
{
    struct aldict_dataitem d = m_doc.dataitem(item->addr);
    printf("Aldict:onClick (%d, %u)\n", index, item->addr);
    iDictItem i;
    i.addr = item->addr;
    if (d.len_phon > 0) {
        i.phonetic = string((const char*)d.ptr_phon, (int)d.len_phon);
        free(d.ptr_phon);
    }
    if (d.len_expl > 0) {
        i.expl = string((const char*)d.ptr_expl, (int)d.len_expl);
        /*printf("Aldict:lookup result: len_word: (%d,%s)\n",
               d.len_expl, d.ptr_expl);*/
        free(d.ptr_expl);
    }

	free(d.ptr_word);
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
    printf("{getLanguage} (%s, %s)\n", s.c_str(), d.c_str());    
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
    text.append((char *)(m_doc.m_header.d_identi));
    text.append("\n");
//    text.append("version:");
//    text.append((char *)(m_doc.m_header.d_version));
    text.append("\n");
//    text.append("publisher:");
//    text.append((char *)(m_doc.m_header.p_identi));
    //text.append("publish date:");
    //text.append((char *)(m_doc.m_header.p_date));
    text.append("entry:");
    u32 entries = ald_read_u32(m_doc.m_header.d_entries);
	stringstream stream;
	stream << entries;
	text.append(stream.str());
}
