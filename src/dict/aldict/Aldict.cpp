#include "Aldict.h"

Aldict::Aldict(const string& dictpath)
{
	m_doc.loadDict(dictpath);
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
    i.identify = "AlphaDict";

	printf("Aldict:lookup result: len_word:(%d:%s), (%d:%s), (%d,%s)\n",
		   d.len_word, d.ptr_word,
		   d.len_phon, d.ptr_phon,
		   d.len_expl, d.ptr_expl);
	free(d.ptr_word);
	free(d.ptr_phon);
	free(d.ptr_expl);
	return i;
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
