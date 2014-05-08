#include "DictManager.h"
#include "MessageQueue.h"
#include "Application.h"
#include "Configure.h"
#include "SysMessager.h"
#include "Log.h"


SysMessager::SysMessager(): Thread(0),m_bReloadDict(false)
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
        //printf("{SysMessager} no message, exit\n");
        return;
    }
    //g_log(LOG_DEBUG,"SysMessager: processMessage() id:%d\n", msg.id);
	switch (msg.id) {
        case MSG_DICT_QUERY: {
            DictManager::getReference().lookup(msg.strArg1);
            break;
        }
        
        case MSG_DICT_PENDING_QUERY: {
            DictManager::getReference().lookup(msg.strArg1, msg.iArg1);
            break;
        }

        case MSG_DICT_INDEX_QUERY: {
            DictManager::getReference().onClick(msg.iArg1, (iIndexItem*)msg.pArg1);
            break;
        }

        case MSG_SET_SRCLAN: {
            DictManager::getReference().setDictSrcLan(msg.strArg1);
            break;
        }

        case MSG_SET_DETLAN: {
            DictManager::getReference().setDictDetLan(msg.strArg1);
            break;
        }

        case MSG_SET_DICTEN: {
            // When UI init, will send a invalid message.
            if (g_application.m_configure->m_dictNodes[msg.iArg1].en != msg.iArg2) {
			    g_application.m_configure->enableDict(msg.iArg1, msg.iArg2);
			    //DictManager::getReference().reloadDict();
				m_bReloadDict = true;
            }
            break;
        }

        case MSG_MOVE_DICTITEM: {
			g_application.m_configure->moveDictItem(msg.iArg1, msg.iArg2);
			//DictManager::getReference().reloadDict();
			m_bReloadDict = true;
            break;
        }
	    case MSG_CURTAB_PGDICT: {
			if (m_bReloadDict) {
			    m_bReloadDict = false;
			    DictManager::getReference().reloadDict();
			}
			break;
		}
	    case MSG_QUIT: {
            abort();
            break;
        }

        default:
            break;
    }
	//printf("Message done\n");
}
