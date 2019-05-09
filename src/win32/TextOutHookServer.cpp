#include "win32/TextOutHookServer.h"
#include "Log.h"

void injectDummy(HWND) { }
void uninjectDummy() { }
void getCaptureTextDummy(char *, int *, int *) {}
int  getDllCountDummy() { return 0;}
void captureTextEnableDummy(int) { }

HINSTANCE g_hTextOutHook = NULL;

static char    separatorA[] = {' ' ,  ',',  ';',  '\t',  '\n', '\r',
                               '\\',  '.',  '-',  '_',  ':', '\0'};

static wchar_t separatorW[] = {L' ',  L',',   L';',  L'\t', L'\n', L'\r', 
                               L'\\',  L'.',  L'-',  L'_',  L':', L'\0'};

static bool isSeparatorA(char c)
{
    int i = 0;
    if (c == '\0')
        return true;
    if (c >= 128)
        return false;
    if ((c >= 'a' &&  c <= 'z')
        ||(c >= 'A' && c <= 'Z')) 
        return false;
    return true;
#if 0
    while (separatorA[i] != '\0' && c != separatorA[i])
        ++i;
    return separatorA[i] != '\0';
#endif
}

static bool isSeparatorW(WCHAR c)
{
    int i = 0;
    if (c == L'\0')
        return true;
    if (c >= 128)
        return false;
    if ((c >= L'a' &&  c <= L'z')
        ||(c >= L'A' && c <= L'Z')) 
        return false;
    return true;
#if 0
    while (separatorW[i] != L'\0' && c != separatorW[i])
        ++i;
    return separatorW[i] != L'\0';
#endif
}


TextOutHookServer& TextOutHookServer::getReference()
{
    static TextOutHookServer textOutSrv;
    return textOutSrv;
}

TextOutHookServer::TextOutHookServer(): m_loadLib(false)
{
    m_hDriverInjecter = LoadLibraryEx(L"TextOutHookInjecter", NULL, 0);
    if (m_hDriverInjecter) {
        inject   = (injectDriver_t)GetProcAddress(m_hDriverInjecter, "InjectTextOutDriver");
        uninject = (uninjectDriver_t)GetProcAddress(m_hDriverInjecter, "UninjectTextOutDriver");
        _getCaptureText = (getCaptureText_t)GetProcAddress(m_hDriverInjecter, "GetCaptureText");
        getDllCount = (getDllCount_t)GetProcAddress(m_hDriverInjecter, "GetDllCount");
        captureTextEnable = (captureTextEnable_t)GetProcAddress(m_hDriverInjecter, "CaptureTextEnable");
        m_loadLib = true;
    } else {
        inject = injectDummy;
        uninject = uninjectDummy;
        _getCaptureText = getCaptureTextDummy;
        getDllCount = getDllCountDummy;
        captureTextEnable = captureTextEnableDummy;
    }
}

TextOutHookServer::~TextOutHookServer()
{
    unloadHookLib();
}

void TextOutHookServer::unloadHookLib()
{
    if (m_loadLib && m_hDriverInjecter) {

        uninject();
#if 0
        inject = injectDummy;
        uninject = uninjectDummy;
        _getCaptureText = getCaptureTextDummy;
        getDllCount = getDllCountDummy;
        captureTextEnable = captureTextEnableDummy;
#endif
        FreeLibrary(m_hDriverInjecter);
        m_loadLib = false;
        m_hDriverInjecter = NULL;
    }
}


void TextOutHookServer::getCaptureText(char *strbuf, int bufLength, bool isWChr)
{
    int start, end, pos=0, nString=0;
    int copysize, copystart;
    int MAX_BUFLEN = bufLength;

    //MutexLock lock(m_cs);
#if 0  // for test
    if (isWChr) {
        memset(strbuf, 0, MAX_BUFLEN);
        wchar_t wtest[] =  L"°¢¶û·¨´Êµä";
        //strbuf[0] = L"°¢¶û·¨´Êµä";
        memcpy(strbuf, (char*)wtest, sizeof(wtest));
    } else {
        memset(strbuf, 0, MAX_BUFLEN);
        char test[] = "alphadict";
        memcpy(strbuf, test, sizeof(test));
    }
    return;
#endif

#define IS_SEPARATOR_LOOP(pos, bWChr) \
if (bWChr) { \
    wchar_t wc = ((WCHAR *)strbuf)[pos]; \
    if (isSeparatorW(wc)) break; \
} else {  \
    if (isSeparatorA(strbuf[pos]))  break; \
}
    _getCaptureText(strbuf, &pos, &nString);
    if ( pos >= 0  && pos < nString && nString < MAX_BUFLEN) {
        for (start = pos; start >= 0; start--) {
            IS_SEPARATOR_LOOP(start, isWChr)
        }
        start += 1;
        end = start < pos ? pos : start+1; // 'POS' is a separator.
        for (; end <= nString-1; end++) {
            IS_SEPARATOR_LOOP(end, isWChr)
        }
        end -= 1;
        if (end > start) {
            copysize = (end - start + 1);
            copystart =  start;
            if (isWChr) {
                copysize = copysize * sizeof(wchar_t);
                copystart = copystart * sizeof(wchar_t);
            }

            if (copystart < MAX_BUFLEN-2) {
                /* It should be MAX_BUFLEN-1 for ANSI string, but it doesn't matter, 
                   Let us simplify this.*/
                if (copystart + copysize < MAX_BUFLEN-2) {
                    memmove(strbuf, strbuf+copystart, copysize);
                    strbuf[copysize] = '\0';
                    strbuf[copysize+1] = '\0';
                    return;
                }
            }
        } // end > start
    }

/* something wrong */
    strbuf[0] = '\0';
    strbuf[1] = '\0';
}

