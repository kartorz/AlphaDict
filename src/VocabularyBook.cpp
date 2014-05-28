# ifdef WIN32
#include <Windows.h>
# endif

#include <sstream>

#include "Log.h"
#include "DictManager.h"
#include "Util.h"
#include "VocabularyBook.h"

VocabularyBook::VocabularyBook()
{
}

VocabularyBook::VocabularyBook(const string&  bookfile)
{
    load(bookfile);
}

bool VocabularyBook::load(const string& bookfile)
{
    m_bookpath = bookfile;
    if (!Util::isFileExist(bookfile)) {
        const char* rootstr = "<book></book>";
		m_doc.Parse(rootstr);
        return true;
    }
    
    if (m_doc.LoadFile(bookfile.c_str()) != XML_NO_ERROR) {
        g_log(LOG_ERROR, "can't load vocabulary book\n");
        return false;
    }

    XMLElement* rootElement = m_doc.FirstChildElement();
    if (rootElement == false) {
        g_log(LOG_ERROR, "can't get root element of vocabulary book xml file\n");
        return false;      
    }

    XMLElement* wordElement = rootElement->FirstChildElement();
    while (wordElement) {
        struct VBItem item;
		const char* textE;
        if (wordElement->FirstChildElement("word")) {
		    if ((textE = wordElement->FirstChildElement("word")->GetText()) != NULL)
		        item.word = textE;
        }
        if (wordElement->FirstChildElement("expl")) {
		    if ((textE = wordElement->FirstChildElement("expl")->GetText()) != NULL)
		        item.expl = textE;
        }
        m_wdlist.push_back(item);
		wordElement = wordElement->NextSiblingElement();
    }
    return true;
}

bool VocabularyBook::add(const string& word)
{
    DictItemList itemList;
    DictManager::getReference().lookup(word, 0, itemList);
    string expl;
    for (int i=0; i< itemList.size(); i++) {
        expl += itemList[i].expl + "\n";
    }
    struct VBItem  item;
    item.word = word;
    item.expl = expl;
    m_wdlist.push_back(item);

    stringstream stream;
    stream << "id_" << m_wdlist.size() - 1;

    XMLElement* root = m_doc.RootElement();
    if (root) {
        XMLElement* newElement = m_doc.NewElement(stream.str().c_str());
        XMLElement* tempChildE;
        tempChildE = m_doc.NewElement("word");
        tempChildE->InsertFirstChild(m_doc.NewText(item.word.c_str()));
        newElement->InsertFirstChild(tempChildE);
        
        tempChildE = m_doc.NewElement("expl");
        tempChildE->InsertFirstChild(m_doc.NewText(item.expl.c_str()));
        newElement->InsertFirstChild(tempChildE);
        root->InsertEndChild(newElement);
        m_doc.SaveFile(m_bookpath.c_str());
        return true;
    }
    return false;
}

void VocabularyBook::remove(const int row)
{
    stringstream stream;
    stream << "id_" << row;

    XMLElement* root = m_doc.RootElement();
    XMLElement* targetElement = root->FirstChildElement(stream.str().c_str());
    root->DeleteChild(targetElement);

    list<struct VBItem>::iterator iter = m_wdlist.begin();
    ITER_ADVANCE(iter, row);
    m_wdlist.erase(iter);
    m_doc.SaveFile(m_bookpath.c_str());
}

void VocabularyBook::clear()
{
    m_wdlist.clear();
    XMLElement* root = m_doc.RootElement();
    if (root) {
        root->DeleteChildren();
        m_doc.SaveFile(m_bookpath.c_str());
    }
}

int VocabularyBook::size() const
{
    return m_wdlist.size();
}

string VocabularyBook::getWord(const int row) const
{
    list<struct VBItem>::iterator iter = m_wdlist.begin();
    ITER_ADVANCE(iter, row);
    return (*iter).word;
}

string VocabularyBook::getExpl(const int row) const
{
    if ( row < m_wdlist.size()) {
        list<struct VBItem>::iterator iter = m_wdlist.begin();
        ITER_ADVANCE(iter, row);
        return (*iter).expl;
    }
    return "";
}
