
#include "DictManager.h"
#include "Application.h"
#include "Configure.h"
#include "aldict/Aldict.h"
#include "alphadict.h"
#include "MessageQueue.h"
#include "Util.h"

#include <unistd.h>

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
    iDictItem item = m_dict->lookup(m_input);
    m_dmgr->onAddLookupResult(m_id, item);
}

void LookupTask::abort()
{
    Task::abort();
}

void LoadDictTask::doWork()
{
    if (DictManager::getReference().loadDict())
        g_uiMessageQ.push(MSG_RESET_INDEXLIST);
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
                usleep(1000*20);
                pthread_yield();
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
        TaskManager::getInstance()->addTask(new LoadDictTask(), 0);
    }
}

void DictManager::lookup(const string& input, int which)
{
    MutexLock lock(m_cs);
    if (m_dictTotal == 0)
        return;
    if (which == -1) {
        for (int i=0; i<m_dictTotal; i++) {
            if (m_dictOpen[i].task != NULL) {
                m_dictOpen[i].task->abort();
                m_dictOpen[i].pending = input; 
               /* The Task will use the variable 'm_dictOpen', 
                  can't assign m_dictOpen[i] a new pinter directly. */
            } else {
                m_dictOpen[i].task = new LookupTask(input, i, this);
                m_dictOpen[i].pending = ""; 
                TaskManager::getInstance()->addTask(m_dictOpen[i].task, 0);
            }
        }
    } else {
        if (which < m_dictTotal) { 
            m_dictOpen[which].task = new LookupTask(input, which, this);
            m_dictOpen[which].pending = ""; 
            TaskManager::getInstance()->addTask(m_dictOpen[which].task, 0);
        }
    }
}

void DictManager::onAddLookupResult(int which, iDictItem& item)
{
    if (!m_dictOpen[which].task->isAbort()) {
        iDictItem* arg1 = new iDictItem();
        *arg1 = item;
        int did = m_dictOpen[which].dictId;
	    (*arg1).dictname = g_application.m_configure->m_dictNodes[did].name;
        g_uiMessageQ.push(MSG_SET_DICTITEM, -1, (void *)arg1); /* UI should delete arg1 */
        m_dictOpen[which].task = NULL; /* TaskManager will delete this pointer */
        //printf("{onAddLookupResult} %u\n", Util::getTimeMS());
    } else {
        if (m_dictOpen[which].pending != "") {                
            Message msg;
            msg.strArg1 = m_dictOpen[which].pending;
            msg.iArg1 = which;
            msg.id = MSG_DICT_PENDING_QUERY;
            m_dictOpen[which].task = NULL; /* TaskManager will delete this pointer */
            g_sysMessageQ.push(msg);
        }
    }
}

int DictManager::getIndexList(IndexList& indexList, int start, int end)
{
    iDict* dict = m_dictOpen[0].dict;
    if (!dict)
        return 0;
    return dict->getIndexList(indexList, start, end);
}

int DictManager::indexListSize()
{
    iDict* dict = m_dictOpen[0].dict;
    if (!dict)
        return 0;
    return dict->indexListSize();
}

void DictManager::onClick(int index, iIndexItem* item)
{
    iDictItem* arg1 = new iDictItem();
    *arg1 = m_dictOpen[0].dict->onClick(index, item);
    int did = m_dictOpen[0].dictId;
    (*arg1).dictname = g_application.m_configure->m_dictNodes[did].name;
    g_uiMessageQ.push(MSG_SET_DICTITEM, -1, (void *)arg1);
}

void DictManager::setDictSrcLan(string& srclan)
{
    if (g_application.m_configure->m_srcLan.compare(srclan) != 0) {
        g_application.m_configure->writeSrcLan(srclan);
        if (m_dictTotal > 0) {
            int did = m_dictOpen[0].dictId;
            string& srclan = g_application.m_configure->m_dictNodes[did].srclan;
            string& detlan = g_application.m_configure->m_dictNodes[did].detlan;
            if (!matchDict(srclan, detlan))
                DictManager::getReference().reloadDict();
        }
    }
}

void DictManager::setDictDetLan(string& detlan)
{
    if (g_application.m_configure->m_detLan.compare(detlan) != 0) {
        g_application.m_configure->writeDetLan(detlan);
        if (m_dictTotal > 0) {
            int did = m_dictOpen[0].dictId;
            string& srclan = g_application.m_configure->m_dictNodes[did].srclan;
            string& detlan = g_application.m_configure->m_dictNodes[did].detlan;
            if (!matchDict(srclan, detlan))
                DictManager::getReference().reloadDict();
        }
    }
}

/* Check if this dictionary supports accroding lanauage */
bool DictManager::matchDict(const string& dictSrcLan, const string& dictDetLan)
{
    Configure *config = g_application.m_configure;

    if (dictSrcLan == "any") {
        if (dictDetLan == "any")
            return true;
        if (dictDetLan == config->m_detLan)
            return true;
        return false;
    } else {
        if (dictSrcLan != config->m_srcLan)
            return false;
        if (dictDetLan == "any")
            return true;
        if (dictDetLan == config->m_detLan)
            return true;
        return false;
    }
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
