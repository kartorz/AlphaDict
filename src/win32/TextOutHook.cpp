#include "HookAPI.h"

#pragma data_seg(".STRBUF")
HANDLE g_bufMutex = NULL;
CHAR   g_strbuf[256] = {0};
DWORD  g_dllCount = 0;
#pragma data_seg()
#pragma comment(linker, "/section:.STRBUF,rws")

HWND g_hHookServer = NULL;
bool g_bCapture = FALSE;
int  g_nMouseX = -1;
int  g_nMouseY = -1;

typedef BOOL (WINAPI *TextOutAHook_t)(
    HDC hdc, int nXStart, int nYStart, LPCSTR lpszString, int cbString);

typedef BOOL (WINAPI *TextOutWHook_t)(
    HDC hdc, int nXStart, int nYStart, LPCWSTR lpszString, int cbString);

typedef BOOL (WINAPI *ExtTextOutAHook_t)(HDC hdc, int nXStart, int nYStart, 
    UINT fuOptions, CONST RECT *lprc, LPCSTR lpszString, UINT cbString, CONST INT *lpDx);

typedef BOOL (WINAPI *ExtTextOutWHook_t)(HDC hdc, int nXStart, int nYStart,
    UINT fuOptions, CONST RECT *lprc, LPCWSTR lpszString, UINT cbString, CONST INT *lpDx);

TextOutAHook_t TextOutAOri = NULL;
TextOutWHook_t TextOutWOri = NULL; 
ExtTextOutAHook_t ExtTextOutAOri = NULL;
ExtTextOutWHook_t ExtTextOutWOri = NULL;

static BOOL isTextUnderMouse(int nMouseX, int nMouseY,
                             int nXStart, int nYStart, int cbString)
{
    if (nMouseX <= nXStart &&  nMouseY <= nYStart)
        return TRUE;
    return FALSE; 
}

static void CaptureTextOutA(int nXStart, int nYStart, LPCSTR lpszString, int cbString)
{
    if (g_bCapture) {
        //if (isTextUnderMouse(g_nMouseX, g_nMouseY, nXStart, nYStart, cbString)) {
            WaitForSingleObject(g_bufMutex, INFINITE);
             __try {
                memset(g_strbuf, 0, 256);
                BYTE size = cbString > 255 ?  255 : cbString;  // ending with '/0'
                CopyMemory(g_strbuf, lpszString, size);
            } __finally {
                ReleaseMutex(g_bufMutex);
            }
            DWORD  ret;
            SendMessageTimeoutA(g_hHookServer, WM_USER+101, 0, 0, SMTO_ABORTIFHUNG, 50, &ret);
        //}
    }
}

static void CaptureTextOutW(int nXStart, int nYStart, LPCWSTR lpszString, int cbString)
{
    if (g_bCapture) {
    //    if (isTextUnderMouse(g_nMouseX, g_nMouseY, nXStart, nYStart, cbString)) {
             WaitForSingleObject(g_bufMutex, INFINITE);
             __try {
                 memset(g_strbuf, 0, 256);
                 BYTE size = cbString*sizeof(WCHAR) > 255 ? 255 : cbString*sizeof(WCHAR);
                 CopyMemory(g_strbuf, lpszString, size);
             } __finally {
                 ReleaseMutex(g_bufMutex);
             }
             DWORD  ret;
             SendMessageTimeoutW(g_hHookServer, WM_USER+102, 0, 0, SMTO_ABORTIFHUNG, 50, &ret);
    //    }
    }
}

BOOL WINAPI TextOutAHook(HDC hdc, int nXStart, int nYStart, LPCSTR lpszString, int cbString)
{
    CaptureTextOutA(nXStart, nYStart, lpszString, cbString);  
    return TextOutAOri(hdc, nXStart, nYStart, lpszString, cbString);
}

BOOL WINAPI TextOutWHook(HDC hdc, int nXStart, int nYStart, LPCWSTR lpszString, int cbString)
{
    CaptureTextOutW(nXStart, nYStart, lpszString, cbString);
    return TextOutWOri(hdc, nXStart, nYStart, lpszString, cbString);
}

BOOL WINAPI ExtTextOutAHook(HDC hdc, int nXStart, int nYStart, UINT fuOptions,
    CONST RECT *lprc, LPCSTR lpszString, UINT cbString, CONST INT *lpDx)
{
    CaptureTextOutA(nXStart, nYStart, lpszString, cbString);
    return ExtTextOutAOri(hdc, nXStart, nYStart, fuOptions, lprc, lpszString, cbString, lpDx);
}

BOOL WINAPI ExtTextOutWHook(HDC hdc, int nXStart, int nYStart, UINT fuOptions,
    CONST RECT *lprc, LPCWSTR lpszString, UINT cbString, CONST INT *lpDx)
{
    CaptureTextOutW(nXStart, nYStart, lpszString, cbString);
    return ExtTextOutWOri(hdc, nXStart, nYStart, fuOptions, lprc, lpszString, cbString, lpDx);
}

static void InstallTextOutHooks()
{
    if (TextOutAOri == NULL)
        HookAPI("gdi32.dll", "TextOutA",    (PROC)TextOutAHook,    (PROC*)&TextOutAOri);
    if (TextOutWOri == NULL)
        HookAPI("gdi32.dll", "TextOutW",    (PROC)TextOutWHook,    (PROC*)&TextOutWOri);
    if (ExtTextOutAOri == NULL)
        HookAPI("gdi32.dll", "ExtTextOutA", (PROC)ExtTextOutAHook, (PROC*)&ExtTextOutAOri);
    if (ExtTextOutWOri == NULL)
        HookAPI("gdi32.dll", "ExtTextOutW", (PROC)ExtTextOutWHook, (PROC*)&ExtTextOutWOri);
}

static void UninstallTextOutHooks()
{
    if (TextOutAOri != NULL) {
        HookAPI("gdi32.dll", "TextOutA", (PROC)TextOutAOri, NULL);
        TextOutAOri = NULL;
    }

    if (TextOutWOri != NULL) {
        HookAPI("gdi32.dll", "TextOutW", (PROC)TextOutWOri, NULL);
        TextOutWOri = NULL;
    }

    if (ExtTextOutAOri != NULL) {
        HookAPI("gdi32.dll", "ExtTextOutA", (PROC)ExtTextOutAOri, NULL);
        ExtTextOutAOri = NULL;
    }

    if (ExtTextOutWOri != NULL) {
        HookAPI("gdi32.dll", "ExtTextOutW", (PROC)ExtTextOutWOri, NULL);
        ExtTextOutAOri = NULL;
    }
}

extern "C" __declspec(dllexport) void CaptureTextEnable(HWND hHookServer, HWND hWnd, POINT pMouse)
{
    ScreenToClient(hWnd, &pMouse);
    g_hHookServer = hHookServer;
    g_nMouseX = pMouse.x;
    g_nMouseY = pMouse.y;

    DWORD  ret;
    SendMessageTimeout(g_hHookServer, WM_USER+104, (WPARAM)(&pMouse), 0, SMTO_ABORTIFHUNG, 50, &ret);

    if (pMouse.y >= 0) {
        RECT UpdateRect;
	GetClientRect(hWnd, &UpdateRect);
	UpdateRect.top = pMouse.y;
	UpdateRect.bottom = pMouse.y + 1;
        g_bCapture = TRUE;
        {
            DWORD  ret;
            SendMessageTimeout(g_hHookServer, WM_USER+105, 0, 0, SMTO_ABORTIFHUNG, 50, &ret);
        }
	InvalidateRect(hWnd, &UpdateRect, FALSE);
	UpdateWindow(hWnd);
        //g_bCapture = FALSE;
        {
        DWORD  ret;
        SendMessageTimeout(g_hHookServer, WM_USER+106, 0, 0, SMTO_ABORTIFHUNG, 50, &ret);
        }
    }
}

extern "C" __declspec(dllexport) void GetCaptureText(CHAR *str)
{
    //CopyMemory((PVOID)str, (const VOID*)g_strbuf, 256);
    WaitForSingleObject(g_bufMutex, INFINITE);
    __try {
        //TCHAR buf[128] = TEXT("This is the destination in hook");
        //char buf2[256];
        //CopyMemory(buf2, buf, 256);
        //CopyMemory(str, buf2, 256);
        CopyMemory(str, g_strbuf, 256);
    } __finally {
        ReleaseMutex(g_bufMutex);
    }
}

BOOL WINAPI DllMain(
    _In_  HINSTANCE hinstDLL,
    _In_  DWORD fdwReason,
    _In_  LPVOID lpvReserved
)
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        InstallTextOutHooks();

	if (g_bufMutex == NULL) {
            g_bufMutex = CreateMutex( NULL, TRUE, NULL);
        }
        ++g_dllCount;
        break;
    case DLL_PROCESS_DETACH:
	WaitForSingleObject(g_bufMutex, INFINITE);

	UninstallTextOutHooks();
        --g_dllCount;
        ReleaseMutex(g_bufMutex);
        if (g_bufMutex && (g_dllCount == 0)) {
            // be careful
            //WaitForSingleObject(g_bufMutex, INFINITE); 
            CloseHandle(g_bufMutex);
            g_bufMutex = NULL;
        }
        break;
    default:
        break;
    }

    return TRUE; /* Loader will check this return code. */
}