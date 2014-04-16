#ifndef _DICTMANAGER_H_
#define _DICTMANAGER_H_

#define DICT_MAX_OPEN  10

#include "iDict.h"
#include "TaskManager.h"
#include "MutexLock.h"

using namespace std;

class DictManager;

class LookupTask: public Task
{
public:
      LookupTask(const string& input, int id, DictManager* dmgr);
      ~LookupTask() {}
      virtual void doWork();
      virtual void abort();
private:
      iDict* m_dict;
      DictManager* m_dmgr;
      int m_id;
      string m_input;
};


class DictManager
{
friend class LookupTask;

public:
    static DictManager& getReference();
    DictManager();
    ~DictManager();

	void load();
	void lookup(const string& input);
	void lookup(const string& srcLan, const string& detLan, const string& input);

	IndexList* getIndexList();
	void onClick(int index, iIndexItem* item);

    void setSrcLan(const string& lan) { m_srcLan = lan; }
    void setDetLan(const string& lan) { m_detLan = lan; }

    void onAddLookupResult(string& input, int which, iDictItem& item);

private:
    iDict* m_Dicts[DICT_MAX_OPEN];
    Task* m_tasks[DICT_MAX_OPEN];

    int m_dictTotal;

    string m_srcLan;
    string m_detLan;
    string m_input;

    MutexCriticalSection m_cs;
};

#endif
