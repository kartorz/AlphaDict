#include "alphadict.h"
#include "DictManager.h"
#include "TaskManager.h"
#include "SysMessager.h"
#include "Application.h"

#define MAX_WORK_THREAD 1

void InitTask::doWork()
{
    DictManager::getReference().load();
}

Application&  Application::getRefrence()
{
    static Application app;
	return app;
}

Application::Application():m_init(false)
{
}

Application::~Application()
{
}

void Application::initialization()
{
	g_log.setLevel(LOG_DEBUG);
    g_log(LOG_INFO, "Application initialization\n");

    TaskManager::getInstance()->start(MAX_WORK_THREAD);
    SysMessager::getInstance()->start();

    TaskManager::getInstance()->addTask(new InitTask(this), 20);
}

void Application::onTaskDone()
{
    m_init = true;
}
