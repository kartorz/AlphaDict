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
#define MAX_DEPTH_STRINX 5

typedef ktree::kary_tree<aldict_charindex>&  IndexTreeRef;
static int total_chrindex = 0; /* Used in recursion funciton write_charindex */

static void usage();
static void make_dict(const string& xmlfile, const string& dictfile);
static void add_to_indextree(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			     char *str, off_t pos);
static off_t add_to_dictfile(const XMLElement* wordElement, FILE *file);
static void  add_alias(IndexTreeRef indexTreeRef, const XMLElement* wordElement, off_t pos);
static void trim_indextree(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			   FILE* sinxFile);
static void write_charindex(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			    FILE* cinxFile);
static wchar_t mbrtowc_nextwc(char** strmb);
static void merge_file(FILE *det, FILE *src);

int main(int argc, char* argv[])
{
	int c;
	string dicpath = "";
	string xmlpath = "";
	string outpath = "";

	if (argc < 2) {
		usage();
		return 0;
	}

	setlocale(LC_ALL, "C.UTF-8");

	while (( c = getopt(argc, argv, "x:o:")) != -1) {
	switch (c) {
	case 'x':
		xmlpath = optarg;
	break;
	case 'o':
		outpath = optarg;
		outpath += ".ibd";
	break;
	case '?':
		usage();
		goto out;
	}
	}

	if (!xmlpath.empty()) {
		make_dict(xmlpath, outpath);
	}
out:
	return 0;
}

static void usage()
{
	printf("Usage: AlConvert [-x xml] [-o dictionary]\n");
	printf("Convert xml file to dictionary file.\n");
	printf("    -x:  specify xml file \n");
	printf("    -c:  specify dict file \n");
	printf("For example:\n");
	printf("    AlConvert -c ../xmls/xx.xml -o xx\n");
}

static void make_dict(const string& xmlpath, const string& dictpath)
{
	FILE *dictFile;
	AL_ASSERT((dictFile = fopen(dictpath.c_str(),"w")) != NULL,
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

		strTemp = headerElement->FirstChildElement("publishdate")->\
				FirstChild()->Value();

		boost::split(splitVec, strTemp, boost::is_any_of("-"),
			     boost::algorithm::token_compress_on);

		header.p_date[0] = boost::lexical_cast<char>(splitVec[2]);
		header.p_date[1] = boost::lexical_cast<char>(splitVec[1]);
		header.p_date[2] = boost::lexical_cast<short>(splitVec[2]) & 0x00ff;
		header.p_date[3] = boost::lexical_cast<short>(splitVec[2]) >> 8;
		
		pstrTemp = headerElement->FirstChildElement("publisher")->\
				FirstChild()->Value();

		strncpy((char *)(header.p_identi), pstrTemp, 60);

		splitVec.clear();
		strTemp.clear();
		strTemp = headerElement->FirstChildElement("dictversion")->\
				FirstChild()->Value();

		boost::split(splitVec, strTemp, boost::is_any_of("."),
			     boost::algorithm::token_compress_on);

		header.d_version[0] = boost::lexical_cast<char>(splitVec[1]);
		header.d_version[1] = boost::lexical_cast<char>(splitVec[0]);
		
		pstrTemp = headerElement->FirstChildElement("dictname")->FirstChild()->Value();
		strncpy((char *)(header.d_identi), pstrTemp, 60);

		strTemp = headerElement->FirstChildElement("entries")->FirstChild()->Value();
		u32Temp = boost::lexical_cast<unsigned int>(strTemp);
		ald_write_u32(header.d_entries, u32Temp);
		//fwrite(&header, sizeof(struct aldict_header), 1, dictFile);
	}
	catch (exception e) {
		AL_ASSERT(false, "Prase xml failure");
	}

	/*-@ Stage 2: Create Index tree, write data file. */
	#ifdef _LINUX
	FILE *dictTmpFile = fopen("/tmp/alphadict_tmp", "w");
	FILE *strIndexTmpFile = fopen("/tmp/alphadict_tmp2", "w");
	#endif

	struct aldict_charindex charIndex;
	memset(&charIndex, 0, sizeof(struct aldict_charindex));
	ktree::kary_tree<aldict_charindex> indexTree(charIndex);
	const XMLElement* wordsElement = rootElement->FirstChildElement("words");
	AL_ASSERT(wordsElement, "Parse xml failure");

	const XMLElement* wordElement = wordsElement->FirstChildElement();

	while (wordElement) {
		off_t start;
		start = add_to_dictfile(wordElement, dictTmpFile);

		pstrTemp = wordElement->Value();
		if (pstrTemp && (*pstrTemp != '\0')) {
			add_to_indextree(indexTree.root(), (char *)pstrTemp, start);
			add_alias(indexTree, wordElement, start);
		}

		wordElement = wordElement->NextSiblingElement();
	}

	/*-@ Stage 3: Trim index tree, remove string index and write it to string index temp file. */
	trim_indextree(indexTree.root(), strIndexTmpFile);
	
	/*-@ Stage 4: Write char index to dict file. */
	fseek(dictFile, (INDEX_BLOCK_NR-1)*ALD_BLOCK, SEEK_SET);
	// Write root node.
	ktree::tree_node<aldict_charindex>::treeNodePtr rootNode = indexTree.root();
	struct aldict_charindex& rootIndex = rootNode->value();
	ald_write_u32(rootIndex.location, sizeof(struct aldict_charindex));
	ald_write_u32(rootIndex.len_content, rootNode->children().size());
	fwrite(&rootIndex, sizeof(struct aldict_charindex), 1, dictFile);
	total_chrindex = 1;
	// Write all nodes recursively.
	write_charindex(rootNode, dictFile);

	/*-@ Stage 5: Merge temple files */

	ald_write_u32(header.loc_chrindex, INDEX_BLOCK_NR);

	fseek(dictFile, 0, SEEK_END);
	int bnr = ALD_BLOCK_NR(ftello(dictFile)) + 1;
	ald_write_u32(header.loc_strindex, bnr);
	fseek(dictFile, bnr*ALD_BLOCK, SEEK_SET);
	merge_file(dictFile, strIndexTmpFile);
	
	bnr = ALD_BLOCK_NR(ftello(dictFile)) + 1;
	ald_write_u32(header.loc_data, bnr);
	fseek(dictFile, bnr*ALD_BLOCK, SEEK_SET);
	merge_file(dictFile, dictTmpFile);

	fclose(dictFile);
	fclose(strIndexTmpFile);
	fclose(dictTmpFile);
}

static off_t add_to_dictfile(const XMLElement* wordElement, FILE* dictFile)
{
	unsigned char len[2];
	string strPhonetic;
	XMLConstHandle wordElementHd(wordElement);
	const XMLElement* phoneticNameElement = 
		wordElementHd.FirstChildElement("phonetic").FirstChild().ToElement();
	for (; phoneticNameElement; ) {
		strPhonetic += phoneticNameElement->Value();
		strPhonetic += " [";
		strPhonetic += phoneticNameElement->GetText();
		strPhonetic += "]";
		strPhonetic += "\n";
		phoneticNameElement = phoneticNameElement->NextSiblingElement();
	}
	
	len[0] = strPhonetic.length();	
	fwrite((void *)len, 1, 1, dictFile);
	fwrite((void *)strPhonetic.c_str(), len[0], 1, dictFile);

	string strExpln;
	const XMLElement* explnTextElement = 
		wordElementHd.FirstChildElement("explanation").FirstChild().ToElement();
	for ( ; explnTextElement; ) {
		strExpln += explnTextElement->GetText();
		strExpln += "\n";
		explnTextElement = explnTextElement->NextSiblingElement();
	}

	ald_write_u16(len, strExpln.length());	
	fwrite((void *)len, 2, 1, dictFile);
	fwrite((void *)strExpln.c_str(), strExpln.length(), 1, dictFile);
	return ftello(dictFile);
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
		wchar_t chr = ald_read_u16(parent->child(i)->value().wchr);
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
	bool leaf = *(str+1) == '\0';
	if (!found) {
		struct aldict_charindex  charInx;
		ald_write_u16(charInx.wchr, charIn);
		if (leaf)
			ald_write_u32(charInx.location, pos);

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
 *                                   /\
 *                                  &  & 
 * 
 *         figure-1      figure-2   figure-3
 *
 *    figure-1 should be string index.
 *    figure-2 and figure-3 should be char index.
 */
static bool is_in_stringindex(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			    int number, int depth)
{	
	if (++depth > MAX_DEPTH_STRINX)
		return false;

	for (int i=0; i< parent->children().size(); i++) {
		struct aldict_charindex& charIndex = (*parent)[i]->value();
		if (ald_read_u32(charIndex.location) != ALD_INVALID_ADDR) {
			if (++number > MAX_WORDS_STRINX)
				return false;
		}	
		return is_in_stringindex((*parent)[i], number, depth);
	}
	return true;
}

static int write_stringindex(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			     wchar_t index[], int len_inx, int* number, FILE* file)
{
	for (int i=0; i<parent->children().size(); i++) {
		struct aldict_charindex& charIndex = (*parent)[i]->value();
		wchar_t wchr = ald_read_u32(charIndex.wchr);
		if (len_inx < MAX_DEPTH_STRINX) {
			index[len_inx] = wchr;
		} else {
			printf("WARRING: length of string index greatter then MAX_DEPTH_STRINX\n");
		}

		if (ald_read_u32(charIndex.location) != ALD_INVALID_ADDR) {
			int len_strinx = sizeof(aldict_stringindex)-1;
			len_strinx += 4*len_inx;
			aldict_stringindex *strinx = (aldict_stringindex *)malloc(len_strinx);
			memcpy(strinx->location, charIndex.location, 4);
			strinx->len_str[0] = len_inx;
			memcpy(strinx->str, index, len_inx);
			fwrite(strinx, len_strinx, 1, file);
			(*number) += len_strinx;
		}
	        write_stringindex((*parent)[i], index, len_inx+1, number, file);
	}
}

/*
 *   Strip string index, Save it to sinxFile.
 *  "parent" should be saved to char index area, check if its children should be saved to 
 *  char index area or string index area. If a child should be saved to char index area,
 *  then all the children should be save to char index area.
 *  "is_in_stringindex(..)" decides which index area children should be saved to.
 *
 */
static void trim_indextree(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			   FILE* sinxFile)
{
	bool bIsStrIndex = is_in_stringindex(parent, 0, 0);
	if (!bIsStrIndex) {
		struct aldict_charindex& cinx = parent->value();
		ald_write_u32(cinx.location, ftello(sinxFile));
		cinx.location[3] |= 0x80;

		wchar_t inx[MAX_DEPTH_STRINX] = {0};
		int len = 0;
		write_stringindex(parent, inx, 0, &len, sinxFile);/* parent is char index */
		ald_write_u32(cinx.len_content, len);
		parent->children().clear(); /* Children have been save to string index area */
	} else {
		//parent.len_content = parent->children().size();
		for (int i=0; i<parent->children().size(); i++) {
			trim_indextree((*parent)[i], sinxFile);
		}
	}
}

/*
 * After trimming index tree, only char index left, write it to file.
 */
static void write_charindex(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			    FILE* cinxFile)
{
	/* Reserve room for children, children should be save together at parent's location.*/
	total_chrindex += parent->children().size(); 
	for (int i=0; i<parent->children().size(); i++) {
		struct aldict_charindex& cinx = (*parent)[i]->value();
		ald_write_u32(cinx.len_content, (*parent)[i]->children().size());

		/* Reserve room for parent[i]'s children.
		   Write sequentialy from the end of file. 
		   So, total_chrindex must be a global variable. */
		ald_write_u32(cinx.location, total_chrindex*sizeof(struct aldict_charindex));

		// Children's room has been reserved in parent's location.
		off_t offset = ald_read_u32(parent->value().location) + i*sizeof(struct aldict_charindex);
		fseek(cinxFile, INDEX_BLOCK_NR*ALD_BLOCK+offset, SEEK_SET);
		fwrite(&cinx, sizeof(struct aldict_charindex), 1, cinxFile);

		write_charindex((*parent)[i], cinxFile);
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
		strmb += nbytes;
		return wctmp[0];
	}
}

static void merge_file(FILE *det, FILE *src)
{
	unsigned char buf[2048];
	fseek(det, 0, SEEK_END);
	fseek(src, 0, SEEK_SET);
	size_t nr;
	while ((nr = fread(buf, 2048, 1, src)) > 0) {
		fwrite(buf, nr, 1, det);
	}
}
