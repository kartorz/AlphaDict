#ifndef _MESSAGEQUEUE_H_
#define _MESSAGEQUEUE_H_

#include "ThdCond.h"
#include "SpinLock.h"

#include <stddef.h>

#include <list>
#include <string>

#include <stdio.h>

/* form UI to system */
enum {
    MSG_DICT_QUERY = 0,
    MSG_DICT_INDEX_QUERY,
    MSG_SET_INDEXLIST,
	MSG_SET_DICTITEM,
    MSG_QUIT,
};

class Message
{
public:
    Message()
    :id(-1), iArg1(0), iArg2(0),
     fArg1(0.0),fArg2(0.0),
     dArg1(0.0),dArg2(0.0),
     pArg1(NULL), pArg2(NULL),
     strArg1(""), strArg2("")
     {
     }

    ~Message() { }

	int id;
    int iArg1;
    int iArg2;
    float fArg1;
    float fArg2;
    double dArg1;
    double dArg2;
    void* pArg1;
    void* pArg2;
    std::string strArg1;
    std::string strArg2;
};

class MessageQueue : public ThdCond
{
public:
    MessageQueue(std::string identi="");
    virtual ~MessageQueue();

    void push(Message& msg);
    void push(int id);
    void push(int id, int arg1, int arg2=0);
    void push(int id, float arg1, float arg2=0.0);
    void push(int id, double arg1, double arg2=0.00);
    void push(int id, std::string arg1, std::string arg2="");
    void push(int id, std::string& arg1, std::string& arg2);
    void push(int id, int arg1, void *arg2, void *arg3=NULL);

    bool pop(Message& msg, bool bwait=true);
    void flush();
    void flush(int id);

protected:
    virtual void onConsume(void *v);
    virtual void onProduce(void *v);
    virtual bool canConsume(void *v) { return !m_queue.empty(); }

private:
    std::list<Message> m_queue;
    SpinCriticalSection m_crs;
    std::string m_identify;
};


extern MessageQueue g_uiMessagerQ;
extern MessageQueue g_sysMessagerQ;

#endif
