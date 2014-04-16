#include "DictManager.h"
#include "MessageQueue.h"
#include "SysMessager.h"
#include "Log.h"

SysMessager* SysMessager::getInstance()
{
    static SysMessager* pMsg = new SysMessager();
    return pMsg;
}

SysMessager::SysMessager(): Thread(0)
{
}

SysMessager::~SysMessager()
{
    g_sysMessagerQ.push(MSG_QUIT);
    join();
}

void SysMessager::onStartup()
{
}

void SysMessager::doWork()
{
    processMessage();
}

void SysMessager::processMessage()
{
    Message msg;
    bool ret = g_sysMessagerQ.pop(msg);
    if (ret == false)
        return;
    printf("processMessage\n");
    g_log(LOG_DEBUG,"SysMessager: processMessage() id:%d\n", msg.id);
	switch (msg.id) {
        case MSG_DICT_QUERY: {
            DictManager::getReference().lookup(msg.strArg1);
            break;
        }
        case MSG_DICT_INDEX_QUERY: {
            DictManager::getReference().onClick(msg.iArg1, (iIndexItem*)msg.pArg1);
            break;
        }

        case MSG_QUIT: {
            abort();
            break;
        }

        default:
            break;
    }
}
