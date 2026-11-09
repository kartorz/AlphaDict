# ifdef WIN32
#include <windows.h>
#include "win32/posixtoiso.h"
# endif

# ifdef _LINUX
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#endif

#include <stdlib.h>

#include "Application.h"
#include "Log.h"
#include "DictManager.h"
#include "TaskManager.h"
#include "SysMessager.h"
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
    // Have to do 'Application::stop()' before, or 'WaitForSingleObject' will be blocked.
    delete TaskManager::getInstance();

    delete m_configure;

    /* Some tasks may push message to message queue, before delete MessageQ, 
       all the tasks should be stopped. */
    delete m_uiMessageQ;
    delete m_sysMessageQ;
    //printf("~Application");
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

/* $XDG_RUNTIME_DIR is per login session; /tmp only for a bare console. */
static string instanceDir()
{
    const char* xdg = getenv("XDG_RUNTIME_DIR");
    if (xdg != NULL && xdg[0] == '/') {
        string dir(xdg);
        while (dir.size() > 1 && dir[dir.size() - 1] == '/')
            dir.erase(dir.size() - 1);
        return dir;
    }

    string path;
    Util::tempDir(path);
    return path;
}

string Application::pidPath()
{
    string path = instanceDir();
    path += "/alphadict";
# ifdef _LINUX
    path += "-";
    path += Util::intToString((int)getuid());
# endif
    path += ".pid";
    return path;
}

string Application::instanceServerName()
{
# ifdef _LINUX
    string name = instanceDir();
    name += "/alphadict-";
    name += Util::intToString((int)getuid());
    name += ".sock";
    return name;
# else
    /* Windows uses a named pipe. */
    return string("AlphaDict");
# endif
}

bool Application::existProcess()
{
    string path = pidPath();
    FILE* fpid = fopen(path.c_str(), "r");
    if (fpid != NULL) {
        char buf[8] = {'\0'};
        size_t nread = fread(buf, 1, 7, fpid);
        fclose(fpid);
        if (nread == 0)
            return false;
        int pid = Util::stringToInt(buf);
        //printf("existProcess, pidbuf:%s, pid:%d\n", buf, pid);
        if (pid > 100) {
# ifdef _LINUX
            /* A stale file left by a crash must not block startup. */
            if (pid == (int)Util::curpid())
                return false;
            if (kill((pid_t)pid, 0) != 0 && errno == ESRCH) {
                unlink(path.c_str());
                return false;
            }
# endif
            return true;
        }
    }
    return false;
}

void Application::writePidFile()
{
    string path = pidPath();
    FILE* fpid = fopen(path.c_str(), "w");
    if (fpid != NULL) {
        string strpid = Util::intToString(Util::curpid());
        //printf("writePidFile, strpid:%s\n", strpid.c_str());
        fwrite(strpid.c_str(), 1, strpid.size(), fpid);
        fclose(fpid);
    }
}

// Can't put stop or ~Application(), when exit by 'on_exit'
// will enter this functions;
void Application::delPidFile()
{
    //printf("delPidFile");
    string path = pidPath();
    FILE* fpid = fopen(path.c_str(), "w");
    if (fpid != NULL) {
        fclose(fpid);
        unlink(path.c_str());
    }
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
