# ifdef WIN32
#include <Windows.h>
# endif

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <sstream>
# ifdef _LINUX
#include <dirent.h>
# endif

#include "Log.h"
#include "Application.h"
#include "Util.h"
#include "CharUtil.h"
#include "DictManager.h"
#include "MessageQueue.h"
#include "Configure.h"
#include "alphadict.h"
#include "SpinLock.h"

using namespace tinyxml2;
using namespace boost::filesystem;

#define DICTNODE_NUM_MAX   48

Configure& Configure::getRefrence()
{
    static Configure configure;
    return configure;
}

Configure::Configure():m_homeDir(""),m_configFile(""),
 m_srcLan("any"),m_detLan("any"),m_dirty(false),m_uilanID(0)
{
}

Configure::~Configure()
{
    m_doc.SaveFile(m_configFile.c_str());
}

int Configure::initialization()
{
    int ret = 0;
    Util::usrHomeDir(m_homeDir);
    m_configFile = m_homeDir + "/configure.xml";
    g_log(LOG_INFO, "home direcotry:(%s)\n", m_homeDir.c_str());

#ifdef WIN32
    m_dataDir = m_homeDir + "/system";
#else
    m_dataDir = DATADIR;
#endif
    g_log(LOG_INFO, "system dir :(%s)\n", m_dataDir.c_str());
    if (!Util::isDirExist(m_homeDir)) {
         if (Util::createDir(m_homeDir) == true) {
             string path = m_dataDir;
             Util::copyFile(m_dataDir + "/configure.xml.in", m_configFile);
         } else {
         #ifdef WIN32
             g_log.e("can't create home direcotry\n");
             return ERR_MKHOME;
         #else
             AL_ASSERT(false, "can't create home direcotry\n");
         #endif
         }
         //Util::copyFile(m_dataDir + "/language.txt.in", m_homeDir + "/language.txt");
    }
    if (!Util::isFileExist(m_configFile)) {
        if (Util::copyFile(m_dataDir + "/configure.xml.in", m_configFile) == false) {
        #ifdef WIN32
            g_log.e("{Configure} can't create home dir\n");
            return ERR_CPCFG;
        #else
            AL_ASSERT(false, "{Configure} can't create home dir\n");
        #endif
        }
    }

    ret = load(m_configFile);
    loadLanguage();
    
    Message msg;
    msg.id = MSG_SET_LANLIST;
    msg.strArg1 = m_srcLan;
    msg.strArg2 = m_detLan;
    msg.pArg1 = &m_languages;
    g_application.uiMessageQ()->push(msg);
    return ret;
}

int Configure::load(const string& xmlpath)
{
#ifdef WIN32
    if (m_doc.LoadFile(xmlpath.c_str()) != XML_NO_ERROR) {
        g_log.e("{Configure} can't load xml\n");
        return ERR_LDCFG;
    }
#else
    AL_ASSERT(m_doc.LoadFile(xmlpath.c_str()) == XML_NO_ERROR, "{Configure} can't load xml\n");
#endif
    XMLElement* rootElement = m_doc.RootElement();

    XMLElement* tempElement = rootElement->FirstChildElement("srclan");
    if (tempElement && tempElement->GetText()) {
        m_srcLan = tempElement->GetText();
    }

    tempElement = rootElement->FirstChildElement("detlan");
    if (tempElement && tempElement->GetText()) {
        m_detLan = tempElement->GetText();
    }

    tempElement = rootElement->FirstChildElement("uilan");
    if (tempElement && tempElement->GetText()) {
        stringstream  stream;
        stream << tempElement->GetText();
        stream >> m_uilanID;
    }

    vector<string> dictFiles;
    scanDictDir(dictFiles);

    XMLElement* dictsElement = rootElement->FirstChildElement("dict");
    XMLElement* dictElement = dictsElement->FirstChildElement();
    while (dictElement) {
        struct DictNode dict;
        const char* textE;
        int pos;

        if (dictElement->FirstChildElement("path")) {
            if ((textE = dictElement->FirstChildElement("path")->GetText()) != NULL)
                dict.path = textE;
        }

        if ((pos = findDict(dict.path, dictFiles)) == -1 ) {
            dictsElement->DeleteChild(dictElement);
            dictElement = dictElement->NextSiblingElement();
            continue;
        }
        dictFiles[pos] = "";

        dict.en = dictElement->BoolAttribute("en");
        if (dictElement->Attribute("open"))
            dict.open   = string(dictElement->Attribute("open"));

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

        if (dictElement->FirstChildElement("summary")) {
            if ((textE = dictElement->FirstChildElement("summary")->GetText()) != NULL)
	        dict.summary = textE;
        }
        m_dictNodes.push_back(dict);
	dictElement = dictElement->NextSiblingElement();
    }

    vector<string>::iterator iter = dictFiles.begin();
    for (; iter != dictFiles.end(); iter++) {
        if (*iter != "") {
            struct DictNode d;
            d.en = true;
            d.path = *iter;
            d.name = boost::filesystem::path(d.path).filename().string();
            //printf("%s\n",d.name.c_str());
            m_dictNodes.push_back(d);
        }
    }

    m_doc.SaveFile(xmlpath.c_str());
    return 0;
}

void Configure::scanDictDir(vector<string>& dictFiles)
{
    //string dictDir = "dicts";
    //scanDictDir(dictDir);
//    dictDir = SYSTEM_HOME;
//    dictDir += "/dicts";  
    scanDictDir(m_dataDir+"/dicts", dictFiles);
}

void Configure::scanDictDir(const string& path, vector<string>& dictFiles)
{
#ifdef WIN32
    boost::filesystem::path p(path);
    try {
        for (directory_iterator iter(p); iter != directory_iterator(); iter++) {
            string mbpath = iter->path().string();
            const char* nm = mbpath.c_str();
            char *nm2 = CharUtil::mbsrtoutf8s(nm);
            string utf8path = string(nm2);
            free(nm2);
            dictFiles.push_back(utf8path);
        }
    } catch (const filesystem_error& ex) {
        g_log(LOG_ERROR, "{scanDictDir}: %s\n", ex.what());
    }
#else
    struct dirent *ep;
    DIR *dp = opendir(path.c_str());
    //printf("dict dir path:(%s)\n", path.c_str());
    if (dp != NULL) {
        while (ep = readdir(dp)) {
            if (ep->d_name[0] == '.')
                continue;
            string f = path + "/" + string(ep->d_name);
            //printf("dirctory:(%s)\n", f.c_str());
            dictFiles.push_back(f);
        }
        closedir(dp);
    }
#endif
}

void Configure::loadLanguage()
{
    string path = m_dataDir + "/language.txt";
    FILE *f = fopen(path.c_str(),"r");
    if (!f) {
        g_log(LOG_ERROR, "can find language file\n");
        return;
    }

    util::ReadFile read;
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
    int newindex = down ? index + 1 : index - 1;
    if (newindex < 0 || newindex >= m_dictNodes.size()) {
        g_log(LOG_ERROR, "{Configure} move to a invalid row(%d)\n", newindex);
        return;
    }
    struct DictNode tempNode = m_dictNodes[index];
    m_dictNodes[index] = m_dictNodes[newindex];
    m_dictNodes[newindex] = tempNode;
    // writeDictItem(index);
    // writeDictItem(newindex);
    SpinLock lock(m_cs);
    int afterThis, addThis, max;
    if (down) {
        afterThis = newindex;
        addThis = index;
        max = newindex;
    } else {
       afterThis = index;
       addThis = newindex;
       max = index;
    }
    XMLElement* dictElement = XMLHandle(m_doc.RootElement()).FirstChildElement("dict").ToElement();
    XMLElement* e = dictElement->FirstChildElement();
    XMLElement* afterThisElement = NULL;
    XMLElement* addThisElement = NULL;
    for (int i=0; i<=max; i++) {
        if (i == afterThis)
            afterThisElement = e;
        else if (i == addThis)
            addThisElement = e;

        e = e->NextSiblingElement();
    }
    dictElement->InsertAfterChild(afterThisElement, addThisElement);
}

void Configure::enableDict(int index, bool en)
{
    if (m_dictNodes[index].en != en) {
        SpinLock lock(m_cs);
        m_dictNodes[index].en = en;
        XMLElement* p = m_doc.RootElement()->FirstChildElement("dict");
        XMLElement* e = util::XMLUtil::child(p, index);
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
    XMLElement* e = util::XMLUtil::child(dictElement, item);
    if (e == NULL) {
        e = m_doc.NewElement("d");
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
    } else {
        e->SetAttribute("open",   m_dictNodes[item].open.c_str());
        e->SetAttribute("en",     m_dictNodes[item].en);

        XMLText* txt = e->FirstChildElement("path")->FirstChild()->ToText();
    	txt->SetValue(m_dictNodes[item].path.c_str());

        txt = e->FirstChildElement("name")->FirstChild()->ToText();
    	txt->SetValue(m_dictNodes[item].name.c_str());

        txt = XMLHandle(e).FirstChildElement("detlan").FirstChild().ToText();
        if (txt)
    	    txt->SetValue(m_dictNodes[item].detlan.c_str());

        txt = XMLHandle(e).FirstChildElement("srclan").FirstChild().ToText();
        if (txt)
    	    txt->SetValue(m_dictNodes[item].srclan.c_str());

        txt = XMLHandle(e).FirstChildElement("summary").FirstChild().ToText();
        if (txt)
            txt->SetValue(m_dictNodes[item].summary.c_str());

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

void Configure::writeUILanID(int id)
{
    if (m_uilanID != id) {
        SpinLock lock(m_cs);
	    m_uilanID = id;
        stringstream  stream;
        stream << id;

        XMLElement* e = XMLHandle(m_doc.RootElement()).FirstChildElement("uilan").ToElement();
        if (e == NULL) {
            e = m_doc.NewElement("uilan");
            e->InsertFirstChild(m_doc.NewText(stream.str().c_str()));
            m_doc.RootElement()->InsertEndChild(e);
            return;
        }

        XMLText* txt = e->FirstChild()->ToText();
        if (txt) {
            txt->SetValue(stream.str().c_str());
            m_dirty = true;
        } else {
            g_log(LOG_ERROR, "writeUILanID: can't find text node\n");    
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

int Configure::findDict(const string& path, const vector<string>& dictFiles)
{
    for (int i=0; i<dictFiles.size(); i++) {
        //string filename = boost::filsystem::path(dictFiles[i]).filename();
        if (dictFiles[i] != "" && dictFiles[i] == path) {
            return i;
        }
    }
    return -1;
}
