#include "AldictDocument.h"
#include "tinyxml2/tinyxml2.h"

bool AldictDocument::xmlToDict(const string& xmlPath, const string& dictPath)
{
	FILE *xmlFile;
	FILE *dictFile;

	if ((xmlFile = fopen(xmlPath,"r,ccs=C.UTF-8")) == NULL) {
		g_log.e("Can't open (%s)\n", xmlPath.c_str());
		return false;
	}

	if ((dictFile = fopen(dictPath,"w,ccs=C.UTF-8")) != NULL) {
		g_log.e("Can't open (%s), please check permission\n", dictPath);
		return false;
	}

	/* Read Header
	/* The <header\> shall have every "Node" needed.  */
	memset(&m_header, 0, sizeof(struct AldictHeader));
	
	const XMLElement* xml_root = doc.RootElement();
	AL::AL_ASSERT(xml_root, "xmlparser");
	
	const XMLElement* xml_header = xml_root.FirstChildElement("header");
	AL::AL_ASSERT(xml_header, "xmlparser");
	
	vector<string> split_vec;
	string str_temp;
	const char *pstr_temp;
	unsigned int u32_temp;

	try {

		m_header.magic[0] = 0x77;
		m_header.magic[1] = 0x88;
	 
		m_header.h_version[0] = xml_header.FirstChildElement("version").FirstChild().Value()[0];

		str_temp = xml_header.FirstChildElement("publishdate").FirstChild().Value();
		boost::split(split_vec, str_temp, boost::is_any_of("-"), token_compress_on);
		m_header.p_date[0] = boost::lexical_cast<char>(split_vec[2]);
		m_header.p_date[1] = boost::lexical_cast<char>(split_vec[1]);
		m_header.p_date[2] = boost::lexical_cast<short>(split_vec[2]) & 0x00ff;
		m_header.p_date[3] = boost::lexical_cast<short>(split_vec[2]) >> 8;
		
		pstr_temp = xml_header.FirstChildElement("publisher").FirstChild().Value();
		strncpy(m_header.p_identi, str_identi, 60);

		split_vec.clear();
		str_temp.clear();
		str_temp = xml_header.FirstChildElement("dictversion").FirstChild().Value();
		boost::split(split_vec, str_temp, boost::is_any_of("."), token_compress_on);
		m_header.d_version[0] = boost::lexical_cast<char>(split_vec[1]);
		m_header.d_version[1] = boost::lexical_cast<char>(split_vec[0]);
		
		pstr_temp = xml_header.FirstChildElement("dictname").FirstChild().Value();
		strncpy(header.d_identi, pstr_temp, 60);

		str_temp = xml_header.FirstChildElement("entries").FirstChild().Value();
		u32_temp = boost::lexical_cast<unsigned int>(str_temp);
		ald_write_u32(&header.d_entries, u32_temp);
		
		u32_temp  = ALD_BLOCK;
		ald_write_u32(&header.loc_index, u32_temp);

		fwrite(&m_header, sizeof(struct AldictHeader), 1, fd_dict);
	}
	catch (exception e) {
		AL::AL_ASSERT(false, "Prase xml failure");
	}

	FILE *f_indexch, *f_indexstr, *f_data;

#ifdef CFG_LINUX
	f_indexch = fopen("/tmp/alphadict/index_char","w,ccs=C.UTF-8");
	f_indexstr = fopen("/tmp/alphadict/index_string","w,ccs=C.UTF-8");
	f_data = fopen("/tmp/alphadict/data","w,ccs=C.UTF-8");
#endif

	if (!f_indexch || !f_indexstr || !f_data) {
		fclose(xmlFile);
		fclose(f_dict);
		exit(EXIT_FAILURE);
	}

	/* Write Index
	/* The <word\> notes shall be in alphabetical order */
	struct aldict_index index;
	memset(&aldict_index, 0, sizeof(struct aldict_index));
	
	const XMLElement* xml_words = xml_root.FirstChildElement("words");
	AL::AL_ASSERT(xml_words, "Parse xml failure");

	XMLElement* xml_word = xml_words.FirstChildElement();
	while (xml_word) {
		pstr_temp = xml_word.value();
		
		xml_word = xml_word.NextSiblingElement();
	}
}

void AldictDocument::writeToXml(const std::string& path)
{
}
