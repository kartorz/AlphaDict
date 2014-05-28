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

enum UILanID {
    UILAN_EN = 0,
    UILAN_CN,
};

class Configure
{
public:
    static Configure& getRefrence();
    Configure();
    ~Configure();

    int  initialization();
    void scanDictDir(vector<string>& dictFiles);

    void moveDictItem(int index, bool down=true);
    void enableDict(int index, bool en=true);
    void writeDictItem(int item);
    void writeSrcLan(const string& lan);
    void writeDetLan(const string& lan);
    void writeUILanID(int id);
    void writeXml();

    vector<string> m_languages;
    string m_dataDir;
    string m_homeDir;
    string m_configFile;
    string m_srcLan;
    string m_detLan;
    int m_uilanID;

    vector<struct DictNode> m_dictNodes;

    string getVBPath() { return m_homeDir+"/vbook.xml";}

private:
    int    load(const string& xmlpath);
    void   loadLanguage();
    void   scanDictDir(const string& path, vector<string>& dictFiles);
    int    findDict(const string& path, const vector<string>& dictFiles);

    tinyxml2::XMLDocument m_doc;
    bool m_dirty;
    SpinCriticalSection m_cs;
};

#endif
