#include "DictManager.h"
#include "MessageQueue.h"
#include "Application.h"
#include "Configure.h"
#include "SysMessager.h"
#include "Log.h"


SysMessager::SysMessager(): Thread(0)
{
}

SysMessager::~SysMessager()
{
    g_sysMessageQ.push(MSG_QUIT);
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
    bool ret = g_sysMessageQ.pop(msg);
    if (ret == false) {
        printf("{SysMessager} no message, exit\n");
        return;
    }
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
        case MSG_SET_SRCLAN: {
            Application::getRefrence().m_configure->writeSrcLan(msg.strArg1);
            DictManager::getReference().setSrcLan(msg.strArg1);
            break;
        }
        case MSG_SET_DETLAN: {
            Application::getRefrence().m_configure->writeDetLan(msg.strArg1);
            DictManager::getReference().setDetLan(msg.strArg1);
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
