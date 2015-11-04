# ifdef WIN32
#include <Windows.h>
# endif

#include "Log.h"
#include "DictManager.h"
#include "Util.h"
#include "VocabularyBook.h"
#include "db/DBFactory.h"
#include "db/DBUtil.h"

// argn: reserve for use in the future.
const string VocabularyBook::s_tblWordColumnName[] = {"word",             "expl", "count",   "complexity", "date", "arg1", "arg2", "arg3"};
const string VocabularyBook::s_tblWordColumnType[] = {"TEXT PRIMARY KEY", "TEXT", "INTEGER", "INTEGER",    "TEXT", "TEXT", "TEXT", "TEXT"};
const int VocabularyBook::s_tblWordRowLen = 8;
const string VocabularyBook::s_tblWordName = "tbl_word";

VocabularyBook::VocabularyBook()
{
    m_db = DBFactory::create();
}

VocabularyBook::VocabularyBook(const string&  bookfile)
{
    m_db = DBFactory::create();
    load(bookfile);
}

bool VocabularyBook::load(const string& bookfile)
{
    m_total = VB_CAPACITY;

    string dbfile = bookfile + m_db->suffix();    
    if (Util::isFileExist(dbfile)) {
        m_db->open(dbfile);
        return loadDB(dbfile);
    } else {
        m_db->open(dbfile);  // will create db file.
        m_db->createTable(s_tblWordName, s_tblWordColumnName, s_tblWordColumnType, s_tblWordRowLen);

        return loadXml(bookfile + ".xml"); // Used to use xml to save data.
    }
}

bool VocabularyBook::loadDB(const string& dbfile)
{
    vector< vector<string> > table = m_db->queryTable(s_tblWordName);
    for (int row = 0; row < table.size(); row++) {
        //g_sysLog(LOG_DEBUG, "[loadDB]: (%s,%s,%s,%s)\n", table[row][0].c_str(), table[row][1].c_str(), table[row][2].c_str(), table[row][3].c_str());
        struct VBookItem item;
        item.word = table[row][0];
        item.expl = table[row][1];
        item.count = Util::stringToInt(table[row][2]);
        item.complexity = Util::stringToInt(table[row][3]);
        item.date = table[row][4];

        m_wdlist.push_back(item);
    }
	return true;
}

bool VocabularyBook::loadXml(const string& bookfile)
{
    if (!Util::isFileExist(bookfile)) {
        return true;
    }

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(bookfile.c_str()) != XML_NO_ERROR) {
        g_sysLog(LOG_ERROR, "can't load vocabulary book\n");
        return false;
    }

    XMLElement* rootElement = doc.FirstChildElement();
    if (rootElement == false) {
        g_sysLog(LOG_ERROR, "can't get root element of vocabulary book xml file\n");
        return false;      
    }

    XMLElement* wordElement = rootElement->FirstChildElement();
    while (wordElement) {
        string word, expl;
		const char* textE;
        if (wordElement->FirstChildElement("word")) {
		    if ((textE = wordElement->FirstChildElement("word")->GetText()) != NULL)
		        word = textE;
        }
        if (wordElement->FirstChildElement("expl")) {
		    if ((textE = wordElement->FirstChildElement("expl")->GetText()) != NULL)
		        expl = textE;
        }

        if (!wordExist(word)) {
            add(word, expl);
        } else {
            incCount(word);
        }

		wordElement = wordElement->NextSiblingElement();
    }
    return true;
}

bool VocabularyBook::wordExist(const string& word)
{
    string key[] = {s_tblWordColumnName[0]};
    string val[] = {word};

    return m_db->rowExist(s_tblWordName, key, val, 1);
}

bool VocabularyBook::add(const string& word)
{
    if (!wordExist(word)) {
        if (m_wdlist.size() >= m_total)
            return false;

        DictItemList itemList;
        DictManager::getReference().lookup(word, 0, itemList);

        string expl;
        for (int i=0; i< itemList.size(); i++) {
            if (itemList[i].bfind)
                expl += itemList[i].expl + "\n";
        }
//    if (expl == "")
//        return false;
        return add(word, expl);
    }
 
    incCount(word);
    return true;
}

bool VocabularyBook::add(const string& word, const string& expl)
{
    string date =  Util::getDate();
    string val[s_tblWordRowLen] = {word, expl, "1", "1", date};
    bool ret = m_db->insertTable(s_tblWordName, s_tblWordColumnName, val, s_tblWordRowLen);

    struct VBookItem item;
    item.word = word;
    item.expl = expl;
    item.count = 1;
    item.complexity = 1;
    item.date = date;

    m_wdlist.push_back(item);
    return ret;
}

void VocabularyBook::incCount(const string& word)
{
    int row = findRow(word);
    if (row != NPOS) {
        ++m_wdlist[row].count;
        updateCount(word, m_wdlist[row].count);
    } else {
        updateCount(word, getCount(word) + 1);
    }
 }

int VocabularyBook::getCount(const string& word)
{
    string col[] = {"count"};
    string cond = "WHERE word == \'?\'";
    cond = DBUtil::addParam(cond, word);

    vector< vector<string> > ret = m_db->queryTable(s_tblWordName, col, 1, cond);
    if (ret.empty())
        return 0;
    return Util::stringToInt(ret[0][0]);
}

void VocabularyBook::updateCount(const string& word,  int count)
{
    string col[] = {"count"};
    string val[] = {Util::intToString(count)};

    string cond = "WHERE word == \'?\'";
    cond = DBUtil::addParam(cond, word);

    m_db->updateTable(s_tblWordName, col, val, 1, cond);
}

void VocabularyBook::setComplexity(const int row, int cmplx)
{
    if (row < m_wdlist.size()) {
        updateComplexity(m_wdlist[row].word, cmplx);
        m_wdlist[row].complexity = cmplx;
    }
}

void VocabularyBook::updateComplexity(const string& word, int cmplx)
{
    string col[] = {"complexity"};
    string val[] = {Util::intToString(cmplx)};

    string cond = "WHERE word == \'?\'";
    cond = DBUtil::addParam(cond, word);

    m_db->updateTable(s_tblWordName, col, val, 1, cond);
}

void VocabularyBook::remove(const int row)
{
    if (row < m_wdlist.size()) {
        m_db->deleteTable(s_tblWordName, "word", m_wdlist[row].word);
        m_wdlist.erase(m_wdlist.begin() + row);
    }
}

void VocabularyBook::clear()
{
    m_wdlist.clear();
    m_db->deleteTable(s_tblWordName);
}

int VocabularyBook::size() const
{
    return m_wdlist.size();
}

string VocabularyBook::getWord(const int row) const
{
    /*
    list<struct VBookItem>::iterator iter = m_wdlist.begin();
    ITER_ADVANCE(iter, row);
    return (*iter).word;
    */
    return (row < m_wdlist.size()) ?  m_wdlist[row].word : "";
}

string VocabularyBook::getExpl(const int row) const
{
    return (row < m_wdlist.size()) ?  m_wdlist[row].expl : "";
}

VBookItem VocabularyBook::getItem(const int row) const
{
    VBookItem ret;
    return  (row < m_wdlist.size()) ?  m_wdlist[row] : ret;
}

VBookItem VocabularyBook::getItem(const string& word) const
{
    string cond = "WHERE word == \'?\'";
    cond = DBUtil::addParam(cond, word);
    vector< vector<string> > table = m_db->queryTable(s_tblWordName, s_tblWordColumnName, s_tblWordRowLen, cond);

    struct VBookItem item;
    if (table.size() == 1) {
        item.word = table[0][0];
        item.expl = table[0][1];
        item.count = Util::stringToInt(table[0][2]);
        item.complexity = Util::stringToInt(table[0][3]);
        item.date = table[0][4];
    }
    return item;
}

int VocabularyBook::findRow(const string& word)
{
    vector<struct VBookItem>::iterator iter = m_wdlist.begin();
    for (int i = 0; i < m_wdlist.size(); i++, iter++) {
        if ((*iter).word == word)
            return i;
    }

    return NPOS; 
}
