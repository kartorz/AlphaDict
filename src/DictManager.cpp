#include "DictManager.h"
#include "aldict/Aldict.h"
#include "alphadict.h"
#include "MessageQueue.h"

LookupTask::LookupTask(const string& input, int id, DictManager* dmgr)
:Task(0, false)
{
    m_dmgr = dmgr;
    m_dict = dmgr->m_dicts[id];
    m_id = id;
    m_input = input;
}
 
void LookupTask::doWork()
{
    iDictItem item = m_dict->lookup(m_input);
    item.dictIdentifier = m_dict->identifier();
    if (!isAbort())
        m_dmgr->onAddLookupResult(m_input, m_id, item);
}

void LookupTask::abort()
{
    //Task.abort();
    m_id = -1;
}

DictManager& DictManager::getReference()
{
    static DictManager mgr;
    return mgr;
}

DictManager::DictManager()
:m_srcLan(""),m_detLan(""),m_input(""),m_dictTotal(0)
{
    iDict *aldict = new Aldict();
    m_dictMap[aldict->identifier()] = aldict;
    
    for (int i=0; i<m_dictTotal; i++) {
        m_tasks[i] = NULL;
    }
}

DictManager::~DictManager()
{
}

bool DictManager::load(const string& dictname, string *dictidenti)
{
    if (m_dictTotal > DICT_MAX_OPEN) {
        m_dictTotal = DICT_MAX_OPEN;
        g_log.e("total of dicts is greater then DICT_MAX_OPEN\n");
        return false;
    }

    map<string, iDict*>::iterator iter;
    for (iter = m_dictMap.begin(); iter != m_dictMap.end(); iter++) {
	    iDict *dict = iter->second;
        if (dict->support(dictname)) {
            dict->load(dictname);
            m_dicts[m_dictTotal++] = dict;
            *dictidenti = dict->identifier();
            return true;
        }
    }
    return false;
}

bool DictManager::load(const string& dictname, const string& dictidenti)
{
    if (m_dictTotal > DICT_MAX_OPEN) {
        m_dictTotal = DICT_MAX_OPEN;
        g_log.e("total of dicts is greater then DICT_MAX_OPEN\n");
        return false;
    }

    if (m_dictMap.find(dictidenti) != m_dictMap.end()) {
	    iDict *dict = m_dictMap[dictidenti];
        dict->load(dictname);
        m_dicts[m_dictTotal++] = dict;
        return true;
    }
    return false;
}

void DictManager::sendIndexListMessage()
{
    iDict* dict = findIndexDict();
    if (dict) {
        IndexList *list = dict->getIndexList();
        g_uiMessageQ.push(MSG_SET_INDEXLIST, -1, (void *)list);
    }
}

void DictManager::lookup(const string& srcLan, const string& detLan, const string& input)
{
    MutexLock lock(m_cs);

    m_srcLan = srcLan;
    m_detLan = detLan;
    lookup(input);
}

void DictManager::lookup(const string& input)
{
    MutexLock lock(m_cs);

    m_input = input;
    for (int i=0; i<m_dictTotal; i++) {
        if (m_tasks[i] != NULL)
            m_tasks[i]->abort();

        if (m_dicts[i]->canLookup(m_srcLan, m_detLan)) {            
            m_tasks[i] = new LookupTask(input, i, this);
            TaskManager::getInstance()->addTask(m_tasks[i], 0);
        }
    }
}

void DictManager::onAddLookupResult(string& input, int which, iDictItem& item)
{
    MutexLock lock(m_cs);

    if (which != -1 && input == m_input) {
	    m_tasks[which] = NULL;
		iDictItem* arg1 = new iDictItem();
		*arg1 = item;
		g_uiMessageQ.push(MSG_SET_DICTITEM, -1, (void *)arg1); /* UI should delete arg1 */
    } else {
        // be cancelled.
    }
}

IndexList* DictManager::getIndexList()
{
    iDict* dict = findIndexDict();
    if (dict) {
        return dict->getIndexList();
    }
    return NULL;
}

void DictManager::onClick(int index, iIndexItem* item)
{
    iDictItem* arg1 = new iDictItem();
    *arg1 = m_dicts[0]->onClick(index, item);
    g_uiMessageQ.push(MSG_SET_DICTITEM, -1, (void *)arg1);    
}

iDict* DictManager::findIndexDict()
{
    return m_dicts[0];
}
