/*
 * Convert xml file to  AlphaDictionary.
 *
 * See Doc/dictionary_inner.pdf for more inner format detail.
 *
 * Changes:
 * 11-Nov-2013, initial version: [LiQiong lee]
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
#define STRINX_WORDS_MAX 10
#define STRINX_DEPTH_MAX 5
#define CHRINX_DEPTH_MIN 1
#define DICT_SUFFIX      ".aldict"
#define STRINX_LEN_MAX   60  /* a string index must be within a block. */
/* If word tag is a pahrase that will contains blanks, 
   put the rest words after first blank in to 'phrase' attribute of elements. */
#define WORD_ATTR  "word"

typedef ktree::kary_tree<aldict_charindex>&  IndexTreeRef;
static int total_chrindex = 0; /* Used in recursion funciton write_charindex */

static void usage();
static void make_dict(const string& xmlfile, const string& dictfile);
static void add_to_indextree(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			     char *str, off_t pos);
static void add_to_dictfile(const string& strWord, const XMLElement* wordElement, FILE *file);
static void add_alias(IndexTreeRef indexTreeRef, const XMLElement* wordElement, off_t pos);
static void trim_indextree(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			   int depth, FILE* sinxfile);
static void write_charindex(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			    FILE* cinxfile);
static wchar_t mbrtowc_nextwc(char** strmb);
static void merge_file(FILE *det, FILE *src);
static off_t check_block_bound(off_t pos, int nbytes);
static char* wcsrtombs_r(const wchar_t *wc, size_t *mb_len);

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
		//outpath += DICT_SUFFIX;
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
	printf("    AlConvert xx.xml  xx     [create a dict xx]\n");
}

static void make_dict(const string& xmlpath, const string& dictpath)
{
	FILE *dictfile;
    printf("{make_dict}, xmlpath(%s), dictpath(%s)\n", xmlpath.c_str(), dictpath.c_str());

	AL_ASSERT((dictfile = fopen(dictpath.c_str(),"w")) != NULL,
		  "Can't open dict file, please check permission");
    
	XMLDocument doc;
	int xmlerr_code = doc.LoadFile(xmlpath.c_str());
	if (xmlerr_code != XML_NO_ERROR) {
	    fprintf(stderr, "XMLDocument can't load xml: error code(%d)\n", xmlerr_code);
	    exit(xmlerr_code);
	}
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
	const XMLElement* tempElement = NULL;
	try {
		header.magic[0] = 0x77;
		header.magic[1] = 0x88;
	 
		tempElement = headerElement->FirstChildElement("version");
		if (tempElement)
		  header.h_version[0] = tempElement->FirstChild()->Value()[0];

		tempElement= headerElement->FirstChildElement("publishdate");
		if (tempElement) {
		    strTemp = tempElement->FirstChild()->Value();
		    boost::split(splitVec, strTemp, boost::is_any_of("-"),
		    	     boost::algorithm::token_compress_on);
		    header.p_date[0] = boost::lexical_cast<int>(splitVec[2]);
		    header.p_date[1] = boost::lexical_cast<int>(splitVec[1]);
		    header.p_date[2] = boost::lexical_cast<int>(splitVec[0]) & 0x00ff;
		    header.p_date[3] = boost::lexical_cast<int>(splitVec[0]) >> 8;
		    //lexical_cast<int>(strTemp.data(), 4);
		}

		tempElement = headerElement->FirstChildElement("publisher");
		if (tempElement) {
		    pstrTemp = tempElement->FirstChild()->Value();
		    strncpy((char *)(header.p_identi), pstrTemp, 60);
		}

		splitVec.clear();
		strTemp.clear();
		tempElement = headerElement->FirstChildElement("dictversion");
		if (tempElement) {
		    strTemp = tempElement->FirstChild()->Value();

     	    boost::split(splitVec, strTemp, boost::is_any_of("."),
			     boost::algorithm::token_compress_on);
		    if (splitVec.size() > 1) {
		    	header.d_version[0] = boost::lexical_cast<int>(splitVec[1]);
		    } else {
		    	header.d_version[0] = 0;
		    }
		    header.d_version[1] = boost::lexical_cast<int>(splitVec[0]);
		}

		tempElement = headerElement->FirstChildElement("dictname");
		if (tempElement) {
		    pstrTemp = tempElement->FirstChild()->Value();
		    strncpy((char *)(header.d_identi), pstrTemp, 60);
            header.d_identi[59] = '\0';
		}

		tempElement =  headerElement->FirstChildElement("entries");
        if (tempElement) {
            strTemp = tempElement->FirstChild()->Value();

      		u32Temp = boost::lexical_cast<unsigned int>(strTemp);
		    ald_write_u32(header.d_entries, u32Temp);
		}
		//fwrite(&header, sizeof(struct aldict_header), 1, dictfile);

        tempElement = headerElement->FirstChildElement("srclan");
		if (tempElement) {
		    pstrTemp = tempElement->FirstChild()->Value();
        } else {
		    pstrTemp = "any";
		}
		strncpy((char *)(header.src_lan), pstrTemp, 15);
        header.src_lan[14] = '\0';

        tempElement = headerElement->FirstChildElement("detlan");
		if (tempElement) {
		    pstrTemp = tempElement->FirstChild()->Value();
        } else {
		    pstrTemp = "any";
		}
		strncpy((char *)(header.det_lan), pstrTemp, 15);
        header.det_lan[14] = '\0';
	}
	catch (exception e) {
	    /* It's annoy to check every header tag if has a text node,so xml should be responsible for that .*/
		AL_ASSERT(false, "Prase xml failure, please check header tag, every tag of 'header' should have a text node.");
	}
	printf("parse header, pass\n");
    printf("to create index tree\n");
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
		char *word = (char *)wordElement->Attribute(WORD_ATTR);

		start = ftello(dict_tmpfile);
		add_to_dictfile(string(word), wordElement, dict_tmpfile);

		if (word != NULL) {
			//add_to_indextree(indexTree.root(), (char *)pstrTemp, start);
		    add_to_indextree(indexTree.root(), word, start);
			add_alias(indexTree, wordElement, start);
		}

		wordElement = wordElement->NextSiblingElement();
	}
	printf("add to index tree, done\n");
	/*-@ Stage 3: Trim index tree, remove string index and write it to string index temp file. */
	trim_indextree(indexTree.root(), 0, strinx_tempfile);
	
	printf("trim index tree, done\n");
	/*-@ Stage 4: Write char index to dict file. */
	fseek(dictfile, (INDEX_BLOCK_NR-1)*ALD_BLOCK, SEEK_SET);
	// Write root node.
	ktree::tree_node<aldict_charindex>::treeNodePtr rootNode = indexTree.root();
	struct aldict_charindex& rootIndex = rootNode->value();
	ald_write_u32(rootIndex.location, sizeof(struct aldict_charindex));
	ald_write_u16(rootIndex.len_content, rootNode->children().size());
	fwrite(&rootIndex, sizeof(struct aldict_charindex), 1, dictfile);
	total_chrindex = 1;
	// Write all nodes recursively.
	write_charindex(rootNode, dictfile);

	/*-@ Stage 5: Merge temple files */
	header.loc_chrindex[0] = INDEX_BLOCK_NR;

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
	printf("write dict file, done\n");

	fclose(dictfile);
	fclose(strinx_tempfile);
	fclose(dict_tmpfile);
}

static void add_to_dictfile(const string& strWord, const XMLElement* wordElement, FILE* dict)
{
	unsigned char len[2];

	len[0] = strWord.length();
	fwrite((void *)len, 1, 1, dict);
	fwrite((void *)strWord.c_str(), len[0], 1, dict);

	string strPhonetic="";
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
	fwrite((void *)len, 1, 1, dict);
    if (len[0] > 0)
	    fwrite((void *)strPhonetic.c_str(), len[0], 1, dict);

	string strExpln="";
	const XMLElement* explnElement = 
		wordElementHd.FirstChildElement("explanation").ToElement();
    strExpln += explnElement->FirstChild()->Value();

	ald_write_u16(len, strExpln.length());
	fwrite((void *)len, 2, 1, dict);
	fwrite((void *)strExpln.c_str(), strExpln.length(), 1, dict);
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
        ald_write_u16(charInx.len_content, 0);
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
 *    - the number of words less equal than STRINX_WORDS_MAX.
 *    - the length of tree less equal than STRINX_DEPTH_MAX.
 *
 * Figures:
 *    "&" represents a end node(location != ALD_INVALID_ADDR).
 *    "@" represents a normal node.
 *
 *             @             @          @    -- (parent, 0, 0)
 *           / |          /  |   \      |
 *          &  @         &   @ .. @     @
 *         /   |        /    |  |       |
 *        &    &       &     &  $       ..
 *                                      | 
 *                                      @
 *                                      |                                   
 *                                      &  
 * 
 *         figure-1    figure-2     figure-3
 *
 *    figure-1 and figure-3 should be string index.
 *    figure-2 should be char index.
 */
static bool is_in_stringindex(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			      int words, int depth)
{
	if (parent->children().size() > STRINX_WORDS_MAX)
		return false;
	// firgure-3, if there is one more children, this node should be char index.
	int max_depth = words > 1 ? STRINX_DEPTH_MAX : STRINX_LEN_MAX;
	if (++depth > max_depth)
		return false;

	for (int i=0; i< parent->children().size(); i++) {
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

/* @len_inx - length of string.
   @total - how many items. */
static void write_stringindex(ktree::tree_node<aldict_charindex>::treeNodePtr parent,
			                 int len_inx, int* total, off_t *start, FILE* file)
{
	static wchar_t index[STRINX_LEN_MAX] = {0};

	for (int i=0; i<parent->children().size(); i++) {
		struct aldict_charindex& charIndex = (*parent)[i]->value();
		wchar_t wchr = ald_read_u32(charIndex.wchr);
		if (len_inx < STRINX_LEN_MAX) {
			index[len_inx] = wchr;
		} else {
			printf(
			"WARRING: length of string index greatter then STRINX_LEN_MAX,please check function is_in_stringindex\n");
            index[STRINX_LEN_MAX-1] = wchr;
		}

		if (ald_read_u32(charIndex.location) != ALD_INVALID_ADDR) {
			int nbytes_strinx = sizeof( struct aldict_stringindex)-1;
            size_t  nbytes_str=0;
            index[len_inx+1] = L'\0';
            char* mbindex = wcsrtombs_r(index, &nbytes_str);
            if (mbindex != NULL) {
                if (nbytes_str == (size_t) -1) {
                    nbytes_str = strlen(mbindex);
                    printf("WARRING: part of index was converted to utf-8, length(%lu)\n", nbytes_str);
                }
                //printf("%s-->%lu\n", mbindex, nbytes_str);
                nbytes_strinx += nbytes_str;
			    struct aldict_stringindex *strinx = ( struct aldict_stringindex *)malloc(nbytes_strinx);
			    memcpy(strinx->str, mbindex, nbytes_str);
			    memcpy(strinx->location, charIndex.location, 4);
			    strinx->len_str[0] = nbytes_str;
                
			    off_t offset = check_block_bound(ftello(file), nbytes_strinx);
			    fseek(file, offset, SEEK_CUR);
                if ((*total) == 0) {
                    *start = ftello(file);
                }
			    fwrite(strinx, nbytes_strinx, 1, file);
			    free(strinx);
                free(mbindex);
			    (*total) += 1;
            }
		}
		write_stringindex((*parent)[i], len_inx+1, total, start, file);
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
	if (++depth > CHRINX_DEPTH_MIN && 
	    ald_read_u32(parent->value().location) == ALD_INVALID_ADDR) {
		bIsStrIndex = is_in_stringindex(parent, 0, 0);
	}
	if (bIsStrIndex) {
		struct aldict_charindex& cinx = parent->value();
		//ald_write_u32(cinx.location, ftello(sinxfile));
		int len_content = 0;
		off_t loc = 0;
        write_stringindex(parent, 0, &len_content, &loc, sinxfile);/* parent is char index */
        ald_write_u32(cinx.location, loc);
        cinx.location[3] |= 0x80;
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
	total_chrindex += parent->children().size(); /* as a global variable, always point to the last file position */
	for (int i=0; i<parent->children().size(); i++) {
		struct aldict_charindex& i_cinx = (*parent)[i]->value();
		int loc = ald_read_u32(i_cinx.location);
		int clen = (*parent)[i]->children().size();

		if (loc == ALD_INVALID_ADDR) { /* Situation 3 */
			ald_write_u16(i_cinx.len_content, clen);
			/* Reserve room for parent[i]'s children.
			   Write sequentialy from the end of file. 
			   So, total_chrindex must be a global variable. */
			ald_write_u32(i_cinx.location, total_chrindex*sizeof(struct aldict_charindex));			
		} else if ((*parent)[i]->children().size() > 0) { /* Situation 1 */
			struct aldict_charindex inx;

			ald_write_u32(inx.location, loc);
			ald_write_u32(inx.wchr, 0);
			ald_write_u16(inx.len_content, 0);
			(*parent)[i]->insert(inx, 0); /* Add a '0' index specifing the location in data area */

			ald_write_u16(i_cinx.len_content, clen+1);
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

static char* wcsrtombs_r(const wchar_t *wc, size_t *mb_len)
{
    size_t len = wcslen(wc);
    len = len*sizeof(wchar_t) + 1;
    char *result = (char *)malloc(len);
    memset(result, '\0', len);
    *mb_len = wcsrtombs(result, &wc, len, NULL);
    if (*mb_len == (size_t)-1) {
        if (result[0] == '\0') {
            free(result);
            return NULL;
        } else {
            printf("{wcsrtombs_r}: (%s), encounter a invalid wide character(0x%x)\n", result, *wc);
        }
    }

    return result;
}

static void merge_file(FILE *det, FILE *src)
{
	unsigned char buf[2048];
	int nr;
	
	while ((nr = fread(buf, 1, 2048, src)) > 0) {
		fwrite(buf, 1, nr, det);
	}
}

/*@return  - offset aganist current pos. caller should use SEEK_CUR */
static off_t check_block_bound(off_t pos, int nbytes)
{
	off_t remain = ALD_BLOCK - pos%ALD_BLOCK;
	if (nbytes > remain){
		return remain; /* seek to next block */
    }
	return 0; /* don't seek */
}
