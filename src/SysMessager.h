#ifndef _SYSMESSAGER_H_
#define _SYSMESSAGER_H_

#include "Thread.h"
#include "MessageQueue.h"

class SysMessager: public Thread
{
public:
    SysMessager();
    virtual ~SysMessager();
    
    void processMessage();
protected:
    virtual void doWork();
    virtual void onStartup();

private:
    MessageQueue* m_msgQ;
};


#endif
