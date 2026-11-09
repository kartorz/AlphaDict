/*
 * aldict - build AlphaDict dictionaries from plain text, export them back.
 *
 *   aldict build  [-o out.aldict] file.txt [more.txt ...]
 *   aldict export [-o out.txt]    dict.aldict
 *   aldict info                   dict.aldict
 *
 * The text format is described in doc/front-end_format.txt.  The older XML
 * front-end is still accepted by "build" so the scripts in src/tools/ keep
 * working.
 *
 * See doc/dictionary_inner.pdf for the binary format.
 */

#ifdef WIN32
#include <Windows.h>
#include <mbctype.h>
#endif
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#ifdef WIN32
#include "win32/pgetopt.h"
#endif
#include "type.h"
#include "alphadict.h"
#include "aldict/aldict_inner.h"
#include "dict/aldict/AldictDocument.h"
#include "tinyxml2/tinyxml2.h"
#include "Util.h"
#include "CharUtil.h"
#include "Log.h"

using namespace std;
using namespace tinyxml2;

#define VERSION     "2.0"

#define INDEX_BLOCK_NR   2
#define STRINX_WORDS_MAX 10
#define STRINX_DEPTH_MAX 5
#define CHRINX_DEPTH_MIN 1
#define DICT_SUFFIX      ".aldict"
#define STRINX_LEN_MAX   60  /* a string index must be within a block. */

/* Limits imposed by the binary format: the length of a word and of the
 * phonetic string is stored in one byte, the explanation in two. */
#define WORD_LEN_MAX  255
#define PHON_LEN_MAX  255
#define EXPL_LEN_MAX  65535

#define WORD_ATTR  "word"


struct Header {
	string name;
	string publisher;
	string date;         /* YYYY-MM-DD */
	string format;       /* format version, one byte in the header */
	string version;      /* dictionary data version, e.g. "2.6" */
	string srclang;
	string dstlang;
};

struct Entry {
	string word;
	vector<string> phonetic;   /* one line each */
	vector<string> alias;
	string expl;               /* may span lines */
};

struct Dictionary {
	Header header;
	vector<Entry> entries;
};

static int total_chrindex = 0; /* Used in recursion funciton write_charindex */
static int total_entry = 0;
static bool duplicate_index_flag = false;

static void add_to_indextree(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			     char *str, off_t pos);
static void trim_indextree(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			   int depth, FILE* sinxfile);
static void write_charindex(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			    FILE* cinxfile);
static void merge_file(FILE *det, FILE *src);
static off_t check_block_bound(off_t pos, int nbytes);

static void usage()
{
	printf("Usage: aldict COMMAND [options]\n"
	       "\n"
	       "  build  [-o out.aldict] file.txt [more.txt ...]\n"
	       "         make a dictionary from text files (XML is also accepted)\n"
	       "  export [-o out.txt]    dict.aldict\n"
	       "         dump a dictionary as editable text\n"
	       "  info                   dict.aldict\n"
	       "         show what is inside a dictionary\n"
	       "\n"
	       "Text format: doc/front-end_format.txt\n");
}

static string trim(const string& s)
{
	size_t b = s.find_first_not_of(" \t\r\n");
	if (b == string::npos)
		return "";
	size_t e = s.find_last_not_of(" \t\r\n");
	return s.substr(b, e - b + 1);
}

static string tolower(const string& s)
{
	string r = s;
	for (size_t i = 0; i < r.size(); i++)
		r[i] = (char)tolower((unsigned char)r[i]);
	return r;
}

static bool ends_with(const string& s, const string& suffix)
{
	return s.size() >= suffix.size() &&
	       s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static string base_name(const string& path)
{
	size_t slash = path.find_last_of("/\\");
	string name = slash == string::npos ? path : path.substr(slash + 1);
	size_t dot = name.find_last_of('.');
	return dot == string::npos ? name : name.substr(0, dot);
}

/* Cut a UTF-8 string at a character boundary, so truncation never leaves a
 * partial multi-byte sequence in the dictionary. */
static void utf8_truncate(string& s, size_t maxbytes)
{
	if (s.size() <= maxbytes)
		return;
	size_t n = maxbytes;
	while (n > 0 && ((unsigned char)s[n] & 0xC0) == 0x80)
		--n;
	s.resize(n);
}

static bool read_lines(const string& path, vector<string>& lines)
{
	FILE* file = fopen(path.c_str(), "rb");
	if (file == NULL) {
		fprintf(stderr, "error: cannot open %s\n", path.c_str());
		return false;
	}

	string data;
	char buf[8192];
	size_t nr;
	while ((nr = fread(buf, 1, sizeof(buf), file)) > 0)
		data.append(buf, nr);
	fclose(file);

	size_t i = 0;
	if (data.size() >= 3 && (unsigned char)data[0] == 0xEF &&
	    (unsigned char)data[1] == 0xBB && (unsigned char)data[2] == 0xBF)
		i = 3; /* UTF-8 BOM */

	string line;
	for (; i < data.size(); i++) {
		if (data[i] == '\n') {
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);
			lines.push_back(line);
			line.clear();
		} else {
			line.push_back(data[i]);
		}
	}
	if (!line.empty()) {
		if (line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		lines.push_back(line);
	}
	return true;
}

static bool is_section(const string& t, string& name)
{
	if (t.size() < 3 || t[0] != '[' || t[t.size() - 1] != ']')
		return false;
	name = tolower(trim(t.substr(1, t.size() - 2)));
	return true;
}

/*
 * Parse the text front-end.
 *
 * [header] holds "key = value" pairs, [entry] starts one entry.  Inside an
 * entry "explanation:" opens a block that runs to the next section marker, so
 * its lines are kept verbatim (they may contain "=", "#" or blank lines).
 */
static bool parse_txt(const string& path, Dictionary& dict)
{
	vector<string> lines;
	if (!read_lines(path, lines))
		return false;

	string sec;
	Entry entry;
	bool haveEntry = false;
	vector<string> explLines;
	bool inExpl = false;

	struct State {
		Dictionary& dict;
		Entry& entry;
		bool& haveEntry;
		vector<string>& explLines;
		bool& inExpl;

		void flush()
		{
			if (!haveEntry)
				return;
			while (!explLines.empty() && trim(explLines.back()).empty())
				explLines.pop_back();
			for (size_t i = 0; i < explLines.size(); i++) {
				entry.expl += explLines[i];
				entry.expl += '\n';
			}
			dict.entries.push_back(entry);
			entry = Entry();
			explLines.clear();
			haveEntry = false;
			inExpl = false;
		}
	} state = { dict, entry, haveEntry, explLines, inExpl };

	for (size_t i = 0; i < lines.size(); i++) {
		string t = trim(lines[i]);

		if (inExpl) {
			string marker;
			if (!is_section(t, marker)) {
				explLines.push_back(lines[i]);
				continue;
			}
			state.flush();
		}

		if (t.empty())
			continue;
		if (t[0] == '#' || t[0] == ';')
			continue;

		string section;
		if (is_section(t, section)) {
			state.flush();
			if (section != "header" && section != "entry") {
				fprintf(stderr, "warning: unknown section [%s] in %s\n",
					section.c_str(), path.c_str());
			}
			sec = section;
			continue;
		}

		string key, value;
		size_t eq = t.find('=');
		if (eq != string::npos) {
			key = tolower(trim(t.substr(0, eq)));
			value = trim(t.substr(eq + 1));
		} else if (t[t.size() - 1] == ':') {
			key = tolower(trim(t.substr(0, t.size() - 1)));
		} else {
			fprintf(stderr, "warning: ignoring line in %s: %s\n",
				path.c_str(), t.c_str());
			continue;
		}

		if (sec == "header") {
			if (key == "name" || key == "dictname")
				dict.header.name = value;
			else if (key == "publisher")
				dict.header.publisher = value;
			else if (key == "date" || key == "publishdate")
				dict.header.date = value;
			else if (key == "version" || key == "format")
				dict.header.format = value;
			else if (key == "dictversion")
				dict.header.version = value;
			else if (key == "srclang" || key == "srclan")
				dict.header.srclang = value;
			else if (key == "dstlang" || key == "detlan")
				dict.header.dstlang = value;
			else if (key != "entries" && key != "charset")
				fprintf(stderr, "warning: unknown header field '%s' in %s\n",
					key.c_str(), path.c_str());
			continue;
		}

		if (sec != "entry") {
			fprintf(stderr, "warning: '%s' outside a section in %s\n",
				key.c_str(), path.c_str());
			continue;
		}

		if (key == "word") {
			if (haveEntry)
				state.flush();  /* a missing [entry] still starts a new one */
			entry.word = value;
			haveEntry = true;
		} else if (!haveEntry) {
			fprintf(stderr, "warning: '%s' before 'word' in %s\n",
				key.c_str(), path.c_str());
		} else if (key == "phonetic" || key == "pron") {
			entry.phonetic.push_back(value);
		} else if (key == "alias") {
			entry.alias.push_back(value);
		} else if (key == "explanation" || key == "expl" || key == "meaning") {
			if (value.empty()) {
				inExpl = true;
			} else {
				if (!entry.expl.empty())
					entry.expl += '\n';
				entry.expl += value;
			}
		} else {
			fprintf(stderr, "warning: unknown field '%s' in %s (word '%s')\n",
				key.c_str(), path.c_str(), entry.word.c_str());
		}
	}
	state.flush();
	return true;
}

static void xml_header_field(Header& header, const string& key, const string& value)
{
	if (key == "name" || key == "dictname")
		header.name = value;
	else if (key == "publisher")
		header.publisher = value;
	else if (key == "date" || key == "publishdate")
		header.date = value;
	else if (key == "version" || key == "format")
		header.format = value;
	else if (key == "dictversion")
		header.version = value;
	else if (key == "srclang" || key == "srclan")
		header.srclang = value;
	else if (key == "dstlang" || key == "detlan")
		header.dstlang = value;
}

/* Read the old XML front-end (doc/front-end_format.xml). */
static bool parse_xml(const string& path, Dictionary& dict)
{
	XMLDocument doc;
	int err = doc.LoadFile(path.c_str());
	if (err != XML_NO_ERROR) {
		fprintf(stderr, "error: cannot parse %s (code %d)\n", path.c_str(), err);
		return false;
	}

	XMLElement* root = doc.RootElement();
	if (root == NULL) {
		fprintf(stderr, "error: %s is empty\n", path.c_str());
		return false;
	}

	XMLElement* headerElement = root->FirstChildElement("header");
	for (XMLElement* e = headerElement ? headerElement->FirstChildElement() : NULL;
	     e != NULL; e = e->NextSiblingElement()) {
		const char* key = e->Value();
		const char* text = e->GetText();
		if (key && text)
			xml_header_field(dict.header, tolower(key), trim(text));
	}

	XMLElement* wordsElement = root->FirstChildElement("words");
	if (wordsElement == NULL) {
		fprintf(stderr, "error: no <words> in %s\n", path.c_str());
		return false;
	}

	for (XMLElement* w = wordsElement->FirstChildElement(); w != NULL;
	     w = w->NextSiblingElement()) {
		Entry entry;
		const char* word = w->Attribute(WORD_ATTR);
		if (word)
			entry.word = word;
		if (entry.word.empty()) {
			fprintf(stderr, "warning: skipping element without '%s' in %s\n",
				WORD_ATTR, path.c_str());
			continue;
		}

		for (XMLElement* p = w->FirstChildElement("phonetic"); p != NULL;
		     p = p->NextSiblingElement("phonetic")) {
			XMLElement* n = p->FirstChildElement();
			if (n == NULL) {
				if (p->GetText())
					entry.phonetic.push_back(p->GetText());
				continue;
			}
			for (; n != NULL; n = n->NextSiblingElement()) {
				string line = n->Value() ? n->Value() : "";
				if (n->GetText())
					line += n->GetText();
				entry.phonetic.push_back(line);
			}
		}

		for (XMLElement* ex = w->FirstChildElement("explanation"); ex != NULL;
		     ex = ex->NextSiblingElement("explanation")) {
			/* Take every text node, not just the first one: a rich text
			 * explanation is often split into several of them. */
			string text;
			for (XMLNode* n = ex->FirstChild(); n != NULL; n = n->NextSibling()) {
				if (n->ToText())
					text += n->Value();
				else if (n->ToElement() && n->ToElement()->GetText())
					text += n->ToElement()->GetText();
			}
			if (text.empty() && ex->GetText())
				text = ex->GetText();
			if (!text.empty()) {
				if (!entry.expl.empty())
					entry.expl += '\n';
				entry.expl += text;
			}
		}

		for (XMLElement* al = w->FirstChildElement("alias"); al != NULL;
		     al = al->NextSiblingElement("alias")) {
			for (XMLElement* as = al->FirstChildElement(); as != NULL;
			     as = as->NextSiblingElement()) {
				const char* text = as->GetText();
				if (text && *text)
					entry.alias.push_back(text);
			}
		}

		dict.entries.push_back(entry);
	}
	return true;
}

static void copy_field(u8* dst, size_t size, const string& src)
{
	size_t n = src.size();
	if (n > size - 1)
		n = size - 1;  /* the header fields are not NUL terminated otherwise */
	memcpy(dst, src.data(), n);
	dst[n] = '\0';
}

static void fill_header(struct aldict_header& header, const Header& src)
{
	memset(&header, 0, sizeof(struct aldict_header));
	header.magic[0] = ALD_MAGIC_L;
	header.magic[1] = ALD_MAGIC_H;
	header.h_version[0] = src.format.empty() ? '1' : src.format[0];

	int year = 0, month = 0, day = 0;
	if (sscanf(src.date.c_str(), "%d-%d-%d", &year, &month, &day) == 3) {
		header.p_date[0] = (u8)day;
		header.p_date[1] = (u8)month;
		header.p_date[2] = (u8)(year & 0xff);
		header.p_date[3] = (u8)((year >> 8) & 0xff);
	}

	copy_field(header.p_identi, sizeof(header.p_identi), src.publisher);
	copy_field(header.d_identi, sizeof(header.d_identi), src.name);

	/* "2.6" means major 2, minor 6; the struct stores them the other way. */
	int major = 0, minor = 0;
	if (sscanf(src.version.c_str(), "%d.%d", &major, &minor) == 2) {
		header.d_version[0] = (u8)minor;
		header.d_version[1] = (u8)major;
	} else if (sscanf(src.version.c_str(), "%d", &major) == 1) {
		header.d_version[0] = 0;
		header.d_version[1] = (u8)major;
	}

	copy_field(header.src_lan, sizeof(header.src_lan),
		   src.srclang.empty() ? "any" : src.srclang);
	copy_field(header.det_lan, sizeof(header.det_lan),
		   src.dstlang.empty() ? "any" : src.dstlang);
}

static bool write_entry(FILE* file, const Entry& entry)
{
	if (entry.word.size() > WORD_LEN_MAX) {
		fprintf(stderr, "error: word longer than %d bytes: %s\n",
			WORD_LEN_MAX, entry.word.c_str());
		return false;
	}

	u8 len = (u8)entry.word.size();
	fwrite(&len, 1, 1, file);
	if (len > 0)
		fwrite(entry.word.data(), 1, len, file);

	string phonetic;
	for (size_t i = 0; i < entry.phonetic.size(); i++) {
		phonetic += entry.phonetic[i];
		phonetic += '\n';
	}
	if (phonetic.size() > PHON_LEN_MAX) {
		fprintf(stderr, "warning: phonetic too long, cut to %d bytes: %s\n",
			PHON_LEN_MAX, entry.word.c_str());
		utf8_truncate(phonetic, PHON_LEN_MAX);
	}
	len = (u8)phonetic.size();
	fwrite(&len, 1, 1, file);
	if (len > 0)
		fwrite(phonetic.data(), 1, len, file);

	string expl = entry.expl;
	if (expl.size() > EXPL_LEN_MAX) {
		fprintf(stderr, "warning: explanation too long, cut to %d bytes: %s\n",
			EXPL_LEN_MAX, entry.word.c_str());
		utf8_truncate(expl, EXPL_LEN_MAX);
	}
	u8 buf[2];
	ald_write_u16(buf, (u16)expl.size());
	fwrite(buf, 2, 1, file);
	if (!expl.empty())
		fwrite(expl.data(), 1, expl.size(), file);
	return true;
}

static void add_index(ktree::kary_tree2<aldict_charindex>& tree,
		      const string& key, off_t pos)
{
	if (key.empty())
		return;
	/* add_to_indextree() walks the string in place, so hand it a copy. */
	vector<char> buf(key.begin(), key.end());
	buf.push_back('\0');
	add_to_indextree(tree.root(), &buf[0], pos);
}

static bool build_dictionary(const Dictionary& dict, const string& outpath)
{
	FILE* dictfile = fopen(outpath.c_str(), "wb");
	if (dictfile == NULL) {
		fprintf(stderr, "error: cannot create %s\n", outpath.c_str());
		return false;
	}

	/* tmpfile() keeps concurrent conversions apart and cleans up on error. */
	FILE* datafile = tmpfile();
	FILE* strinxfile = tmpfile();
	if (datafile == NULL || strinxfile == NULL) {
		fprintf(stderr, "error: cannot create temporary file\n");
		fclose(dictfile);
		if (datafile)
			fclose(datafile);
		if (strinxfile)
			fclose(strinxfile);
		remove(outpath.c_str());
		return false;
	}

	struct aldict_header header;
	fill_header(header, dict.header);

	struct aldict_charindex charIndex;
	memset(&charIndex, 0, sizeof(struct aldict_charindex));
	ktree::kary_tree2<aldict_charindex> indexTree(charIndex);

	total_entry = 0;
	total_chrindex = 0;
	duplicate_index_flag = false;

	for (size_t i = 0; i < dict.entries.size(); i++) {
		const Entry& entry = dict.entries[i];
		off_t pos = ftello(datafile);
		if (!write_entry(datafile, entry)) {
			fclose(dictfile);
			fclose(datafile);
			fclose(strinxfile);
			remove(outpath.c_str());  /* do not leave a truncated dictionary behind */
			return false;
		}
		add_index(indexTree, entry.word, pos);
		for (size_t j = 0; j < entry.alias.size(); j++)
			add_index(indexTree, entry.alias[j], pos);
	}
	printf("read %lu entries\n", (unsigned long)dict.entries.size());

	/* Strip string index from the tree, save it to a temp file. */
	trim_indextree(indexTree.root(), 0, strinxfile);

	/* Char index area. */
	fseek(dictfile, (INDEX_BLOCK_NR - 1) * ALD_BLOCK, SEEK_SET);
	ktree::tree_node<aldict_charindex>::treeNodePtr rootNode = indexTree.root();
	struct aldict_charindex& rootIndex = rootNode->value();
	ald_write_u32(rootIndex.location, sizeof(struct aldict_charindex));
	ald_write_u16(rootIndex.len_content, rootNode->children().size());
	fwrite(&rootIndex, sizeof(struct aldict_charindex), 1, dictfile);
	total_chrindex = 1;
	write_charindex(rootNode, dictfile);

	/* String index and data areas, each starting on a block boundary. */
	header.loc_chrindex[0] = INDEX_BLOCK_NR;
	ald_write_u32(header.d_entries, total_entry);

	fseek(dictfile, 0, SEEK_END);
	int bnr = ALD_BLOCK_NR(ftello(dictfile)) + 1;
	ald_write_u32(header.loc_strindex, bnr);
	header.flags[0] |= duplicate_index_flag ? F_DUPLICATEINX : 0;

	fseek(dictfile, (bnr - 1) * ALD_BLOCK, SEEK_SET);
	fseek(strinxfile, 0, SEEK_SET);
	merge_file(dictfile, strinxfile);

	bnr = ALD_BLOCK_NR(ftello(dictfile)) + 1;
	ald_write_u32(header.loc_data, bnr);
	fseek(dictfile, (bnr - 1) * ALD_BLOCK, SEEK_SET);
	fseek(datafile, 0, SEEK_SET);
	merge_file(dictfile, datafile);

	fseek(dictfile, 0, SEEK_SET);
	fwrite(&header, sizeof(struct aldict_header), 1, dictfile);

	printf("write %s, entries(%d), char index(%d), costs(%u)s\n",
	       outpath.c_str(), total_entry, total_chrindex, Util::getTimeMS() / 1000);
	fclose(dictfile);
	fclose(strinxfile);
	fclose(datafile);
	return true;
}

static bool load_dictionary(Dictionary& dict, const vector<string>& inputs)
{
	for (size_t i = 0; i < inputs.size(); i++) {
		Dictionary part;
		bool ok = ends_with(tolower(inputs[i]), ".xml")
			  ? parse_xml(inputs[i], part)
			  : parse_txt(inputs[i], part);
		if (!ok)
			return false;
		if (part.entries.empty())
			fprintf(stderr, "warning: no entries in %s\n", inputs[i].c_str());
		if (i == 0)
			dict.header = part.header;
		else if (!part.header.name.empty() &&
			 part.header.name != dict.header.name)
			fprintf(stderr, "warning: %s has dictionary name '%s', ignored\n",
				inputs[i].c_str(), part.header.name.c_str());
		dict.entries.insert(dict.entries.end(),
				    part.entries.begin(), part.entries.end());
	}
	return true;
}

static void format_date(const u8* p_date, string& date)
{
	char buf[32];
	int year = p_date[2] | (p_date[3] << 8);
	int month = p_date[1];
	int day = p_date[0];
	if (year == 0 && month == 0 && day == 0) {
		date.clear();
		return;
	}
	snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, day);
	date = buf;
}

static bool export_dictionary(const string& dictpath, const string& txtpath)
{
	AldictDocument doc;
	if (!doc.loadDict(dictpath)) {
		fprintf(stderr, "error: %s is not a readable dictionary\n", dictpath.c_str());
		return false;
	}

	IndexList indexes;
	doc.getIndexList(indexes, 0, INT_MAX);

	/* Aliases carry the address of their entry, so group by address: the
	 * first index string of a group is the word itself, the rest are aliases. */
	map<address_t, vector<string> > groups;
	vector<address_t> order;
	for (size_t i = 0; i < indexes.size(); i++) {
		address_t addr = indexes[i]->addr;
		if (groups.find(addr) == groups.end())
			order.push_back(addr);
		groups[addr].push_back(indexes[i]->index);
	}
	for (size_t i = 0; i < indexes.size(); i++)
		delete indexes[i];

	FILE* file = fopen(txtpath.c_str(), "wb");
	if (file == NULL) {
		fprintf(stderr, "error: cannot create %s\n", txtpath.c_str());
		return false;
	}

	struct aldict_header& h = doc.m_header;
	string date;
	format_date(h.p_date, date);

	fprintf(file, "# AlphaDict dictionary text, see doc/front-end_format.txt\n");
	fprintf(file, "\n[header]\n");
	if (h.d_identi[0])
		fprintf(file, "name = %s\n", (char*)h.d_identi);
	if (h.p_identi[0])
		fprintf(file, "publisher = %s\n", (char*)h.p_identi);
	if (!date.empty())
		fprintf(file, "date = %s\n", date.c_str());
	if (h.h_version[0])
		fprintf(file, "version = %c\n", h.h_version[0]);
	fprintf(file, "dictversion = %d.%d\n", h.d_version[1], h.d_version[0]);
	fprintf(file, "srclang = %s\n", (char*)h.src_lan);
	fprintf(file, "dstlang = %s\n", (char*)h.det_lan);
	fprintf(file, "entries = %lu\n", (unsigned long)order.size());

	for (size_t i = 0; i < order.size(); i++) {
		address_t addr = order[i];
		struct aldict_dataitem item = doc.dataitem(addr);

		string word = item.len_word
			      ? string((const char*)item.ptr_word, item.len_word) : "";
		string phonetic = item.len_phon
			      ? string((const char*)item.ptr_phon, item.len_phon) : "";
		string expl = item.len_expl
			      ? string((const char*)item.ptr_expl, item.len_expl) : "";
		if (item.ptr_word)
			free(item.ptr_word);
		if (item.ptr_phon)
			free(item.ptr_phon);
		if (item.ptr_expl)
			free(item.ptr_expl);

		/* Some dictionaries keep the word only in the index area. */
		if (word.empty() && !groups[addr].empty())
			word = groups[addr][0];

		fprintf(file, "\n[entry]\nword = %s\n", word.c_str());

		size_t start = 0;
		while (start <= phonetic.size()) {
			size_t end = phonetic.find('\n', start);
			if (end == string::npos)
				end = phonetic.size();
			string line = phonetic.substr(start, end - start);
			if (!trim(line).empty())
				fprintf(file, "phonetic = %s\n", line.c_str());
			if (end == phonetic.size())
				break;
			start = end + 1;
		}

		const vector<string>& names = groups[addr];
		for (size_t j = 0; j < names.size(); j++) {
			if (names[j] == word)
				continue;
			bool seen = false;
			for (size_t k = 0; k < j && !seen; k++)
				seen = names[k] == names[j];
			if (!seen)
				fprintf(file, "alias = %s\n", names[j].c_str());
		}

		if (!expl.empty()) {
			fprintf(file, "explanation:\n");
			fwrite(expl.data(), 1, expl.size(), file);
			if (expl[expl.size() - 1] != '\n')
				fputc('\n', file);
		}
	}

	fclose(file);
	printf("write %s, entries(%lu)\n", txtpath.c_str(), (unsigned long)order.size());
	return true;
}

static bool print_info(const string& dictpath)
{
	AldictDocument doc;
	if (!doc.loadDict(dictpath)) {
		fprintf(stderr, "error: %s is not a readable dictionary\n", dictpath.c_str());
		return false;
	}

	struct aldict_header& h = doc.m_header;
	string date;
	format_date(h.p_date, date);

	IndexList indexes;
	doc.getIndexList(indexes, 0, INT_MAX);
	map<address_t, int> seen;
	for (size_t i = 0; i < indexes.size(); i++)
		seen[indexes[i]->addr] = 1;
	for (size_t i = 0; i < indexes.size(); i++)
		delete indexes[i];

	printf("file       : %s\n", dictpath.c_str());
	printf("name       : %s\n", (char*)h.d_identi);
	printf("publisher  : %s\n", (char*)h.p_identi);
	if (!date.empty())
		printf("date       : %s\n", date.c_str());
	printf("language   : %s -> %s\n", (char*)h.src_lan, (char*)h.det_lan);
	printf("version    : %d.%d (format %c)\n", h.d_version[1], h.d_version[0],
	       h.h_version[0] ? h.h_version[0] : '?');
	printf("entries    : %lu (index %lu)\n",
	       (unsigned long)seen.size(), (unsigned long)indexes.size());
	return true;
}

int main(int argc, char* argv[])
{
#ifdef _LINUX
	setlocale(LC_ALL, "C.UTF-8");
#endif
	Util::getTimeMS();

	if (argc < 2) {
		usage();
		return 2;
	}

	string cmd = argv[1];
	if (cmd == "-h" || cmd == "--help" || cmd == "help") {
		usage();
		return 0;
	}
	if (cmd == "-v" || cmd == "--version" || cmd == "version") {
		printf("aldict %s\n", VERSION);
		return 0;
	}

	string outpath;
	vector<string> inputs;
	for (int i = 2; i < argc; i++) {
		string arg = argv[i];
		if (arg == "-o" || arg == "--output") {
			if (i + 1 >= argc) {
				fprintf(stderr, "error: %s needs a path\n", arg.c_str());
				return 2;
			}
			outpath = argv[++i];
		} else if (arg == "-h" || arg == "--help") {
			usage();
			return 0;
		} else if (arg.size() > 1 && arg[0] == '-') {
			fprintf(stderr, "error: unknown option %s\n", arg.c_str());
			return 2;
		} else {
			inputs.push_back(arg);
		}
	}

	if (cmd == "build" || cmd == "export" || cmd == "info") {
		if (inputs.empty()) {
			fprintf(stderr, "error: %s needs an input file\n", cmd.c_str());
			return 2;
		}
	}

	if (cmd == "build") {
		if (outpath.empty())
			outpath = base_name(inputs[0]) + DICT_SUFFIX;

		Dictionary dict;
		if (!load_dictionary(dict, inputs))
			return 1;
		return build_dictionary(dict, outpath) ? 0 : 1;
	}

	if (cmd == "export") {
		if (inputs.size() != 1) {
			fprintf(stderr, "error: export takes one dictionary\n");
			return 2;
		}
		if (outpath.empty())
			outpath = base_name(inputs[0]) + ".txt";
		string dictpath = inputs[0];
		if (!ends_with(tolower(dictpath), DICT_SUFFIX))
			dictpath += DICT_SUFFIX;
		return export_dictionary(dictpath, outpath) ? 0 : 1;
	}

	if (cmd == "info") {
		if (inputs.size() != 1) {
			fprintf(stderr, "error: info takes one dictionary\n");
			return 2;
		}
		return print_info(inputs[0]) ? 0 : 1;
	}

	fprintf(stderr, "error: unknown command '%s'\n", cmd.c_str());
	usage();
	return 2;
}

static int bsearch(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
                   u32 key, int min, int max)
{
	int mid = (min + max) / 2;
	u32 chr = ald_read_u32(parent->child(mid)->value().wchr);
	if (min >= max)
		return min;
	if (chr == key)
		return mid;
	if (chr < key)
		return bsearch(parent, key, mid + 1, max);

	if (mid > min)
		return bsearch(parent, key, min, mid - 1);
	else
		return min;
}

static void add_to_indextree(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
                             char *str, off_t d_off)
{
	const u32 key = CharUtil::utf8byteToUCS4Char((const char**)&str); /* "str" will be modified */
	if (!key)
		return;

	static int cache[3] = {-1, -1, -1};
	static int cchinx = 0;

	ktree::tree_node<aldict_charindex>::treeNodePtr next;
	int size = parent->children().size();
	bool found = false;
	u32 chr;
	int pos = 0;

	if (size > 0) {
		if (cchinx < 3) {
			if (cache[cchinx] != -1) {
				pos = cache[cchinx];
				chr = ald_read_u32(parent->child(pos)->value().wchr);
				if (chr == key) {
					found = true;
					goto ADD;
				}
				/* a new tree, clear cache. */
				for (int i = cchinx; i < 3; i++)
					cache[i] = -1;
			}
		}

		pos = size - 1;
		chr = ald_read_u32(parent->child(pos)->value().wchr);
		if (chr == key) {
			found = true;
			goto ADD;
		} else if (chr < key) {
			pos = size;
			goto ADD;
		}

		pos = bsearch(parent, key, 0, size - 1);
		chr = ald_read_u32(parent->child(pos)->value().wchr);
		if (chr == key) {
			found = true;
			goto ADD;
		}
		if (chr < key && pos < size)
			++pos;
		goto ADD;
	}

ADD:
	bool leaf = *str == '\0' ? true : false;

	struct aldict_charindex charInx;
	ald_write_u32(charInx.wchr, key);
	ald_write_u16(charInx.len_content, 0);
	if (leaf) {
		ald_write_u32(charInx.location, d_off);
		++total_entry;
	} else {
		ald_write_u32(charInx.location, ALD_INVALID_ADDR);
	}

	if (!found) {
		if (pos == size)
			next = parent->insert(charInx);
		else
			next = parent->insert(charInx, pos);
	} else {
		if (leaf) {
			if (ald_read_u32((*parent)[pos]->value().location) == ALD_INVALID_ADDR) {
				ald_write_u32((*parent)[pos]->value().location, d_off);
			} else {
				/* Two different words can collapse into one index (e.g.
				 * "apple" and "apple " after trimming); keep both by
				 * inserting a dummy node in front of the existing one. */
				printf("WARRNING: append a duplicate index--last u32('%lc')\n", chr);
				duplicate_index_flag = true;
				ald_write_u32(charInx.wchr, 0);
				(*parent)[pos]->insert(charInx, 0);
			}
		} else {
			next = (*parent)[pos];
		}
	}
	if (cchinx < 3)
		cache[cchinx] = pos;
	/* advance to next level */
	if (!leaf) {
		++cchinx;
		add_to_indextree(next, str, d_off);
		--cchinx;
	}
}

static bool is_in_stringindex(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			      int words, int depth)
{
	if (depth == 0 && parent->children().size() == 0)
		return false;
	if (parent->children().size() > STRINX_WORDS_MAX)
		return false;
	/* figure-3, if there is one more children, this node should be char index. */
	int max_depth = words > 1 ? STRINX_DEPTH_MAX : STRINX_LEN_MAX;
	if (++depth > max_depth)
		return false;

	for (int i = 0; i < parent->children().size(); i++) {
		struct aldict_charindex& charIndex = (*parent)[i]->value();
		if (ald_read_u32(charIndex.location) != ALD_INVALID_ADDR) {
			if (++words > STRINX_WORDS_MAX)
				return false;
		}
		if (!is_in_stringindex((*parent)[i], words, depth))
			return false;
	}
	return true;
}

static void write_stringindex(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			      int len_inx, int* total, off_t *start, FILE* file)
{
	static u32 index[STRINX_LEN_MAX] = {0};

	for (int i = 0; i < parent->children().size(); i++) {
		struct aldict_charindex& charIndex = (*parent)[i]->value();
		u32 wchr = ald_read_u32(charIndex.wchr);
		if (len_inx < STRINX_LEN_MAX) {
			index[len_inx] = wchr;
		} else {
			printf("WARRING: length of string index greatter then STRINX_LEN_MAX,please check function is_in_stringindex\n");
			index[STRINX_LEN_MAX - 1] = wchr;
		}

		if (ald_read_u32(charIndex.location) != ALD_INVALID_ADDR) {
			int nbytes_strinx = sizeof(struct aldict_stringindex) - 1;
			size_t nbytes_str = 0;
			index[len_inx + 1] = L'\0';
			char* mbindex = CharUtil::ucs4StrToUTF8Str(index, &nbytes_str);

			if (mbindex != NULL) {
				if (nbytes_str == (size_t)-1) {
					nbytes_str = strlen(mbindex);
					printf("WARRING: part of index was converted to utf-8, length(%lu)\n", nbytes_str);
				}
				nbytes_strinx += nbytes_str;
				struct aldict_stringindex *strinx = (struct aldict_stringindex *)malloc(nbytes_strinx);
				memcpy(strinx->str, mbindex, nbytes_str);
				memcpy(strinx->location, charIndex.location, sizeof(strinx->location));
				strinx->len_str[0] = nbytes_str;

				off_t offset = check_block_bound(ftello(file), nbytes_strinx);
				fseek(file, offset, SEEK_CUR);
				if ((*total) == 0)
					*start = ftello(file);
				fwrite(strinx, nbytes_strinx, 1, file);
				free(strinx);
				free(mbindex);
				(*total) += 1;
			} else {
				printf("ERROR: index did't be converted to utf-8:(%x) \n", wchr);
			}
		}
		write_stringindex((*parent)[i], len_inx + 1, total, start, file);
	}
}

/*
 * Strip string index, save it to sinxfile.
 *
 * 'parent' should be saved to char index area, check if its children should be
 * saved to char index area or string index area. If a child should be saved to
 * char index area, then all the children should be save to char index area.
 */
static void trim_indextree(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			   int depth, FILE* sinxfile)
{
	bool bIsStrIndex = false;
	if (++depth > CHRINX_DEPTH_MIN &&
	    ald_read_u32(parent->value().location) == ALD_INVALID_ADDR) {
		bIsStrIndex = is_in_stringindex(parent, 0, 0);
	}
	if (bIsStrIndex) {
		struct aldict_charindex& cinx = parent->value();
		int len_content = 0;
		off_t loc = 0;
		write_stringindex(parent, 0, &len_content, &loc, sinxfile); /* parent is char index */
		ald_write_u32(cinx.location, loc);
		cinx.location[3] |= 0x80;
		ald_write_u16(cinx.len_content, len_content);
		parent->children().clear(); /* Children have been save to string index area */
	} else {
		for (int i = 0; i < parent->children().size(); i++)
			trim_indextree((*parent)[i], depth, sinxfile);
	}
}

/*
 * After trimming index tree, only char index left, write it to file.
 */
static void write_charindex(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			    FILE* cinxfile)
{
	/*
	 * Deal with three situation (see figure 2):
	 *     1) parent is a char index and has children().
	 *     2) parent[i] is a node with string index.
	 *     3) parent[i] is only a non-leaf char index node.
	 */
	/* Reserve room for children, children should be save together at parent's location. */
	total_chrindex += parent->children().size();
	for (int i = 0; i < parent->children().size(); i++) {
		struct aldict_charindex& i_cinx = (*parent)[i]->value();
		int loc = ald_read_u32(i_cinx.location);
		int clen = (*parent)[i]->children().size();

		if (loc == ALD_INVALID_ADDR) { /* Situation 3 */
			ald_write_u16(i_cinx.len_content, clen);
			/* Reserve room for parent[i]'s children.
			   Write sequentialy from the end of file. */
			ald_write_u32(i_cinx.location, total_chrindex * sizeof(struct aldict_charindex));
		} else if ((*parent)[i]->children().size() > 0) { /* Situation 1 */
			struct aldict_charindex inx;

			ald_write_u32(inx.location, loc);
			ald_write_u32(inx.wchr, 0);
			ald_write_u16(inx.len_content, 0);
			(*parent)[i]->insert(inx, 0); /* Add a '0' index specifing the location in data area */

			ald_write_u16(i_cinx.len_content, clen + 1);
			ald_write_u32(i_cinx.location, total_chrindex * sizeof(struct aldict_charindex));
		}

		off_t offset = ald_read_u32(parent->value().location) + i * sizeof(struct aldict_charindex);
		fseek(cinxfile, (INDEX_BLOCK_NR - 1) * ALD_BLOCK + offset, SEEK_SET);
		fwrite(&i_cinx, sizeof(struct aldict_charindex), 1, cinxfile);
		write_charindex((*parent)[i], cinxfile);
	}
}

static void merge_file(FILE *det, FILE *src)
{
	unsigned char buf[2048];
	int nr;

	while ((nr = fread(buf, 1, 2048, src)) > 0)
		fwrite(buf, 1, nr, det);
}

/* @return  - offset aganist current pos. caller should use SEEK_CUR */
static off_t check_block_bound(off_t pos, int nbytes)
{
	off_t remain = ALD_BLOCK - pos % ALD_BLOCK;
	if (nbytes > remain)
		return remain; /* seek to next block */
	return 0;
}
