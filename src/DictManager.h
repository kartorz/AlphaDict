#ifndef _DICTMANAGER_H_
#define _DICTMANAGER_H_

/* How many enabled dictionaries may be loaded at once.  The first matching
 * dictionary in the configured list is the main one (index, captured word and
 * status bar); the rest only add their entries to the search result. */
#define DICT_MAX_OPEN 8

#include "iDict.h"
#include "TaskManager.h"
#include "MutexLock.h"

#include <map>

using namespace std;

class DictManager;

class LookupTask: public Task
{
public:
      LookupTask(const string& input, int id, DictManager* dmgr);
      virtual ~LookupTask() {}
      virtual void doWork();
	  virtual void abort();
      virtual void onDrop();
private:
      iDict* m_dict;
      DictManager* m_dmgr;
      int m_id;
      string m_input;
};

class LoadDictTask: public Task
{
public:
      LoadDictTask():Task(0, false) {};
      virtual ~LoadDictTask() {}
      virtual void doWork();
};

#define QUERY_CAPWORD_FLAG  0x01
#define NO_CANDIDATEWORD_FLAG  0x02

class DictManager
{
friend class LookupTask;
friend class LoadDictTask;

public:
    static DictManager& getReference();
    DictManager();
    ~DictManager();
    void initialization();
    void lookup(const string& input, const int which=-1/*all*/, const int flags=0); /* async */
    void lookup(const string& input, const int which, DictItemList& items); /* sync*/

    int  getIndexList(IndexList& indexList, int start, int end, const string& startwith="");
    void onClick(int row, iIndexItem* item);
    void reloadDict();
    int  indexListSize();
    void setDictSrcLan(string& srclan);
    void setDictDetLan(string& detlan);
    bool hasDict() { return m_dictTotal > 0;}
    int  openDictId() const { return m_dictTotal > 0 ? m_dictOpen[0].dictId : -1; }

private:
    void onAddLookupResult(int which, DictItemList& items, bool lookupResult, Task* owner);
    bool loadDict(bool more=false);
    /*! Clear the reload gate; caller holds m_cs.  Returns true when a reload
     *  requested during this one must be scheduled once the lock is released. */
    bool endReload();
    bool matchDict(const string& srcLan, const string& detLan);
    iDict* createHandleByDict(const string dictpath);
    iDict* createHandleByIdenitfier(const string identi);
    bool lookupIgnoreGrammarCase(iDict* dict, const string& input, DictItemList& items);
    bool lookupIgnoreEnglishGrammar(iDict* dict, string input, DictItemList& items);

    struct DictOpen{
        iDict *dict;
        Task  *task;
        int   dictId;
        int   flag;
        string pending;
        /*! flags of the pending query: it may differ from the running task's
         *  (a captured-word lookup arriving while a normal search runs). */
        int   pendingFlag;
    };
    
    DictOpen m_dictOpen[DICT_MAX_OPEN];
    bool m_bReload;
    /*! True while a reload runs (also while it waits for lookups to stop).
     *  New lookups would race with the dictionary being torn down. */
    bool m_loading;
    /*! A language change arrived while a reload was running; run it again
     *  once the current one ends. */
    bool m_reloadPending;
    int m_dictTotal;
    int m_indexListStart;
    IndexList  *m_indexList;

    MutexCriticalSection m_cs;
};

#endif
