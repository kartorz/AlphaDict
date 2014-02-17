/*
 * Convert xml file to  AlphaDictionary.
 *
 * See Doc/dictionary_inner.pdf for more inner format detail.
 *
 * Changes:
 * 11-Nov-2013, initial version: LiQiong lee<LiQiong.kartorz.lee@gmail.com>
 *
 */

#include "alphadict.h"
#include "aldict/aldict_inner.h"
#include "tinyxml2/tinyxml2.h"
#include "kary_tree/kary_tree.hpp"

#include <stddef.h>
#include <locale.h>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>   /* above boost 1.32.0 */

using namespace std;
using namespace tinyxml2;

#define INDEX_BLOCK_NR   2
#define MAX_WORDS_STRINX 3
#define MAX_DEPTH_STRINX 3
#define MIN_DEPTH_CHRINX 1
#define DICT_SUFFIX      ".ad"

typedef ktree::kary_tree<aldict_charindex>&  IndexTreeRef;
static int total_chrindex = 0; /* Used in recursion funciton write_charindex */

static void usage();
static void make_dict(const string& xmlfile, const string& dictfile);
static void add_to_indextree(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			     char *str, off_t pos);
static void add_to_dictfile(const XMLElement* wordElement, FILE *file);
static void add_alias(IndexTreeRef indexTreeRef, const XMLElement* wordElement, off_t pos);
static void trim_indextree(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			   int depth, FILE* sinxfile);
static void write_charindex(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			    FILE* cinxfile);
static wchar_t mbrtowc_nextwc(char** strmb);
static void merge_file(FILE *det, FILE *src);

int main(int argc, char* argv[])
{
	int c;
	string dicpath = "";
	string xmlpath = "";
	string outpath = "";

	setlocale(LC_ALL, "C.UTF-8");

#if 0
	if (argc < 2) {
		usage();
		return 0;
	}

	while (( c = getopt(argc, argv, "x:o:")) != -1) {
	switch (c) {
	case 'x':
		xmlpath = optarg;
	break;
	case 'o':
		outpath = optarg;
		outpath += DICT_SUFFIX;
	break;
	case '?':
		usage();
		goto out;
	}
	}
#endif
	if (argc == 3) {
		xmlpath = argv[1];
		outpath = argv[2];
		outpath += DICT_SUFFIX;
		make_dict(xmlpath, outpath);
	}else {
		usage();
		return 0;
	}
}

static void usage()
{
	printf("Usage: AlConvert <xml> <dict>\n");
	printf("Convert <xml> file to <dict> file.\n");
	printf("For example:\n");
	printf("    AlConvert xx.xml  xx     [create a dict xx.ad]");
}

static void make_dict(const string& xmlpath, const string& dictpath)
{
	FILE *dictfile;
	AL_ASSERT((dictfile = fopen(dictpath.c_str(),"w")) != NULL,
		  "Can't open dict file, please check permission");

	XMLDocument doc;
	AL_ASSERT(doc.LoadFile(xmlpath.c_str()) == XML_NO_ERROR, "XMLDocument can't load xml\n");
	
	/*-@ Stage 1: Write Header -*/
	/* The <header\> shall have envery "Node" needed.  */
	struct aldict_header header;
	memset(&header, 0, sizeof(struct aldict_header));
	
	const XMLElement* rootElement = doc.RootElement();
	AL_ASSERT(rootElement, "xmlparser failure");
	
	const XMLElement* headerElement = rootElement->FirstChildElement("header");
	AL_ASSERT(headerElement, "xmlparser failure");
	
	vector<string> splitVec;
	string strTemp;
	const char *pstrTemp;
	unsigned int u32Temp;

	try {
		header.magic[0] = 0x77;
		header.magic[1] = 0x88;
	 
		header.h_version[0] = headerElement->FirstChildElement("version")->\
				FirstChild()->Value()[0];

		strTemp = headerElement->FirstChildElement("publishdate")->FirstChild()->Value();

		boost::split(splitVec, strTemp, boost::is_any_of("-"),
			     boost::algorithm::token_compress_on);
		header.p_date[0] = boost::lexical_cast<int>(splitVec[2]);
		header.p_date[1] = boost::lexical_cast<int>(splitVec[1]);
		header.p_date[2] = boost::lexical_cast<int>(splitVec[0]) & 0x00ff;
		header.p_date[3] = boost::lexical_cast<int>(splitVec[0]) >> 8;
		//lexical_cast<int>(strTemp.data(), 4);

		pstrTemp = headerElement->FirstChildElement("publisher")->\
				FirstChild()->Value();

		strncpy((char *)(header.p_identi), pstrTemp, 60);

		splitVec.clear();
		strTemp.clear();
		strTemp = headerElement->FirstChildElement("dictversion")->\
				FirstChild()->Value();

		boost::split(splitVec, strTemp, boost::is_any_of("."),
			     boost::algorithm::token_compress_on);
		if (splitVec.size() > 1) {
			header.d_version[0] = boost::lexical_cast<int>(splitVec[1]);
		} else {
			header.d_version[0] = 0;
		}
		header.d_version[1] = boost::lexical_cast<int>(splitVec[0]);
		
		pstrTemp = headerElement->FirstChildElement("dictname")->FirstChild()->Value();
		strncpy((char *)(header.d_identi), pstrTemp, 60);

		strTemp = headerElement->FirstChildElement("entries")->FirstChild()->Value();
		u32Temp = boost::lexical_cast<unsigned int>(strTemp);
		ald_write_u32(header.d_entries, u32Temp);
		//fwrite(&header, sizeof(struct aldict_header), 1, dictfile);
	}
	catch (exception e) {
		AL_ASSERT(false, "Prase xml failure");
	}

	/*-@ Stage 2: Create Index tree, write data file. */
	#ifdef _LINUX
	FILE *dict_tmpfile = fopen("/tmp/alphadict_tmp", "w+");
	FILE *strinx_tempfile = fopen("/tmp/alphadict_tmp2", "w+");
	#endif

	struct aldict_charindex charIndex;
	memset(&charIndex, 0, sizeof(struct aldict_charindex));
	ktree::kary_tree<aldict_charindex> indexTree(charIndex);
	const XMLElement* wordsElement = rootElement->FirstChildElement("words");
	AL_ASSERT(wordsElement, "Parse xml failure");

	const XMLElement* wordElement = wordsElement->FirstChildElement();

	while (wordElement) {
		off_t start;
		start = ftello(dict_tmpfile);
		add_to_dictfile(wordElement, dict_tmpfile);

		pstrTemp = wordElement->Value();
		if (pstrTemp && (*pstrTemp != '\0')) {
			add_to_indextree(indexTree.root(), (char *)pstrTemp, start);
			add_alias(indexTree, wordElement, start);
		}

		wordElement = wordElement->NextSiblingElement();
	}

	/*-@ Stage 3: Trim index tree, remove string index and write it to string index temp file. */
	trim_indextree(indexTree.root(), 0, strinx_tempfile);
	
	/*-@ Stage 4: Write char index to dict file. */
	fseek(dictfile, (INDEX_BLOCK_NR-1)*ALD_BLOCK, SEEK_SET);
	// Write root node.
	ktree::tree_node<aldict_charindex>::treeNodePtr rootNode = indexTree.root();
	struct aldict_charindex& rootIndex = rootNode->value();
	ald_write_u32(rootIndex.location, sizeof(struct aldict_charindex));
	ald_write_u32(rootIndex.len_content, rootNode->children().size());
	fwrite(&rootIndex, sizeof(struct aldict_charindex), 1, dictfile);
	total_chrindex = 1;
	// Write all nodes recursively.
	write_charindex(rootNode, dictfile);

	/*-@ Stage 5: Merge temple files */
	ald_write_u32(header.loc_chrindex, INDEX_BLOCK_NR);

	fseek(dictfile, 0, SEEK_END);
	int bnr = ALD_BLOCK_NR(ftello(dictfile)) + 1;
	ald_write_u32(header.loc_strindex, bnr);
	fseek(dictfile, (bnr-1)*ALD_BLOCK, SEEK_SET);
	fseek(strinx_tempfile, 0, SEEK_SET);
	merge_file(dictfile, strinx_tempfile);
	
	bnr = ALD_BLOCK_NR(ftello(dictfile)) + 1;
	ald_write_u32(header.loc_data, bnr);
	fseek(dictfile, (bnr-1)*ALD_BLOCK, SEEK_SET);
	fseek(dict_tmpfile, 0, SEEK_SET);
	merge_file(dictfile, dict_tmpfile);
	fseek(dictfile, 0, SEEK_SET);
	fwrite(&header, sizeof(struct aldict_header), 1, dictfile);

	fclose(dictfile);
	fclose(strinx_tempfile);
	fclose(dict_tmpfile);
}

static void add_to_dictfile(const XMLElement* wordElement, FILE* dtf)
{
	unsigned char len[2];
	string strWord = wordElement->Value();
	len[0] = strWord.length();
	fwrite((void *)len, 1, 1, dtf);
	fwrite((void *)strWord.c_str(), len[0], 1, dtf);

	string strPhonetic;
	XMLConstHandle wordElementHd(wordElement);
	const XMLElement* phoneticNameElement = 
		wordElementHd.FirstChildElement("phonetic").FirstChild().ToElement();
	for (; phoneticNameElement; ) {
		strPhonetic += phoneticNameElement->Value();
		strPhonetic += phoneticNameElement->GetText();
		strPhonetic += "\n";
		phoneticNameElement = phoneticNameElement->NextSiblingElement();
	}
	
	len[0] = strPhonetic.length();
	fwrite((void *)len, 1, 1, dtf);
	fwrite((void *)strPhonetic.c_str(), len[0], 1, dtf);

	string strExpln;
	const XMLElement* explnTextElement = 
		wordElementHd.FirstChildElement("explanation").FirstChild().ToElement();
	for ( ; explnTextElement; ) {
		strExpln += explnTextElement->GetText();
		strExpln += "\n";
		explnTextElement = explnTextElement->NextSiblingElement();
	}

	ald_write_u16(len, strExpln.length());	
	fwrite((void *)len, 2, 1, dtf);
	fwrite((void *)strExpln.c_str(), strExpln.length(), 1, dtf);
}

static void add_to_indextree(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			     char *str, off_t pos)
{
	const wchar_t charIn = mbrtowc_nextwc(&str); /* "str" will be modified */
	if (!charIn)
		return;

	ktree::tree_node<aldict_charindex>::treeNodePtr next;
	/* Find char index */
	int size = parent->children().size();
	int i;
	bool found = false;
	for (i=0; i<size; i++) {
		wchar_t chr = ald_read_u32(parent->child(i)->value().wchr);
		if (chr < charIn)
			continue;
		else if (chr > charIn)
			break;
		else {
			found = true;
			next = (*parent)[i];
			break;
		}
	}	
	bool leaf = *str == '\0' ? true : false;
	if (!found) {
		struct aldict_charindex  charInx;

		ald_write_u32(charInx.wchr, charIn);
		if (leaf)
			ald_write_u32(charInx.location, pos);
		else
			ald_write_u32(charInx.location, ALD_INVALID_ADDR);

		if (i == size)
			next = parent->insert(charInx);
		else
			next = parent->insert(charInx, i);
	} else if (leaf)
		printf("WARRNING: duplicate index\n");

	if (!leaf)
		add_to_indextree(next, str, pos);
}

static void add_alias(IndexTreeRef indexTree, const XMLElement* wordElement, off_t pos)
{
	XMLConstHandle wordElementHd(wordElement);
	const XMLElement* asElement = 
		wordElementHd.FirstChildElement("alias").FirstChild().ToElement();

	while (asElement) {
		const char *as = asElement->GetText();
		add_to_indextree(indexTree.root(), (char *)as, pos);
		asElement = asElement->NextSiblingElement();
	}
}

/// The below three functions "is_in_stringindex", "write_stringindex" and "trim_indextree"
/// strip string index from index tree and write it to a temp file.
/// The rule for stripping string index is in the function is_in_stringindex.

/* Check if paren's children should be recorded in string index area.
 * If meet the following conditions:
 *    - the number of words less equal than MAX_WORDS_STRINX.
 *    - the length of tree less equal than MAX_DEPTH_STRINX.
 *
 * Figures:
 *    "&" represents a end node(location != ALD_INVALID_ADDR).
 *    "@" represents a normal node.
 *
 *             @            @        @         -- (parent, 0, 0)
 *           / |          / | \      |
 *          &  @         &  @  @     @
 *         /   |        /   |  |     |
 *        &    &       &    &  $     ..
 *                                   | 
 *                                   @
 *                                   |                                   
 *                                   &  
 * 
 *         figure-1    figure-2   figure-3
 *
 *    figure-1 and figure-3 should be string index.
 *    figure-2 should be char index.
 */
static bool is_in_stringindex(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			      int number, int depth)
{
	if (parent->children().size() > MAX_WORDS_STRINX)
		return false;
	// firgure-3, if there is one more children, this node should be char index.
	int max_depth = number > 1 ? MAX_DEPTH_STRINX : ALD_STRINX_MAX;
	if (++depth > max_depth)
		return false;

	for (int i=0; i< parent->children().size(); i++) {
		struct aldict_charindex& charIndex = (*parent)[i]->value();
		if (ald_read_u32(charIndex.location) != ALD_INVALID_ADDR) {
			if (++number > MAX_WORDS_STRINX)
				return false;
		}	
		if (!is_in_stringindex((*parent)[i], number, depth))
			return false;
	}
	return true;
}

static int write_stringindex(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			     int len_inx, int* nbytes_total, FILE* file)
{
	static wchar_t index[ALD_STRINX_MAX] = {0};

	for (int i=0; i<parent->children().size(); i++) {
		struct aldict_charindex& charIndex = (*parent)[i]->value();
		wchar_t wchr = ald_read_u32(charIndex.wchr);
		if (len_inx < ALD_STRINX_MAX) {
			index[len_inx] = wchr;
		} else {
			printf("WARRING: length of string index greatter then MAX_DEPTH_STRINX\n");
		}

		if (ald_read_u32(charIndex.location) != ALD_INVALID_ADDR) {
			int nbytes_strinx = sizeof(aldict_stringindex)-1;
			int nbytes_str = 4*(len_inx+1);
			nbytes_strinx += nbytes_str;		      
			aldict_stringindex *strinx = (aldict_stringindex *)malloc(nbytes_strinx);
			memcpy(strinx->str, index, nbytes_str);
			memcpy(strinx->location, charIndex.location, 4);
			strinx->len_str[0] = len_inx+1; /* How many char */
			fwrite(strinx, nbytes_strinx, 1, file);
			(*nbytes_total) += 1;
		}
	        write_stringindex((*parent)[i], len_inx+1, nbytes_total, file);
	}
}

/*
 *   Strip string index, Save it to sinxfile.
 *  "parent" should be saved to char index area, check if its children should be saved to 
 *  char index area or string index area. If a child should be saved to char index area,
 *  then all the children should be save to char index area.
 *  "is_in_stringindex(..)" decides which index area children should be saved to.
 *
 */
static void trim_indextree(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			   int depth, FILE* sinxfile)
{
	bool bIsStrIndex = false;
	if (++depth > MIN_DEPTH_CHRINX)
		bIsStrIndex = is_in_stringindex(parent, 0, 0);
	if (bIsStrIndex) {
		struct aldict_charindex& cinx = parent->value();
		ald_write_u32(cinx.location, ftello(sinxfile));
		cinx.location[3] |= 0x80;
		int len_content = 0;
		write_stringindex(parent, 0, &len_content, sinxfile);/* parent is char index */
		ald_write_u16(cinx.len_content, len_content);
		parent->children().clear(); /* Children have been save to string index area */
	} else {
		//parent.len_content = parent->children().size();
		for (int i=0; i<parent->children().size(); i++) {
			trim_indextree((*parent)[i], depth, sinxfile);
		}
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
	/* Reserve room for children, children should be save together at parent's location.*/
	total_chrindex += parent->children().size();
	for (int i=0; i<parent->children().size(); i++) {
		struct aldict_charindex& i_cinx = (*parent)[i]->value();
		int loc = ald_read_u32(i_cinx.location);
		int clen = (*parent)[i]->children().size();

		if (loc == ALD_INVALID_ADDR) { /* Situation 3 */
			ald_write_u32(i_cinx.len_content, clen);
			/* Reserve room for parent[i]'s children.
			   Write sequentialy from the end of file. 
			   So, total_chrindex must be a global variable. */
			ald_write_u32(i_cinx.location, total_chrindex*sizeof(struct aldict_charindex));			
		} else if ((*parent)[i]->children().size() > 0) { /* Situation 1 */
			struct aldict_charindex inx;

			ald_write_u32(inx.location, loc);
			ald_write_u32(inx.wchr, 0);
			ald_write_u32(inx.len_content, 0);
			(*parent)[i]->insert(inx, 0); /* Add a '0' index specifing the location in data area */

			ald_write_u32(i_cinx.len_content, clen+1);
			ald_write_u32(i_cinx.location, total_chrindex*sizeof(struct aldict_charindex));		
		}
	
	       // Children's room has been reserved in parent's location.
	       off_t offset = ald_read_u32(parent->value().location) + i*sizeof(struct aldict_charindex);
	       fseek(cinxfile, (INDEX_BLOCK_NR-1)*ALD_BLOCK+offset, SEEK_SET);
	       fwrite(&i_cinx, sizeof(struct aldict_charindex), 1, cinxfile);	       
	       write_charindex((*parent)[i], cinxfile);
	}
}

/// Utils.  ///
static wchar_t mbrtowc_nextwc(char** strmb)
{
	wchar_t wctmp[1];
	size_t len = strlen(*strmb);
	size_t nbytes = mbrtowc(wctmp, *strmb, len, NULL);
	if (nbytes > 0) {
		if (nbytes > (size_t) - 2)
			return 0;
		*strmb += nbytes;
		return wctmp[0];
	}
}

static void merge_file(FILE *det, FILE *src)
{
	unsigned char buf[2048];
	int nr;
	
	while ((nr = fread(buf, 1, 2048, src)) > 0) {
		fwrite(buf, 1, nr, det);
	}
}
