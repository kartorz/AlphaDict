#ifndef _CONFIGURE_H_
#define _CONFIGURE_H_

#include "tinyxml2/tinyxml2.h"

#include <string>
#include <vector>
#include <map>

using namespace std;
using namespace tinyxml2;

struct DictNode {
    string name;
    string open;
    string srclan;
    string detlan;
    bool   en;
    string path;
    string summary;
};

class Configure
{
public:
    static Configure& getRefrence();
    Configure();
    ~Configure();

    void initialization();
    void scanDictDir();

    void moveDictItem(int index, bool down=true);
    void enableDict(int index, bool en=true);
    void writeDictItem(int item);
    void writeSrcLan(const string& lan);
    void writeDetLan(const string& lan);
    void writeXml();

    vector<string> m_languages;
    string m_dataDir;
    string m_homeDir;
    string m_configFile;
    string m_srcLan;
    string m_detLan;
    vector<struct DictNode> m_dictNodes;

private:
    void   load(const string& xmlpath);
    void   loadLanguage();
    void   scanDictDir(const string& path);
    bool   findDict(string name);
    string dictItemName(int item);

    XMLDocument m_doc;
    bool m_dirty;
    SpinCriticalSection m_cs;
};

#endif
