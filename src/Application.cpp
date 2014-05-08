#define _APPLICATION_CPP_

#include "DictManager.h"
#include "TaskManager.h"
#include "SysMessager.h"
#include "Util.h"
#include "Application.h"
#include "alphadict.h"
#include "config.h"

#include <stdlib.h>

void InitTask::doWork()
{
    //equal to onTaskDone
}

// loop time is 5s.
void SlowJob::doWork()
{
    printf("slowjob dowork\n");
    m_owner->slowJob();
}

Application::Application():m_init(false)
{
    m_sysMessager = new SysMessager();
    m_configure =  new Configure();
    TaskManager::getInstance()->start(MAX_WORK_THREAD);
}

Application::~Application()
{
    delete TaskManager::getInstance();
    delete m_sysMessager;
    delete m_configure;
}

void Application::initialization()
{
	g_log.setLevel(LOG_DEBUG);
    g_log(LOG_INFO, "Application initialization\n");

    m_sysMessager->start();
    m_configure->initialization();
    DictManager::getReference().initialization();
    m_init = true;
    //TaskManager::getInstance()->addTask(new SlowJob(this), 0);
    //TaskManager::getInstance()->addTask(new InitTask(this), 0);
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
