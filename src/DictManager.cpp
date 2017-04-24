# ifdef WIN32
#include <Windows.h>
# endif

#include "DictManager.h"
#include "Application.h"
#include "Configure.h"
#include "aldict/Aldict.h"
#include "alphadict.h"
#include "MessageQueue.h"
#include "Util.h"
#include "Log.h"

LookupTask::LookupTask(const string& input, int id, DictManager* dmgr)
:Task(0, false)
{
    m_dmgr = dmgr;
    m_dict = dmgr->m_dictOpen[id].dict;
    m_id = id;
    m_input = input;
}

void LookupTask::doWork()
{
    DictItemList itemList;
    bool lookupResult = m_dict->lookup(m_input, itemList);
    if (lookupResult) {
        m_dmgr->onAddLookupResult(m_id, itemList, lookupResult);
    } else {
        DictItemList itemList2;
        lookupResult = m_dmgr->lookupIgnoreGrammarCase(m_dict, m_input, itemList2);
        if (lookupResult)
            m_dmgr->onAddLookupResult(m_id, itemList2, lookupResult);
        else {
            if (m_dmgr->m_dictOpen[m_id].flag  == QUERY_CAPWORD_FLAG) {
                if (m_input.length() - Util::stringCommonLen(m_input, itemList[0].word) > 3)
                    itemList.clear();
                else if (itemList[0].word.length() - Util::stringCommonLen(m_input, itemList[0].word) > 3)
                    itemList.clear();
            }

            m_dmgr->onAddLookupResult(m_id, itemList, lookupResult);
        }
    }
}

void LookupTask::abort()
{
    Task::abort();
}

void LoadDictTask::doWork()
{
    DictManager::getReference().loadDict();
    g_application.uiMessageQ()->push(MSG_RESET_INDEXLIST);
}

DictManager& DictManager::getReference()
{
    static DictManager mgr;
    return mgr;
}

DictManager::DictManager()
:m_dictTotal(0),m_indexListStart(-1),m_bReload(false)
{   
    for (int i=0; i<DICT_MAX_OPEN; i++) {
        m_dictOpen[i].task = NULL;
        m_dictOpen[i].dict = NULL;
        m_dictOpen[i].dictId = -1;
    }

    m_indexList = new IndexList();
}

DictManager::~DictManager()
{
}

void DictManager::initialization()
{
    Configure *config = g_application.m_configure;

    // Check new dictionary
    vector<struct DictNode>& nodeVec = config->m_dictNodes;
    for (int i=0; i<nodeVec.size(); i++) {
        if (nodeVec[i].open == "") {
            nodeVec[i].open = "none";
            iDict *dict;
            if ((dict = createHandleByDict(nodeVec[i].path)) != NULL) {
                nodeVec[i].open = dict->identifier();
                dict->getLanguage(nodeVec[i].srclan, nodeVec[i].detlan);
				dict->summary(nodeVec[i].summary);
            }
            config->writeDictItem(i);
        }
    }
    TaskManager::getInstance()->addTask(new LoadDictTask(), 0);
}

/* It takes lots of memory for loading a dictionay, actually it depends on 
 * the size of dictionay. Usually, we load only one, but allow user to enable 
 * more dictionaies.
 *
 * When change language comboxs, setting dictionary, should loadDict again.
 *
 * Todo: 
 *     Support multi-lookups at the same time.
 */
bool DictManager::loadDict(bool more)
{
    MutexLock lock(m_cs);

    vector<struct DictNode>& nodeVec = g_application.m_configure->m_dictNodes;
    vector<struct DictNode>::iterator iter;

    for (int i=0; i<m_dictTotal; i++) {
        if (m_dictOpen[i].task != NULL) {
            m_dictOpen[i].task->abort();
            do {
                Util::sleep(20);
            #ifdef _LINUX
                pthread_yield();
            #endif
            }while(m_dictOpen[i].task != NULL);
        }
    }

    for (int i=0; i<DICT_MAX_OPEN && m_dictOpen[i].dict!=NULL; i++) {
        delete m_dictOpen[i].dict;
        m_dictOpen[i].dict = NULL;
    }

    m_dictTotal = 0;

    for (iter=nodeVec.begin(); iter<nodeVec.end(); ++iter) {
        if (iter->en && matchDict(iter->srclan, iter->detlan)) {
            iDict *dict = createHandleByIdenitfier(iter->open);
            if (dict) {
                dict->load(iter->path);
                m_dictOpen[m_dictTotal].dict = dict;
                m_dictOpen[m_dictTotal].dictId = iter - nodeVec.begin();
                m_dictTotal++;
                g_sysLog(LOG_DEBUG,"DictManager::loadDict(%s)\n", iter->path.c_str());
                goto EXIT;
            }
        }
    }

EXIT:
    m_bReload = false;
    return m_dictTotal > 0;
}

void DictManager::reloadDict()
{
    if (!m_bReload) {
        m_bReload = true;
        g_sysLog(LOG_DEBUG,"DictManager::reloadDict\n");
        TaskManager::getInstance()->addTask(new LoadDictTask(), 0);
    }
}

void DictManager::lookup(const string& input, const int which, DictItemList& items)
{
    if (!input.empty()) {
        MutexLock lock(m_cs);

        if (m_dictTotal > 0) {
            if (which == -1) {
                for (int i = 0; i < m_dictTotal; i++) {
                    if (!m_dictOpen[i].dict->lookup(input, items)){
                        DictItemList items2;
                        if (lookupIgnoreGrammarCase(m_dictOpen[i].dict, input, items2))
                            items = items2;
                    }
                }
            } else {
                if (which < m_dictTotal) {
                    if (!m_dictOpen[which].dict->lookup(input, items)) {
                        DictItemList items2;
                        if (lookupIgnoreGrammarCase(m_dictOpen[which].dict, input, items2))
                            items = items2;
                    }
                }
            }
        }
    }
}

void DictManager::lookup(const string& input, const int which, const int flags)
{
    if (!input.empty()) {

        MutexLock lock(m_cs);

        if (m_dictTotal == 0)
            return;
        if (which == -1) {
            for (int i=0; i<m_dictTotal; i++) {
                if (m_dictOpen[i].task != NULL) {
                    //m_dictOpen[i].task->abort(); // need enter into onAddLookupResult
                    m_dictOpen[i].pending = input;
                    /* The Task will use the variable 'm_dictOpen',
                       can't assign m_dictOpen[i] a new pinter directly. */
                } else {
                    m_dictOpen[i].task = new LookupTask(input, i, this);
                    m_dictOpen[i].pending = "";
                    m_dictOpen[i].flag = flags;
                    TaskManager::getInstance()->addTask(m_dictOpen[i].task, 0);
                }
            }
        } else {
            if (which < m_dictTotal) {
                if (m_dictOpen[which].task != NULL) {
                    m_dictOpen[which].pending = input;
                } else {
                    m_dictOpen[which].task = new LookupTask(input, which, this);
                    m_dictOpen[which].pending = "";
                    m_dictOpen[which].flag = flags;
                    TaskManager::getInstance()->addTask(m_dictOpen[which].task, 0);
                    //g_sysLog(LOG_DEBUG,"lookup, flags (%d)\n", flags);
                }
            }
        }
    }
}

void DictManager::onAddLookupResult(int which, DictItemList& items, bool lookupResult)
{
    MutexLock lock(m_cs);

    if (true/*m_dictOpen[which].flag  != QUERY_CAPWORD_FLAG
      || (m_dictOpen[which].flag  == QUERY_CAPWORD_FLAG && lookupResult)*/) {
        DictItemList* arg1 = new DictItemList();
        if (items.size() > 0) {
            *arg1 = items;
            int did = m_dictOpen[which].dictId;
            (*arg1)[0].dictFileName = g_application.m_configure->m_dictNodes[did].name;
        }
        int msgid = m_dictOpen[which].flag == 
            QUERY_CAPWORD_FLAG ? MSG_SET_CAPWORD_DICTITEM : MSG_SET_DICTITEMS;
        g_application.uiMessageQ()->push(msgid, -1, (void *)arg1); /* UI should delete arg1 */
    }

    m_dictOpen[which].task = NULL; /* TaskManager will delete this pointer */
    //printf("{onAddLookupResult} %u\n", Util::getTimeMS());

    if (m_dictOpen[which].pending != "") {
        Message msg;
        msg.strArg1 = m_dictOpen[which].pending;
        msg.iArg1 = which;
        msg.iArg2 = m_dictOpen[which].flag;
        msg.id = MSG_DICT_PENDING_QUERY;
        m_dictOpen[which].pending = "";
        g_application.sysMessageQ()->push(msg);
    }
}

bool DictManager::lookupIgnoreGrammarCase(iDict* dict, const string& input, DictItemList& items)
{
    // Fisrt of all,  Change the first char's case.
    /* {input}: a lot of
     * {dict}: A lot of
     * or, reverse.
     */
    string strs[3];
    strs[0] = Util::stringCaseChange(input, 0, 1);
    if (strs[0] == input) // It's not a latin string.
        return false;
    if (dict->lookup(strs[0], items))
        return true;

    // Then,  convert input to lower case.
    /* {input}: BOY, BOy, bOY, etc.
     * {dict} : boy */
    items.clear();
    strs[1] = Util::stringCaseToLower(input);
    if (strs[1] != input && dict->lookup(strs[1], items))
        return true;

    // Then,  convert input to lower case.
    /* {input}: add
       {dict}: ADD */
    items.clear();
    strs[2] = Util::stringCaseToUpper(input);
    if (strs[2] != input && dict->lookup(strs[2], items))
        return true;

    items.clear();
    for (int i = 0; i < 3; i++) {
        if (lookupIgnoreEnglishGrammar(dict, strs[i], items))
           return true;
    }

    items.clear();
    return false;
}

bool DictManager::lookupIgnoreEnglishGrammar(iDict* dict, string input, DictItemList& items)
{
    string strtemp = "";

    if (input.length() > 2) {
        string subfix = input.substr(input.length()-1, 1);
        string sx[] = {"s", "d", "r", "\0"};
        for (int i = 0; sx[i] != "\0"; i++) {
            if (subfix.compare(sx[i]) == 0) {
                if (dict->lookup(input.substr(0, input.length()-1), items))
                    return true;
                break;
            }
        }
    }

    items.clear();
    if (input.length() > 3) {
        string subfix = input.substr(input.length()-2, 2);
        if (subfix.at(0) == '\'') {
            //"\'s", "\'t"  "n\'t",
            if (dict->lookup(input.substr(0, input.length()-2), items))
                return true;
        } else {
            string sx[] = {"es", "ed", "er", "en", "ly", "\0"};
            for (int i = 0; sx[i] != "\0"; i++) {
                if (subfix.compare(sx[i]) == 0) {
                    if (dict->lookup(input.substr(0, input.length()-2), items))
                        return true;

                    //  ies --> y
                    items.clear();
                    if (input.at(input.length() - 3 ) ==  'i') {
                        if (dict->lookup(input.substr(0, input.length()-3) + "y", items))
                            return true;
                    }

                    // er --> e
                    items.clear();
                    if (dict->lookup(input.substr(0, input.length() - 1), items))
                        return true;

                    strtemp = input.substr(0, input.length()-2);
                    break;
                }
            }
        }
    }

    items.clear();
    if (input.length() > 4) {
        bool bCheck = false;
        string subfix = input.substr(input.length()-3, 3);
        if (subfix.at(0) == '\'') {
            //"\'re", "\'ll", "\'ve"
            if (dict->lookup(input.substr(0, input.length()-3), items))
                return true;
        } else {
            string sx[] = {"est","ing", "\0"};
            for (int i = 0; sx[i] != "\0"; i++) {
                if (subfix.compare(sx[i]) == 0) {
                    if (dict->lookup(input.substr(0, input.length()-3), items))
                        return true;

                    items.clear();
                    if (dict->lookup(input.substr(0, input.length()-3) + "e", items))
                        return true;

                    // iest --> y
                    items.clear();
                    if (i == 0 && input.at(input.length()-4 ) ==  'i') {
                        if (dict->lookup(input.substr(0, input.length()-4) + "y", items))
                            return true;
                    }
                    strtemp = input.substr(0, input.length()-3);
                    break;
                }
            }
        }
    }

    items.clear();
    if (strtemp.length() > 2) {
        //stopped, written, hottest, etc
        if (strtemp.at(strtemp.length()-1) == strtemp.at(strtemp.length()-2)) {
            if (dict->lookup(strtemp.substr(0, strtemp.length()-1), items))
                    return true;
        }
    }

    items.clear();
    return false;
}

int DictManager::getIndexList(IndexList& indexList, int start, int end, const string& startwith)
{
    iDict* dict = m_dictOpen[0].dict;
    if (!dict)
        return 0;
    return dict->getIndexList(indexList, start, end, startwith);
}

int DictManager::indexListSize()
{
    iDict* dict = m_dictOpen[0].dict;
    if (!dict)
        return 0;
    return dict->indexListSize();
}

void DictManager::onClick(int row, iIndexItem* item)
{
    DictItemList* arg1 = new DictItemList();
    arg1->push_back(m_dictOpen[0].dict->onClick(row, item));
    int did = m_dictOpen[0].dictId;
    //(*arg1)[0].dictFileName = g_application.m_configure->m_dictNodes[did].name;
    g_application.uiMessageQ()->push(MSG_SET_DICTITEMS, -1, (void *)arg1);
}

void DictManager::setDictSrcLan(string& srclan)
{
    if (g_application.m_configure->m_srcLan.compare(srclan) != 0) {
        g_application.m_configure->writeSrcLan(srclan);
        //printf("setDictSrcLan %s \n", srclan.c_str());
        if (m_dictTotal > 0) {
            int did = m_dictOpen[0].dictId;
            string& srclan = g_application.m_configure->m_dictNodes[did].srclan;
            string& detlan = g_application.m_configure->m_dictNodes[did].detlan;
            //printf("setDictSrcLan %d %s, %s \n", did, srclan.c_str(), detlan.c_str());
            if (did != 0 || !matchDict(srclan, detlan))
                reloadDict();
        } else {
            reloadDict();
        }
    }
}

void DictManager::setDictDetLan(string& detlan)
{
    if (g_application.m_configure->m_detLan.compare(detlan) != 0) {
        g_application.m_configure->writeDetLan(detlan);
        //printf("setDictDetLan %s \n", detlan.c_str());
        if (m_dictTotal > 0) {
            int did = m_dictOpen[0].dictId;
            string& srclan = g_application.m_configure->m_dictNodes[did].srclan;
            string& detlan = g_application.m_configure->m_dictNodes[did].detlan;
            if (did!=0 || !matchDict(srclan, detlan))
                DictManager::getReference().reloadDict();
        }else {
            reloadDict();        
        }
    }
}

/* Check if this dictionary supports accroding lanauage */
bool DictManager::matchDict(const string& dictSrcLan, const string& dictDetLan)
{
    Configure *config = g_application.m_configure;
    bool src_match = false;
    bool det_match = false;

    if (dictSrcLan == "any")
        src_match = true;
    else {
        if (config->m_srcLan == "any")
            src_match = true;
        else if (dictSrcLan == config->m_srcLan)
            src_match = true;
    }

    if (dictDetLan == "any")
        det_match = true;
    else {
        if (config->m_detLan == "any")
            det_match = true;
        else if (dictDetLan == config->m_detLan)
            det_match = true;
    }
    g_sysLog(LOG_DEBUG, "{matchDict} xml:[%s -> %s], det:[%s, %s]\n",
        config->m_srcLan.c_str(), config->m_detLan.c_str(), dictSrcLan.c_str(), dictDetLan.c_str());
    if (src_match && det_match)
        return true;
    return false;
}

iDict* DictManager::createHandleByDict(const string dictpath)
{
    iDict *aldict = new Aldict();
    if (aldict->support(dictpath))
        return aldict;
    delete aldict;
    return NULL;
}

iDict* DictManager::createHandleByIdenitfier(const string identi)
{
    iDict *aldict = new Aldict();
    if (aldict->identifier() == identi)
        return aldict;
    delete aldict;
    return NULL;
}
