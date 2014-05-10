#include "Application.h"
#include "Util.h"
#include "DictManager.h"
#include "MessageQueue.h"
#include "Configure.h"
#include "alphadict.h"

#include <dirent.h>
#include <boost/algorithm/string.hpp>
//#include <boost/filesystem.hpp>

using namespace tinyxml2;

#define DICTNODE_NUM_MAX   48

Configure& Configure::getRefrence()
{
    static Configure configure;
    return configure;
}

Configure::Configure():m_homeDir(""),m_configFile(""),
 m_srcLan("any"),m_detLan("any"),m_dirty(false)
{
}

Configure::~Configure()
{
    m_doc.SaveFile(m_configFile.c_str());
}

void Configure::initialization()
{
    string userHome;
    if (getenv("HOME"))
        userHome = getenv("HOME");
    else
        userHome = "/root";
    m_homeDir = userHome + "/." + APP_NAME;
    m_configFile = m_homeDir + "/configure.xml";
    g_log(LOG_INFO, "home direcotry:(%s)\n", m_homeDir.c_str());

    m_dataDir = DATADIR;
    //Util::execDir(m_dataDir);
    g_log(LOG_INFO, "system dir :(%s)\n", m_dataDir.c_str());

    if (!Util::isDirExist(m_homeDir)) {
         AL_ASSERT(Util::createDir(m_homeDir) == true, "{Configure} can't create home dir\n");
         string path = m_dataDir;
         Util::copyFile(m_dataDir + "/configure.xml.in", m_configFile);
         //Util::copyFile(m_dataDir + "/language.txt.in", m_homeDir + "/language.txt");
    }
    if (!Util::isFileExist(m_configFile)) {
        AL_ASSERT( Util::copyFile(m_dataDir + "/configure.xml.in", m_configFile) == true,
                   "{Configure} can't create home dir\n");
    }

    load(m_configFile);

    loadLanguage();
    
    Message msg;
    msg.id = MSG_SET_LANLIST;
    msg.strArg1 = m_srcLan;
    msg.strArg2 = m_detLan;
    msg.pArg1 = &m_languages;
    g_uiMessageQ.push(msg);
}

void Configure::load(const string& xmlpath)
{
    AL_ASSERT(m_doc.LoadFile(xmlpath.c_str()) == XML_NO_ERROR, "{Configure} can't load xml\n");
    XMLElement* rootElement = m_doc.RootElement();

    XMLElement* tempElement = rootElement->FirstChildElement("srclan");
    if (tempElement && tempElement->GetText()) {
        m_srcLan = tempElement->GetText();
    }

    tempElement = rootElement->FirstChildElement("detlan");
    if (tempElement && tempElement->GetText()) {
        m_detLan = tempElement->GetText();
    }

    XMLElement* dictsElement = rootElement->FirstChildElement("dict");
    XMLElement* dictElement = dictsElement->FirstChildElement();
    while (dictElement) {
        struct DictNode dict;
        if (dictElement->Attribute("open"))
            dict.open   = string(dictElement->Attribute("open"));
        dict.en = dictElement->BoolAttribute("en");
    #if 0
        if (dictElement->Attribute("name"))
            dict.name = string(dictElement->Attribute("name"));
        if (dictElement->Attribute("srclan"))
            dict.srclan = string(dictElement->Attribute("srclan"));
        if (dictElement->Attribute("detlan"))
            dict.detlan = string(dictElement->Attribute("detlan"));
        if (dictElement->Attribute("path"))
            dict.path  = string(dictElement->Attribute("path"));
    #endif
		const char* textE;
        if (dictElement->FirstChildElement("srclan")) {
		    if ((textE = dictElement->FirstChildElement("srclan")->GetText()) != NULL)
		        dict.srclan = textE;
        }
        if (dictElement->FirstChildElement("detlan")) {
		    if ((textE = dictElement->FirstChildElement("detlan")->GetText()) != NULL)
		        dict.detlan = textE;
        }
        if (dictElement->FirstChildElement("name")) {
		    if ((textE = dictElement->FirstChildElement("name")->GetText()) != NULL)
		        dict.name = textE;
        }
        if (dictElement->FirstChildElement("path")) {
		    if ((textE = dictElement->FirstChildElement("path")->GetText()) != NULL)
		        dict.path = textE;
        }
        if (dictElement->FirstChildElement("summary")) {
		    if ((textE = dictElement->FirstChildElement("summary")->GetText()) != NULL)
		        dict.summary = textE;
        }
        m_dictNodes.push_back(dict);
		dictElement = dictElement->NextSiblingElement();
    }

    scanDictDir();
    m_doc.SaveFile(xmlpath.c_str());
}

void Configure::scanDictDir()
{
    //string dictDir = "dicts";
    //scanDictDir(dictDir);
//    dictDir = SYSTEM_HOME;
//    dictDir += "/dicts";  
    scanDictDir(m_dataDir+"/dicts");
}

void Configure::scanDictDir(const string& path)
{
    struct dirent *ep;
    DIR *dp = opendir(path.c_str());
    //printf("dict dir path:(%s)\n", path.c_str());
    if (dp != NULL) {
        while (ep = readdir(dp)) {
            if (ep->d_name[0] == '.')
                continue;
            string f = string(ep->d_name);
            //printf("dirctory:(%s)\n", f.c_str());
            if (!findDict(f)) {
                struct DictNode d;
                d.en = true;    
                d.name = f;
                d.path  = path + "/" + f;
                m_dictNodes.push_back(d);
            }
        }
        closedir(dp);
    }
}

void Configure::loadLanguage()
{
    string path = m_dataDir + "/language.txt";
    FILE *f = fopen(path.c_str(),"r");
    if (!f) {
        g_log(LOG_ERROR, "can find language file\n");
        return;
    }

    ReadFile read;
    char buf[1024];
    int size = read(f, (void *)buf, 1024);
    buf[size] = '\0';
    string strTemp(buf);
    vector<string> splitVec;
    boost::split(splitVec, strTemp, boost::is_any_of("\n"),
			     boost::algorithm::token_compress_on);

    vector<string>::iterator iter;
    for (iter=splitVec.begin(); iter!=splitVec.end(); iter++) {
        if (*iter != "" && iter->at(0) != ' ' && iter->at(0) != '\n') {
            m_languages.push_back(*iter);
        }
    }
}

void Configure::moveDictItem(int index, bool down)
{
    struct DictNode tempNode = m_dictNodes[index];
    int newIndex = down ? index + 1 : index - 1;
    if (newIndex < 0 || newIndex >= m_dictNodes.size()) {
        g_log(LOG_ERROR, "{Configure} move to a invalid row(%d)\n", newIndex);
        return;
    }
    m_dictNodes[index] = m_dictNodes[newIndex];
    m_dictNodes[newIndex] = tempNode;
    writeDictItem(index);
    writeDictItem(newIndex);

#if 0
    SpinLock lock(m_cs);
    int afterThis = down ? newIndex : index;
    int addThis = down ? index : newIndex;
    XMLElement* dictElement = XMLHandle(m_doc.RootElement()).FirstChildElement("dict").ToElement();
    string temp = dictItemName(afterThis);
    XMLElement* afterThisElement = dictElement->FirstChildElement(temp.c_str());
    temp = dictItemName(addThis);
    XMLElement* addThisElement = dictElement->FirstChildElement(temp.c_str());  
    dictElement->InsertAfterChild(afterThisElement, addThisElement);
#endif

}

void Configure::enableDict(int index, bool en)
{
    if (m_dictNodes[index].en != en) {
        SpinLock lock(m_cs);
        m_dictNodes[index].en = en;
        string ndname = dictItemName(index);
        XMLElement* e = XMLHandle(m_doc.RootElement()).FirstChildElement("dict").
                        FirstChildElement(ndname.c_str()).ToElement();
        e->SetAttribute("en",  en);
        m_dirty = true;
    }
}

void Configure::writeDictItem(int item)
{
    if (item > DICTNODE_NUM_MAX)
        return;
    SpinLock lock(m_cs);
    XMLElement* dictElement = XMLHandle(m_doc.RootElement()).FirstChildElement("dict").ToElement();
    string ndname = dictItemName(item);
    XMLElement* e = dictElement->FirstChildElement(ndname.c_str());
    bool newNode = false;
    if (e == NULL) {
        e = m_doc.NewElement(ndname.c_str());
        e->SetAttribute("open",   m_dictNodes[item].open.c_str());
        e->SetAttribute("en",     m_dictNodes[item].en);

        XMLElement* textE = m_doc.NewElement("summary");
	    textE->InsertFirstChild(m_doc.NewText(m_dictNodes[item].summary.c_str()));
		e->InsertFirstChild(textE);

    	textE = m_doc.NewElement("path");
    	textE->InsertFirstChild(m_doc.NewText(m_dictNodes[item].path.c_str()));
    	e->InsertFirstChild(textE);

    	textE = m_doc.NewElement("name");
    	textE->InsertFirstChild(m_doc.NewText(m_dictNodes[item].name.c_str()));
    	e->InsertFirstChild(textE);

    	textE = m_doc.NewElement("detlan");
    	textE->InsertFirstChild(m_doc.NewText(m_dictNodes[item].detlan.c_str()));
    	e->InsertFirstChild(textE);

    	textE = m_doc.NewElement("srclan");
    	textE->InsertFirstChild(m_doc.NewText(m_dictNodes[item].srclan.c_str()));
    	e->InsertFirstChild(textE);

        dictElement->InsertEndChild(e);
        newNode = true;
    } else {
        e->SetAttribute("open",   m_dictNodes[item].open.c_str());
        e->SetAttribute("en",     m_dictNodes[item].en);

        XMLText* txt = e->FirstChildElement("summary")->FirstChild()->ToText();
        txt->SetValue(m_dictNodes[item].summary.c_str());

        txt = e->FirstChildElement("path")->FirstChild()->ToText();
    	txt->SetValue(m_dictNodes[item].path.c_str());

        txt = e->FirstChildElement("name")->FirstChild()->ToText();
    	txt->SetValue(m_dictNodes[item].name.c_str());

        txt = e->FirstChildElement("detlan")->FirstChild()->ToText();
    	txt->SetValue(m_dictNodes[item].detlan.c_str());

        txt = e->FirstChildElement("srclan")->FirstChild()->ToText();
    	txt->SetValue(m_dictNodes[item].srclan.c_str());
    }

    /*printf("writeDictItem(%s): %d, %s -> %s\n", ndname.c_str(), 
           item, m_dictNodes[item].srclan.c_str(), m_dictNodes[item].detlan.c_str());*/
    m_dirty = true;
}

void Configure::writeSrcLan(const string& lan)
{
    if (m_srcLan.compare(lan) != 0) {
        SpinLock lock(m_cs);
        m_srcLan = lan;
        XMLText* txt = XMLHandle(m_doc.RootElement()).FirstChildElement("srclan").FirstChild().ToText();
        if (txt) {
            txt->SetValue(lan.c_str());
            m_dirty = true;
        }
    }
}

void Configure::writeDetLan(const string& lan)
{
    if (m_detLan.compare(lan) != 0) {
        SpinLock lock(m_cs);
	    m_detLan = lan;
        XMLText* txt = XMLHandle(m_doc.RootElement()).FirstChildElement("detlan").FirstChild().ToText();
        if (txt) {
            txt->SetValue(lan.c_str());
            m_dirty = true;
        }
    }
}

void Configure::writeXml()
{
    SpinLock lock(m_cs);
    if (m_dirty) {
       m_dirty = false;
       m_doc.SaveFile(m_configFile.c_str());
    }
}

string Configure::dictItemName(int item)
{
    char c;
    if (item <= 24)
        c = 'A' + item;
    if (item <= 48)
        c = 'a' + item;
    return string(1, c);
}

bool Configure::findDict(string name)
{
    for (int i=0; i<m_dictNodes.size(); i++) {
        if(m_dictNodes[i].name == name)
            return true;   
    }
    return false;
}
