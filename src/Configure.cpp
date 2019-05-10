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

#define CONF_VERSION  1

#define XML_TAG_CWS "capture-word-setting"
#define XML_TAG_SETTING  "setting"

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

int Configure::initialization()
{
    int ret = 0;
    Util::usrProfileDir(m_homeDir);
    m_configFile = m_homeDir + "/configure.xml";
    g_sysLog(LOG_INFO, "home direcotry:(%s)\n", m_homeDir.c_str());

#ifdef WIN32
    Util::execDir(m_dataDir);
    m_dataDir +=  "/system";
#else
    m_dataDir = DATADIR;
#endif
    g_sysLog(LOG_INFO, "system dir :(%s)\n", m_dataDir.c_str());
    if (!Util::isDirExist(m_homeDir)) {
         if (Util::createDir(m_homeDir)) {
             //string path = m_dataDir;
             Util::copyFile(m_dataDir + "/configure.xml.in", m_configFile);
         } else {
         #ifdef WIN32
             g_sysLog.e("can't create home direcotry\n");
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
            g_sysLog.e("{Configure} can't copy cofigure.xml.in \n");
            return ERR_CPCFG;
        #else
            AL_ASSERT(false, "{Configure} can't copy cofigure.xml.in \n");
        #endif
        }
    }

    ret = load(m_configFile);
    if (ret == ERR_LDCFG) {
        // Reload configure file
        Util::copyFile(m_dataDir + "/configure.xml.in", m_configFile);
        ret = load(m_configFile);
        g_sysLog.w("{Configure} load configure.xml failure, restore to default setting\n");
    }

    loadLanguage();

    Message msg;
    msg.id = MSG_SET_LANLIST;
    msg.strArg1 = m_srcLan;
    msg.strArg2 = m_detLan;
    msg.pArg1 = &m_languages;
    g_application.uiMessageQ()->push(msg); // Cause loading dict.
    return ret;
}

int Configure::load(const string& xmlpath)
{
    if (m_doc.LoadFile(xmlpath.c_str()) != XML_NO_ERROR) {
        g_sysLog.e("{Configure} can't load xml %s\n", xmlpath.c_str());
        return ERR_LDCFG;
    }

    XMLElement* rootElement = m_doc.RootElement();
    if (rootElement == NULL) {
        g_sysLog.e("{Configure} can't get root element %s\n", xmlpath.c_str());
        return ERR_LDCFG;    
    } else {
        bool reset = rootElement->IntAttribute("reset");
        if (reset)
            return ERR_LDCFG;

        int ver = rootElement->IntAttribute("version");
        if (ver != CONF_VERSION)
            return ERR_LDCFG; // Recreate configure.xml
    }

    XMLElement* tempElement = rootElement->FirstChildElement("srclan");
    if (tempElement && tempElement->GetText()) {
        m_srcLan = tempElement->GetText();
    }

    tempElement = rootElement->FirstChildElement("detlan");
    if (tempElement && tempElement->GetText()) {
        m_detLan = tempElement->GetText();
    }

    // Loading dictionary 
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
    // Loading dictionary -- end

    tempElement = rootElement->FirstChildElement(XML_TAG_CWS);
    if (tempElement) {
        m_cws.bselection  = tempElement->BoolAttribute("selection");
        m_cws.bclipboard  = tempElement->BoolAttribute("clipboard");
        m_cws.bmouse      = tempElement->BoolAttribute("mouse");
        m_cws.benable     = tempElement->BoolAttribute("enable"); 
        m_cws.shortcutKey = tempElement->IntAttribute("shortcutkey");
        m_cws.autoCloseEn = tempElement->BoolAttribute("AutoCloseEnable");
        m_cws.autoCloseInv= tempElement->IntAttribute("AutoCloseInterval");
    } else {
        m_cws.benable    = false;
        m_cws.bselection = true;
        m_cws.bmouse     = false;
        m_cws.bclipboard = false;
        m_cws.shortcutKey = 'g'-'a';
        m_cws.autoCloseEn  = true;
        m_cws.autoCloseInv = 1000*10;

        XMLElement* e = m_doc.NewElement(XML_TAG_CWS);
        e->SetAttribute("selection",        m_cws.bselection);
        e->SetAttribute("clipboard",        m_cws.bclipboard);
        e->SetAttribute("mouse",            m_cws.bmouse);
        e->SetAttribute("enable",           m_cws.benable);
        e->SetAttribute("shortcutkey",      m_cws.shortcutKey);
        e->SetAttribute("AutoCloseEnable",  m_cws.autoCloseEn);
        e->SetAttribute("AutoCloseInterval",m_cws.autoCloseInv);

        rootElement->InsertEndChild(e);
    }

    tempElement = rootElement->FirstChildElement(XML_TAG_SETTING);
    if (tempElement) {
        m_setting.uilanID     = tempElement->IntAttribute("uilan");
        m_setting.fontsize    = tempElement->IntAttribute("fontsize");
        m_setting.font        = util::XMLUtil::Attribute(tempElement, "font", "");
        m_setting.bsystemTray = tempElement->BoolAttribute("systemtray");
    } else {
        m_setting.uilanID = UILAN_NONE;
        m_setting.fontsize = 10;
        m_setting.font = "";
        m_setting.bsystemTray = true;

        XMLElement* e = m_doc.NewElement(XML_TAG_SETTING);
        e->SetAttribute("uilan", m_setting.uilanID);
        e->SetAttribute("fontsize", m_setting.fontsize);

        rootElement->InsertEndChild(e);
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

            int pos = utf8path.rfind("\\");
            if (pos != string::npos)
               utf8path.replace(pos, 1, "/");

            dictFiles.push_back(utf8path);
        }
    } catch (const filesystem_error& ex) {
        g_sysLog(LOG_ERROR, "{scanDictDir}: %s\n", ex.what());
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
        g_sysLog(LOG_ERROR, "can find language file\n");
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
        g_sysLog(LOG_ERROR, "{Configure} move to a invalid row(%d)\n", newindex);
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
    m_dirty = true;
}

void Configure::enableDict(int index, bool en)
{
    if (m_dictNodes[index].en != en) {
        SpinLock lock(m_cs);
        m_dictNodes[index].en = en;
        XMLElement* p = m_doc.RootElement()->FirstChildElement("dict");
        XMLElement* e = util::XMLUtil::Child(p, index);
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
    XMLElement* e = util::XMLUtil::Child(dictElement, item);
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
    if (m_setting.uilanID != id) {
        SpinLock lock(m_cs);
	    m_setting.uilanID = id;
        //stringstream  stream;
        //stream << id;
        XMLElement* e = XMLHandle(m_doc.RootElement()).FirstChildElement(XML_TAG_SETTING).ToElement();
        e->SetAttribute("uilan", m_setting.uilanID);
        m_dirty = true;
    }
}

void Configure::writeFontSize(int size)
{
    if (m_setting.fontsize != size) {
	    m_setting.fontsize = size;
        XMLElement* e = XMLHandle(m_doc.RootElement()).FirstChildElement(XML_TAG_SETTING).ToElement();
        e->SetAttribute("fontsize", m_setting.fontsize);
        m_dirty = true;
    }
}

void Configure::writeFont(string font)
{
    if (m_setting.font != font) {
	    m_setting.font = font;
        XMLElement* e = XMLHandle(m_doc.RootElement()).FirstChildElement(XML_TAG_SETTING).ToElement();
        e->SetAttribute("font", m_setting.font.c_str());
        m_dirty = true;
    }
}

void Configure::writeSystemTray(bool en)
{
    if (m_setting.bsystemTray != en) {
	m_setting.bsystemTray = en;
        XMLElement* e = XMLHandle(m_doc.RootElement()).FirstChildElement(XML_TAG_SETTING).ToElement();
        e->SetAttribute("systemtray", m_setting.bsystemTray);
        m_dirty = true;
    }
}

void Configure::writeCwsSelection(bool en)
{
    if (m_cws.bselection != en) {
        m_cws.bselection = en;
        XMLElement* e = XMLHandle(m_doc.RootElement()).FirstChildElement(XML_TAG_CWS).ToElement();
        e->SetAttribute("selection",  m_cws.bselection);
        m_dirty = true;
    }
}

void Configure::writeCwsClipboard(bool en)
{
    if (m_cws.bclipboard != en) {
        m_cws.bclipboard = en;
        XMLElement* e = XMLHandle(m_doc.RootElement()).FirstChildElement(XML_TAG_CWS).ToElement();
        e->SetAttribute("clipboard",  m_cws.bclipboard);
        m_dirty = true;
    }
}

void Configure::writeCwsShortcutKey(int shortcutKey)
{
    if (m_cws.shortcutKey != shortcutKey) {
        m_cws.shortcutKey = shortcutKey;
        XMLElement* e = XMLHandle(m_doc.RootElement()).FirstChildElement(XML_TAG_CWS).ToElement();
        e->SetAttribute("shortcutkey",  m_cws.shortcutKey);
        m_dirty = true;
    }
}

void Configure::writeCwsMouse(bool en)
{
    if (m_cws.bmouse != en) {
        m_cws.bmouse = en;
        XMLElement* e = XMLHandle(m_doc.RootElement()).FirstChildElement(XML_TAG_CWS).ToElement();
        e->SetAttribute("mouse",  m_cws.bmouse);
        m_dirty = true;
    }
}

void Configure::writeCwsEnable(bool en)
{
    if (m_cws.benable != en) {
        m_cws.benable = en;
        XMLElement* e = XMLHandle(m_doc.RootElement()).FirstChildElement(XML_TAG_CWS).ToElement();
        e->SetAttribute("enable",  m_cws.benable);
        m_dirty = true;
    }
}

void Configure::writeCwsAutoCloseEn(bool en)
{
    if (m_cws.autoCloseEn != en) {
        m_cws.autoCloseEn = en;
        XMLElement* e = XMLHandle(m_doc.RootElement()).FirstChildElement(XML_TAG_CWS).ToElement();
        e->SetAttribute("AutoCloseEnable", m_cws.autoCloseEn);
        m_dirty = true;
    }
}

void Configure::writeCwsAutoCloseInv(int inv)
{
    if (m_cws.autoCloseInv != inv) {
        m_cws.autoCloseInv = inv;
        XMLElement* e = XMLHandle(m_doc.RootElement()).FirstChildElement(XML_TAG_CWS).ToElement();
        e->SetAttribute("AutoCloseInterval", m_cws.autoCloseInv);
        m_dirty = true;
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

void Configure::reset()
{
    /* It seems no way to close xml in 'm_doc', so it is risk that overwrite.
       we will overwrite when start*/
    m_doc.RootElement()->SetAttribute("reset", true);
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
