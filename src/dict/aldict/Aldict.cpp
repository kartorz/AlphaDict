#include "Aldict.h"

Aldict::Aldict(const string& dictname)
{
	m_doc.loadDict(dictname);
}

bool Aldict::load(const string& dictname)
{
    return m_doc.loadDict(dictname);
}

iDictItem Aldict::lookup(const string& word)
{
	iDictItem i;
	struct aldict_dataitem d;
    address_t loc = m_doc.lookup(word, &d);
    printf("Aldict:lookup (%s)\n", word.c_str());

    i.phonetic = string((const char*)d.ptr_phon, (int)d.len_phon);
    i.expl = string((const char*)d.ptr_expl, (int)d.len_expl);
    i.addr = loc;

	printf("Aldict:lookup result: len_word:(%d:%s), (%d:%s), (%d,%s)\n",
		   d.len_word, d.ptr_word,
		   d.len_phon, d.ptr_phon,
		   d.len_expl, d.ptr_expl);
	free(d.ptr_word);
	free(d.ptr_phon);
	free(d.ptr_expl);
	return i;
}

const string Aldict::identifier()
{
    return "aldict";
}

IndexList* Aldict::getIndexList()
{
    return m_doc.getIndexList();    
}

iDictItem Aldict::onClick(int index, iIndexItem* item)
{
    struct aldict_dataitem d = m_doc.dataitem(item->addr);
    printf("Aldict:onClick (%d, %d)\n", index, item->addr);
    iDictItem i("AlphaDict");
    i.phonetic = string((const char*)d.ptr_phon, (int)d.len_phon);
    i.expl = string((const char*)d.ptr_expl, (int)d.len_expl);
    i.addr = item->addr;

	printf("Aldict:lookup result: len_word:(%d:%s), (%d:%s), (%d,%s)\n",
		   d.len_word, d.ptr_word,
		   d.len_phon, d.ptr_phon,
		   d.len_expl, d.ptr_expl);
	free(d.ptr_word);
	free(d.ptr_phon);
	free(d.ptr_expl);
    return i;
}

bool Aldict::support(const string& dictname)
{
    return m_doc.support(dictname);
}

bool Aldict::canLookup(const string& srcLan, const string& detLan)
{
    string s((char *)(m_doc.m_header.src_lan));
    string d((char *)(m_doc.m_header.det_lan));
    printf("{canLookup} (%s, %s) --> (%s, %s)\n", s.c_str(), d.c_str(), srcLan.c_str(), detLan.c_str());
    if (s.compare(srcLan) != 0) {
        printf("false 1\n");
        return false;
    }

    if ((detLan.compare("any") != 0) && (d.compare(detLan) != 0) ) {
        printf("false 2\n");
        return false;
    }
    printf("canLookup return true\n");
    return true;
}
