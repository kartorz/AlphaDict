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
const string VocabularyBook::s_tblWordColumnName[] = {"word",             "expl", "count",   "complexity", "date", "date_seconds", "arg1", "arg2", "arg3"};
const string VocabularyBook::s_tblWordColumnType[] = {"TEXT PRIMARY KEY", "TEXT", "INTEGER", "INTEGER",    "TEXT", "INTEGER",      "TEXT", "TEXT", "TEXT"};
const int VocabularyBook::s_tblWordRowLen = 9;
const string VocabularyBook::s_tblWordName = "tbl_word";

VocabularyBook::VocabularyBook()
{
//    m_total = 5000;
    m_db = DBFactory::create();
}

VocabularyBook::VocabularyBook(const string&  bookfile)
{
    m_db = DBFactory::create();
    load(bookfile);
}

bool VocabularyBook::load(const string& bookfile)
{
    string dbfile = bookfile + m_db->suffix();    
    if (Util::isFileExist(dbfile)) {
        m_db->open(dbfile);  // will create db file.
        loadDB(dbfile);
    } else {
        m_db->open(dbfile);  // create db file
        m_db->createTable(s_tblWordName, s_tblWordColumnName, s_tblWordColumnName, s_tblWordRowLen);

        loadXml(bookfile + ".xml"); // Used to use xml to save data.
    }
}

bool VocabularyBook::loadDB(const string& dbfile)
{
    vector< vector<string> > table = m_db->queryTable(s_tblWordName);
    for (int row = 0; row < table.size(); row++) {
        struct VBookItem item;
        item.word = table[row][0];
        item.expl = table[row][1];
        item.count = Util::stringToInt(table[row][2]);
        item.date = table[row][3];

        m_wdlist.push_back(item);
    }
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
            updateCount(word, getCount(word) + 1);   
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
        DictItemList itemList;
        DictManager::getReference().lookup(word, 0, itemList);

        string expl;
        for (int i=0; i< itemList.size(); i++) {
            if (itemList[i].bfind)
                expl += itemList[i].expl + "\n";
        }
//    if (expl == "")
//        return false;
        add(word, expl);
    } else {
        updateCount(word, getCount(word) + 1);
    }
    return false;
}

bool VocabularyBook::add(const string& word, const string& expl)
{
    string date =  Util::getDate();
    string val[s_tblWordRowLen] = {word, expl, "1", date};
    m_db->insertTable(s_tblWordName, s_tblWordColumnName, val, s_tblWordRowLen);

    struct VBookItem item;
    item.word = word;
    item.expl = expl;
    item.count = 1;
    item.date = date;

    m_wdlist.push_back(item);
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

void VocabularyBook::remove(const int row)
{
/*    XMLElement* root = m_doc.RootElement();
    XMLElement* itemElement = root->FirstChildElement();
    ELEMENT_ADVANCE(itemElement, row);

    root->DeleteChild(itemElement);*/

    list<struct VBookItem>::iterator iter = m_wdlist.begin();
    ITER_ADVANCE(iter, row);
    m_wdlist.erase(iter);
//    m_doc.SaveFile(m_bookpath.c_str());
}

void VocabularyBook::clear()
{
    m_wdlist.clear();
/*    XMLElement* root = m_doc.RootElement();
    if (root) {
        root->DeleteChildren();
        m_doc.SaveFile(m_bookpath.c_str());
    }*/
}

int VocabularyBook::size() const
{
    return m_wdlist.size();
}

string VocabularyBook::getWord(const int row) const
{
    list<struct VBookItem>::iterator iter = m_wdlist.begin();
    ITER_ADVANCE(iter, row);
    return (*iter).word;
}

string VocabularyBook::getExpl(const int row) const
{
    if ( row < m_wdlist.size()) {
        list<struct VBookItem>::iterator iter = m_wdlist.begin();
        ITER_ADVANCE(iter, row);
        return (*iter).expl;
    }
    return "";
}
