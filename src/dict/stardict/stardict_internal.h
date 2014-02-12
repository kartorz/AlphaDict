#ifndef _STARTDIC_INTERNAL_H_
#define _STARTDIC_INTERNAL_H_

struct stardict_header {
	std::string version;
	address_t total_words;
	std::string sametypesequence;
	std::string name;
	std::string author;
};

struct stardict_wordsidx {
	std::string words;
	address_t offset;
	address_t size;
};


#endif
