# ifdef WIN32
#include <windows.h>
# endif

#include <stdlib.h>

#include "Application.h"
#include "Log.h"
#include "DictManager.h"
#include "TaskManager.h"
#include "SysMessager.h"
#include "Util.h"
#include "alphadict.h"

Application g_application;

void InitTask::doWork()
{
    //equal to onTaskDone
}

// loop time is 5s.
void SlowJob::doWork()
{
    //printf("slowjob dowork\n");
    //g_sysLog.d("slowjob dowork\n");
    m_owner->slowJob();
}

Application::Application():m_init(false)
{
    m_sysMessageQ = new MessageQueue("ui");   /* UI system should listen this */
    m_uiMessageQ  = new MessageQueue("sys");  /* SysMessager should listen this */
    m_sysMessager = new SysMessager(m_sysMessageQ);
    m_configure   = new Configure();
}

Application::~Application()
{
    delete m_sysMessager;

    // Any thread invoking TaskManager should already stop. (SysMessager)
    delete TaskManager::getInstance();

    delete m_configure;

    /* Some tasks may push message to message queue, before delete MessageQ, 
       all the tasks should be stopped. */
    delete m_uiMessageQ;
    delete m_sysMessageQ;
    g_sysLog.d("~Application\n");
}

int Application::start()
{
    int ret = initialization();
    TaskManager::getInstance()->addTask(new SlowJob(this), 0);
    return ret;
}

int Application::initialization()
{
    int ret = 0;
    TaskManager::getInstance()->start(MAX_WORK_THREAD);
    g_sysLog.setLevel(LOG_DEBUG);
    g_sysLog(LOG_INFO, "Application initialization\n");

    m_sysMessager->start();
    if ((ret = m_configure->initialization()) != 0)
        return ret;

    DictManager::getReference().initialization();
    m_init = true;
    //TaskManager::getInstance()->addTask(new InitTask(this), 0);
    return ret;
}

void Application::stop()
{
    /* Some tasks may send message, invoke g_application, So,
       The first thing to do is stopping all the tasks */
    TaskManager::getInstance()->stop();
    m_sysMessager->stop();
}

void Application::onTaskDone()
{
#if 0
    //DictManager::getReference().initialization();
    m_init = true;
    TaskManager::getInstance()->addTask(new SlowJob(this), 0);
    printf("Application onTaskDone\n");
#endif
}

void Application::slowJob(void)
{
    m_configure->writeXml();
}
