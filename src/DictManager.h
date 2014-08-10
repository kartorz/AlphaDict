#ifndef _DICTMANAGER_H_
#define _DICTMANAGER_H_

#define DICT_MAX_OPEN 3

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

private:
    void onAddLookupResult(int which, DictItemList& items);
    bool loadDict(bool more=false);
    bool matchDict(const string& srcLan, const string& detLan);
    iDict* createHandleByDict(const string dictpath);
    iDict* createHandleByIdenitfier(const string identi);

    struct DictOpen{
        iDict *dict;
        Task  *task;
        int   dictId;
        int   flag;
        string pending;
    };
    
    DictOpen m_dictOpen[DICT_MAX_OPEN];
    bool m_bReload;
    int m_dictTotal;
    int m_indexListStart;
    IndexList  *m_indexList;

    MutexCriticalSection m_cs;
};

#endif
