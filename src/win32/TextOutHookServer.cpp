#include "win32/TextOutHookServer.h"
#include "Log.h"

void injectDummy(HWND, int) { }
void uninjectDummy() { }
void getCaptureTextDummy(char *, int *, int *) {}
int  getDllCountDummy() { return 0;}

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
    if (c > 128)
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

TextOutHookServer::TextOutHookServer()
{
    m_hDriverInjecter = LoadLibraryEx(L"TextOutHookInjecter", NULL, 0);
    if (m_hDriverInjecter) {
        inject   = (injectDriver_t)GetProcAddress(m_hDriverInjecter, "InjectTextOutDriver");
        uninject = (uninjectDriver_t)GetProcAddress(m_hDriverInjecter, "UninjectTextOutDriver");
        _getCaptureText = (getCaptureText_t)GetProcAddress(m_hDriverInjecter, "GetCaptureText");
        getDllCount = (getDllCount_t)GetProcAddress(m_hDriverInjecter, "GetDllCount");
    } else {
        inject = injectDummy;
        uninject = uninjectDummy;
        _getCaptureText = getCaptureTextDummy;
        getDllCount = getDllCountDummy;
    }
}


#define IS_SEPARATOR_LOOP(pos, bWChr) \
if (bWChr) { \
    wchar_t wc = ((WCHAR *)m_strbuf)[pos]; \
    if (isSeparatorW(wc)) break; \
} else {  \
    if (isSeparatorA(m_strbuf[pos]))  break; \
}

char * TextOutHookServer::getCaptureText(bool isWChr)
{
    int start, end, pos, nString;
    int copysize, copystart;

    _getCaptureText(m_strbuf, &pos, &nString);

    if ( pos >= 0  && pos < nString) {
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

            if (copystart < 254) {
                /* It should be 255 for ANSI string, but it doesn't matter, 
                   Let us simplify this.*/
                if (copystart + copysize < 254) {
                    memmove(m_strbuf, m_strbuf+copystart, copysize);
                    m_strbuf[copysize] = '\0';
                    m_strbuf[copysize+1] = '\0';
                    return m_strbuf;;
                }
            }
        } // end > start
    }

/* something wrong */
    m_strbuf[0] = '\0';
    m_strbuf[1] = '\0';
    return m_strbuf;
}

TextOutHookServer::~TextOutHookServer()
{
    if (m_hDriverInjecter) {
        uninject();
        FreeLibrary(m_hDriverInjecter);
    }
}

