#ifndef _SYSMESSAGER_H_
#define _SYSMESSAGER_H_

#include "Thread.h"
#include "MessageQueue.h"

class SysMessager: public Thread
{
public:
    SysMessager();
    SysMessager(MessageQueue* queuq);
    virtual ~SysMessager();
    
    void processMessage();
    MessageQueue* m_msgQ;

protected:
    virtual void doWork();
    virtual void onStartup();

private:
    bool m_bReloadDict;
};


#endif
