# ifdef WIN32
#include <Windows.h>
# endif
#ifdef _LINUX
#include <sched.h>      /* sched_yield() */
#endif

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
        m_dmgr->onAddLookupResult(m_id, itemList, lookupResult, this);
    } else {
        DictItemList itemList2;
        lookupResult = m_dmgr->lookupIgnoreGrammarCase(m_dict, m_input, itemList2);
        if (lookupResult)
            m_dmgr->onAddLookupResult(m_id, itemList2, lookupResult, this);
        else {
            if (m_dmgr->m_dictOpen[m_id].flag  == QUERY_CAPWORD_FLAG
                && !itemList.empty()) {
                if (m_input.length() - Util::stringCommonLen(m_input, itemList[0].word) > 3)
                    itemList.clear();
                else if (itemList[0].word.length() - Util::stringCommonLen(m_input, itemList[0].word) > 3)
                    itemList.clear();
            }

            m_dmgr->onAddLookupResult(m_id, itemList, lookupResult, this);
        }
    }
}

void LookupTask::abort()
{
    Task::abort();
}

void LookupTask::onDrop()
{
    MutexLock lock(m_dmgr->m_cs);
    /* loadDict() waits for these slots to become NULL, so a task that the
     * manager throws away without running must clear the pointer it was
     * registered under, otherwise the slot points to freed memory. */
    if (m_id >= 0 && m_id < DICT_MAX_OPEN
        && m_dmgr->m_dictOpen[m_id].task == static_cast<Task*>(this))
        m_dmgr->m_dictOpen[m_id].task = NULL;
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
:m_dictTotal(0),m_indexListStart(-1),m_bReload(false),m_loading(false),
 m_reloadPending(false)
{   
    for (int i=0; i<DICT_MAX_OPEN; i++) {
        m_dictOpen[i].task = NULL;
        m_dictOpen[i].dict = NULL;
        m_dictOpen[i].dictId = -1;
        m_dictOpen[i].flag = 0;
        m_dictOpen[i].pendingFlag = 0;
    }

    m_indexList = new IndexList();
}

DictManager::~DictManager()
{
}

void DictManager::initialization()
{
    Configure *config = g_application.m_configure;

    /* Check every dictionary, not only the new ones: configure.xml used to
     * be trusted blindy, so a wrong srclan/detlan there was never corrected.
     * The dictionary header is the real source of truth for its languages. */
    vector<struct DictNode>& nodeVec = config->m_dictNodes;
    for (int i=0; i<(int)nodeVec.size(); i++) {
        bool isNew = (nodeVec[i].open == "");
        bool changed = isNew;
        if (isNew)
            nodeVec[i].open = "none";

        iDict *dict = createHandleByDict(nodeVec[i].path);
        if (dict != NULL) {
            if (isNew)
                nodeVec[i].open = dict->identifier();

            /* refresh every time: the cached summary in configure.xml may
             * predate fixes, and the header is cheap to read. */
            string summary;
            dict->summary(summary);
            if (!summary.empty() && summary != nodeVec[i].summary) {
                nodeVec[i].summary = summary;
                changed = true;
            }

            string src, det;
            dict->getLanguage(src, det);
            if (!src.empty() && src != "any" && src != nodeVec[i].srclan) {
                g_sysLog(LOG_INFO, "fix dict srclan: %s %s -> %s\n",
                         nodeVec[i].path.c_str(), nodeVec[i].srclan.c_str(), src.c_str());
                nodeVec[i].srclan = src;
                changed = true;
            }
            if (!det.empty() && det != "any" && det != nodeVec[i].detlan) {
                g_sysLog(LOG_INFO, "fix dict detlan: %s %s -> %s\n",
                         nodeVec[i].path.c_str(), nodeVec[i].detlan.c_str(), det.c_str());
                nodeVec[i].detlan = det;
                changed = true;
            }
            delete dict;
        }

        if (changed)
            config->writeDictItem(i);
    }

    /* Claim the reload flag before scheduling the first load: if the user
     * changes the language while it runs, reloadDict() must queue instead of
     * starting a second loadDict() on the other worker thread. */
    m_bReload = true;
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
    /* Abort the running lookups first, then wait for them *without* holding
     * m_cs: onAddLookupResult() needs the same lock to publish its result and
     * clear the slot, so spinning on it while holding the lock deadlocks. */
    bool waiting[DICT_MAX_OPEN] = { false };
    {
        MutexLock lock(m_cs);
        /* One reload at a time: a second one would tear down the dictionaries
         * the first one is still building, and its wait snapshot would miss
         * the lookups the other one started. */
        if (m_loading) {
            g_sysLog(LOG_INFO, "loadDict: reload already running, ignored\n");
            /* The running reload may already have read the old direction, so
             * its endReload() must run us again instead of dropping this. */
            m_reloadPending = true;
            return false;
        }
        /* From here until the new dictionaries are in place, a lookup would
         * run against a dictionary that is about to be deleted. */
        m_loading = true;
        for (int i=0; i<m_dictTotal && i<DICT_MAX_OPEN; i++) {
            if (m_dictOpen[i].task != NULL) {
                m_dictOpen[i].task->abort();
                waiting[i] = true;
            }
        }
    }

    for (int i=0; i<DICT_MAX_OPEN; i++) {
        /* Bounded wait: a lookup normally clears its slot within a few ms.
         * The bound only guards against a task that never got scheduled. */
        int spins = 0;
        while (waiting[i]) {
            {
                MutexLock lock(m_cs);
                if (m_dictOpen[i].task == NULL)
                    break;
            }
            if (++spins > 300) {
                /* The task cannot be stopped and still holds the dictionary
                 * it was created with.  Deleting the dictionaries now would
                 * free memory a running lookup is using, so keep them and
                 * abandon this reload instead: the direction stays as it is
                 * and the empty result is visible to the user. */
                bool again;
                {
                    MutexLock lock(m_cs);
                    g_sysLog(LOG_INFO, "loadDict: lookup %d did not stop, reload abandoned\n", i);
                    again = endReload();
                }
                if (again)
                    TaskManager::getInstance()->addTask(new LoadDictTask(), 0);
                return false;
            }
            Util::sleep(10);
        }
    }

    bool again;
    {
    MutexLock lock(m_cs);

    vector<struct DictNode>& nodeVec = g_application.m_configure->m_dictNodes;
    vector<struct DictNode>::iterator iter;

    for (int i=0; i<DICT_MAX_OPEN; i++) {
        delete m_dictOpen[i].dict;
        m_dictOpen[i].dict = NULL;
        m_dictOpen[i].dictId = -1;
    }

    m_dictTotal = 0;

    /* Load every enabled dictionary matching the requested direction, in the
     * order the user arranged them.  The first one is the main dictionary:
     * it owns the index list, the captured-word popup and the status bar,
     * while the others simply append their entries to the search result.
     *
     * A direction without a matching dictionary is a normal state, not an
     * error: the language selection is left untouched and the UI shows an
     * empty index/result until the user picks a direction that has a dict. */
    for (iter=nodeVec.begin(); iter<nodeVec.end(); ++iter) {
        if (!iter->en)
            continue;
        if (!matchDict(iter->srclan, iter->detlan))
            continue;
        if (m_dictTotal >= DICT_MAX_OPEN) {
            g_sysLog(LOG_INFO, "too many dicts match, ignored: %s\n",
                     iter->path.c_str());
            continue;
        }

        iDict *dict = createHandleByIdenitfier(iter->open);
        if (dict == NULL)
            continue;
        if (!dict->load(iter->path)) {
            g_sysLog(LOG_INFO, "load dict failed: %s\n", iter->path.c_str());
            delete dict;
            continue;
        }

        m_dictOpen[m_dictTotal].dict = dict;
        m_dictOpen[m_dictTotal].dictId = iter - nodeVec.begin();
        m_dictTotal++;
        g_sysLog(LOG_DEBUG,"DictManager::loadDict(%s)\n", iter->path.c_str());
    }

    if (m_dictTotal == 0)
        g_sysLog(LOG_INFO, "no enabled dict for %s -> %s\n",
                 g_application.m_configure->m_srcLan.c_str(),
                 g_application.m_configure->m_detLan.c_str());

    again = endReload();
    }

    if (again) {
        g_sysLog(LOG_DEBUG, "DictManager::loadDict (queued reload)\n");
        TaskManager::getInstance()->addTask(new LoadDictTask(), 0);
    }
    return m_dictTotal > 0;
}

bool DictManager::endReload()
{
    m_bReload = false;
    m_loading = false;
    if (m_reloadPending) {
        m_reloadPending = false;
        m_bReload = true;
        return true;
    }
    return false;
}

void DictManager::reloadDict()
{
    bool schedule = false;
    {
        MutexLock lock(m_cs);
        if (!m_bReload) {
            m_bReload = true;
            schedule = true;
        } else {
            /* The running reload may already have read the old direction, so
             * do not drop the request: queue it for the moment that ends. */
            m_reloadPending = true;
        }
    }

    if (schedule) {
        g_sysLog(LOG_DEBUG,"DictManager::reloadDict\n");
        TaskManager::getInstance()->addTask(new LoadDictTask(), 0);
    } else {
        g_sysLog(LOG_DEBUG,"DictManager::reloadDict (queued)\n");
    }
}

void DictManager::lookup(const string& input, const int which, DictItemList& items)
{
    if (!input.empty()) {
        MutexLock lock(m_cs);

        if (!m_loading && m_dictTotal > 0) {
            if (which == -1) {
                for (int i = 0; i < m_dictTotal; i++) {
                    if (!m_dictOpen[i].dict->lookup(input, items)){
                        DictItemList items2;
                        if (lookupIgnoreGrammarCase(m_dictOpen[i].dict, input, items2))
                            items = items2;
                    }
                }
            } else {
                if (which >= 0 && which < m_dictTotal) {
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

        if (m_loading || m_dictTotal == 0)
            return;
        if (which == -1) {
            for (int i=0; i<m_dictTotal; i++) {
                if (m_dictOpen[i].task != NULL) {
                    //m_dictOpen[i].task->abort(); // need enter into onAddLookupResult
                    m_dictOpen[i].pending = input;
                    m_dictOpen[i].pendingFlag = flags;
                    /* The Task will use the variable 'm_dictOpen',
                       can't assign m_dictOpen[i] a new pinter directly. */
                } else {
                    m_dictOpen[i].task = new LookupTask(input, i, this);
                    m_dictOpen[i].pending = "";
                    m_dictOpen[i].flag = flags;
                    /* A refused task is deleted by the manager, so the slot
                     * must not keep pointing at it. */
                    if (!TaskManager::getInstance()->addTask(m_dictOpen[i].task, 0))
                        m_dictOpen[i].task = NULL;
                }
            }
        } else {
            if (which >= 0 && which < m_dictTotal) {
                if (m_dictOpen[which].task != NULL) {
                    m_dictOpen[which].pending = input;
                    m_dictOpen[which].pendingFlag = flags;
                } else {
                    m_dictOpen[which].task = new LookupTask(input, which, this);
                    m_dictOpen[which].pending = "";
                    m_dictOpen[which].flag = flags;
                    /* A refused task is deleted by the manager, so the slot
                     * must not keep pointing at it. */
                    if (!TaskManager::getInstance()->addTask(m_dictOpen[which].task, 0))
                        m_dictOpen[which].task = NULL;
                    //g_sysLog(LOG_DEBUG,"lookup, flags (%d)\n", flags);
                }
            }
        }
    }
}

void DictManager::onAddLookupResult(int which, DictItemList& items, bool lookupResult,
                                    Task* owner)
{
    MutexLock lock(m_cs);

    /* The dictionary set may have changed while this lookup was running, so
     * the slot can now belong to another dictionary or to nobody at all.
     * Publishing such a stale result would mix it into the new results. */
    if (which < 0 || which >= DICT_MAX_OPEN || m_dictOpen[which].task != owner)
        return;

    if (true/*m_dictOpen[which].flag  != QUERY_CAPWORD_FLAG
      || (m_dictOpen[which].flag  == QUERY_CAPWORD_FLAG && lookupResult)*/) {
        DictItemList* arg1 = new DictItemList();
        if (items.size() > 0) {
            *arg1 = items;
            int did = m_dictOpen[which].dictId;
            if (did >= 0 && did < (int)g_application.m_configure->m_dictNodes.size())
                (*arg1)[0].dictFileName = g_application.m_configure->m_dictNodes[did].name;
        }
        int msgid = m_dictOpen[which].flag == 
            QUERY_CAPWORD_FLAG ? MSG_SET_CAPWORD_DICTITEM : MSG_SET_DICTITEMS;
        /* Carry the dict index so the UI can merge the results of several
         * dictionaries in list order instead of overwriting them. */
        g_application.uiMessageQ()->push(msgid, which, (void *)arg1); /* UI should delete arg1 */
    }

    m_dictOpen[which].task = NULL; /* TaskManager will delete this pointer */
    //printf("{onAddLookupResult} %u\n", Util::getTimeMS());

    if (m_dictOpen[which].pending != "") {
        Message msg;
        msg.strArg1 = m_dictOpen[which].pending;
        msg.iArg1 = which;
        msg.iArg2 = m_dictOpen[which].pendingFlag;
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
    /* The UI thread reads the index while a reload may be replacing m_dictOpen:
     * without the lock a dictionary could be deleted (closing its file) in the
     * middle of this call.  A reload holds the lock while it parses the index
     * files (about 20 ms for the two bundled dictionaries), so the UI waits
     * instead of blanking the index for a whole reload, waiting period
     * included. */
    MutexLock lock(m_cs);
    iDict* dict = m_dictOpen[0].dict;
    if (!dict)
        return 0;
    return dict->getIndexList(indexList, start, end, startwith);
}

int DictManager::indexListSize()
{
    MutexLock lock(m_cs);
    iDict* dict = m_dictOpen[0].dict;
    if (!dict)
        return 0;
    return dict->indexListSize();
}

void DictManager::onClick(int row, iIndexItem* item)
{
    DictItemList* arg1 = new DictItemList();
    {
        /* The index belongs to the first dictionary.  A click can still be in
         * flight after a language change left no matching dictionary, and the
         * dictionary may be torn down concurrently, so take the lock and answer
         * with an empty list when there is nothing to ask. */
        MutexLock lock(m_cs);
        if (!m_loading && m_dictTotal > 0 && m_dictOpen[0].dict != NULL)
            arg1->push_back(m_dictOpen[0].dict->onClick(row, item));
    }
    g_application.uiMessageQ()->push(MSG_SET_DICTITEMS, -1, (void *)arg1);
}

void DictManager::setDictSrcLan(string& srclan)
{
    if (g_application.m_configure->m_srcLan.compare(srclan) != 0) {
        g_application.m_configure->writeSrcLan(srclan);
        /* Always reload: the matching set depends on the whole direction, and
         * the previously opened dicts may no longer be the right ones. */
        reloadDict();
    }
}

void DictManager::setDictDetLan(string& detlan)
{
    if (g_application.m_configure->m_detLan.compare(detlan) != 0) {
        g_application.m_configure->writeDetLan(detlan);
        reloadDict();
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
