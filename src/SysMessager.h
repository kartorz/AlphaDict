#ifndef _SYSMESSAGER_H_
#define _SYSMESSAGER_H_

#include "Thread.h"
#include "MessageQueue.h"

class SysMessager: public Thread
{
public:
    SysMessager(MessageQueue* queuq);
    virtual ~SysMessager();
    
    void processMessage();
    virtual void stop();

protected:
    virtual void doWork();
    virtual void onStartup();
    virtual void onExit();

private:
    bool m_bReloadDict;
    MessageQueue* m_msgQ;
};


#endif
