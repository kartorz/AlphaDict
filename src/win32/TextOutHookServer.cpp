#include "win32/TextOutHookServer.h"
#include "Log.h"

void injectDummy(HWND) { }
void uninjectDummy() { }
void getCaptureTextDummy(char *) {}

HINSTANCE g_hTextOutHook = NULL;

TextOutHookServer& TextOutHookServer::getReference()
{
    static TextOutHookServer textOutSrv;
    return textOutSrv;
}

TextOutHookServer::TextOutHookServer()
{
    m_hDriverInjecter = LoadLibraryEx(L"TextOutHookInjecter", NULL, 0);
    if (m_hDriverInjecter) {
        inject   = (injectDriver_t)GetProcAddress(m_hDriverInjecter, "injectTextOutDriver");
        uninject = (uninjectDriver_t)GetProcAddress(m_hDriverInjecter, "uninjectTextOutDriver");
        getCaptureText = (getCaptureText_t)GetProcAddress(m_hDriverInjecter, "getCaptureText");
    } else {
        inject = injectDummy;
        uninject = uninjectDummy;
        getCaptureText = getCaptureTextDummy;
    }
#if 0
    g_hTextOutHook = LoadLibrary(L"TextOutHook");
    if (g_hTextOutHook) {
 g_log.d("textouthook\n");
    } else {
g_log.d("textouthook failure \n");
    }
#endif
}

TextOutHookServer::~TextOutHookServer()
{
    if (m_hDriverInjecter) {
        uninject();
        FreeLibrary(m_hDriverInjecter);
    }
}
