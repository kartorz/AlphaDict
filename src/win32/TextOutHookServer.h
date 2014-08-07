#ifndef _TEXTOUTHOOKSERVER_H_
#define _TEXTOUTHOOKSERVER_H_
#include <Windows.h>

#define CAPMODE_MOUSE_OVER       0x01
#define CAPMODE_MOUSE_SELECTION  0x02

typedef void (*injectDriver_t)(HWND);
typedef void (*uninjectDriver_t)();
typedef void (*getCaptureText_t)(char *, int *, int *);
typedef int  (*getDllCount_t)();
typedef void (*captureTextEnable_t)(int);

// A duplication defined in TextOutHookInjecter.cpp
enum {
    WM_CW_ERROR = WM_USER,
    WM_CW_TEXTA,
    WM_CW_TEXTW,
    WM_CW_LBUTTON,
    WM_CW_DEBUG,
};

class TextOutHookServer
{
public:
    static TextOutHookServer& getReference();
    TextOutHookServer();
    ~TextOutHookServer();
    void extractWord();
    char *getCaptureText(bool isWChr);
    injectDriver_t   inject;
    uninjectDriver_t uninject;
    getDllCount_t    getDllCount;
    captureTextEnable_t captureTextEnable;

private:
    getCaptureText_t _getCaptureText;

    HINSTANCE m_hDriverInjecter;
    char m_strbuf[256];
};

#endif