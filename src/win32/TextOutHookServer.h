#ifndef _TEXTOUTHOOKSERVER_H_
#define _TEXTOUTHOOKSERVER_H_
#include <Windows.h>

typedef void (*injectDriver_t)(HWND);
typedef void (*uninjectDriver_t)();
typedef void (*getCaptureText_t)(char *);

class TextOutHookServer
{
public:
    static TextOutHookServer& getReference();
    TextOutHookServer();
    ~TextOutHookServer();

    injectDriver_t   inject;
    uninjectDriver_t uninject;
    getCaptureText_t getCaptureText;

private:
    HINSTANCE m_hDriverInjecter;
    char  m_capStr[256];
};

#endif