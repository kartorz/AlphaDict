#include "DictManager.h"
#include "aldict/Aldict.h"
#include "alphadict.h"
#include "MessageQueue.h"

LookupTask::LookupTask(const string& input, int id, DictManager* dmgr)
:Task(0, false)
{
    m_dmgr = dmgr;
    m_dict = dmgr->m_Dicts[id];
    m_id = id;
    m_input = input;
}
 
void LookupTask::doWork()
{
    iDictItem item = m_dict->lookup(m_input);
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
:m_srcLan(""),m_detLan(""),m_input("")
{
}

void DictManager::load()
{
	iDict *alDict = new Aldict("../doc/alphadict_ch-en.ad");
    m_Dicts[0] = alDict;
    m_dictTotal = 1;

    if (m_dictTotal > DICT_MAX_OPEN) {
        m_dictTotal = DICT_MAX_OPEN;
        g_log.e("total of dicts is greater then DICT_MAX_OPEN\n");
    }

    for (int i=0; i<m_dictTotal; i++) {
        m_tasks[i] = NULL;
    }

	IndexList *list = m_Dicts[0]->getIndexList();
    g_uiMessagerQ.push(MSG_SET_INDEXLIST, -1, (void *)list); /* UI should delete arg1 */
	//alDict->lookup("放弃");
	//alDict->lookup("肚子");
}

DictManager::~DictManager()
{
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

        m_tasks[i] = new LookupTask(input, i, this);
        TaskManager::getInstance()->addTask(m_tasks[i], 0);
    }
}

void DictManager::onAddLookupResult(string& input, int which, iDictItem& item)
{
    MutexLock lock(m_cs);

    if (which != -1 && input == m_input) {
	    m_tasks[which] = NULL;
		iDictItem* arg1 = new iDictItem();
		*arg1 = item;
		g_uiMessagerQ.push(MSG_SET_DICTITEM, -1, (void *)arg1); /* UI should delete arg1 */
    } else {
        // be cancelled.
    }
}

IndexList* DictManager::getIndexList()
{
    return m_Dicts[0]->getIndexList();
}

void DictManager::onClick(int index, iIndexItem* item)
{
    iDictItem* arg1 = new iDictItem();
    *arg1 = m_Dicts[0]->onClick(index, item);
    g_uiMessagerQ.push(MSG_SET_DICTITEM, -1, (void *)arg1);    
}
