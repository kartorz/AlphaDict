#include <Windows.h>
#include <Shlwapi.h>

#define MSG_TIMEOUT   50

#pragma data_seg(".HKT")
HHOOK g_hMouseHook = NULL;
HWND  g_hHookServer = NULL;
TCHAR g_szDllPath[MAX_PATH] = {'\0'};
#pragma data_seg()
#pragma comment(linker, "/section:.HKT,rws")

// Msg
const int MOUSEOVER_INTERVAL = 200;
const int WM_CW_ERROR = WM_USER;

// wParam
// WM_CW_ERROR
/*  1: can't load TextOutHook.dll
 *
 */
HINSTANCE g_hModule = NULL;
HANDLE    g_hSyncMutex = NULL;
HINSTANCE g_hTextOutHook = NULL;
UINT_PTR  g_timerID = NULL;
POINT     g_pMouse = {-1, -1};

typedef void (*CaptureTextEnable_t)(HWND, HWND, POINT);
CaptureTextEnable_t  CaptureTextEnable = NULL;

typedef void (*GetCaptureText_t)(CHAR*);
GetCaptureText_t  GetCaptureText = NULL;

void CALLBACK TimerFunc(HWND hWnd,UINT nMsg,UINT nTimerid,DWORD dwTime)
{
    DWORD result = WaitForSingleObject(g_hSyncMutex, 0);
    if (result == WAIT_OBJECT_0 || result == WAIT_ABANDONED) {
    __try {
        KillTimer(NULL, g_timerID);
        g_timerID = NULL;

        DWORD  ret;
        SendMessageTimeout(g_hHookServer, WM_USER+2, 0, 0, SMTO_ABORTIFHUNG, 50, &ret);

        if (g_hTextOutHook) {
            if (!CaptureTextEnable) {
               CaptureTextEnable = 
               (CaptureTextEnable_t)GetProcAddress(g_hTextOutHook, "CaptureTextEnable");
               if (!CaptureTextEnable) {
                   DWORD  ret;
                   SendMessageTimeout(g_hHookServer, WM_USER+203, 0, 0, SMTO_ABORTIFHUNG, 50, &ret);    
                   return;
               }
            }
            DWORD  ret;
            SendMessageTimeout(g_hHookServer, WM_USER+3, 0, 0, SMTO_ABORTIFHUNG, 50, &ret);
            HWND hWnd = WindowFromPoint(g_pMouse);
            if (hWnd) {
                CaptureTextEnable(g_hHookServer, hWnd, g_pMouse);
            }
        }  
    } __finally {
        ReleaseMutex(g_hSyncMutex);
    }
    }
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (!g_hTextOutHook) {
        g_hTextOutHook = LoadLibrary(g_szDllPath);
        DWORD  ret;
        if (!g_hTextOutHook) {
            DWORD  wParam = GetLastError();
            SendMessageTimeout(g_hHookServer, WM_CW_ERROR+200, (WPARAM)wParam, 0, SMTO_ABORTIFHUNG, MSG_TIMEOUT, &ret);
            //UnhookWindowsHookEx(g_hMouseHook);
        } else {
            SendMessageTimeout(g_hHookServer, WM_CW_ERROR+205, (WPARAM)wParam, 0, SMTO_ABORTIFHUNG, MSG_TIMEOUT, &ret);
        }
    }

    if (nCode == HC_ACTION  && wParam == WM_LBUTTONDOWN) {
        //placehold = wParam;
        //DWORD  ret;
        //SendMessageTimeout(g_hHookServer, WM_MY_SHOW_TRANSLATION, 0, 0, SMTO_ABORTIFHUNG, 50, &ret);
    }
    if ((nCode == HC_ACTION) && ((wParam == WM_MOUSEMOVE) || (wParam == WM_NCMOUSEMOVE))) {
        g_pMouse = ((PMOUSEHOOKSTRUCT)lParam)->pt;
        g_timerID = SetTimer(NULL, g_timerID, MOUSEOVER_INTERVAL, TimerFunc);
    }
    return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) void injectTextOutDriver(HWND hServer)
{
    g_hHookServer = hServer;

    GetModuleFileName(0, g_szDllPath, sizeof(g_szDllPath) - sizeof(TCHAR));
    bool find = false;
    int i = lstrlen(g_szDllPath);
    LPTSTR s = g_szDllPath + i;
    for (; i>0; i--) {
        //LPTSTR  pChr = CharPrev(g_szDllPath, s);
        if (*s == TCHAR('\\')) {
            find = true;
            break;
        }
        --s;
    }
    if (find) {
        //StringCchCopy( s, sizeof(TEXT("/TextOutHook.dll")), TEXT("/TextOutHook.dll"));
        if (MAX_PATH - lstrlen(g_szDllPath) >= sizeof(TEXT("/TextOutHook.dll")) + sizeof(TCHAR))
            lstrcpy(s, TEXT("\\TextOutHook.dll"));  // remove file name.

        g_hMouseHook = SetWindowsHookEx(WH_MOUSE, MouseHookProc, g_hModule, 0);
    }
}

extern "C" __declspec(dllexport) void uninjectTextOutDriver()
{
    if (g_hMouseHook) {
        UnhookWindowsHookEx(g_hMouseHook);
        g_hMouseHook = NULL;
    }
}

extern "C" __declspec(dllexport) void getCaptureText(CHAR *str)
{
    if (g_hTextOutHook) {
        if (!GetCaptureText) {
            GetCaptureText = 
            (GetCaptureText_t)GetProcAddress(g_hTextOutHook, "GetCaptureText");
            if (!GetCaptureText) {
               DWORD  ret;
               SendMessageTimeout(g_hHookServer, WM_USER+204, 0, 0, SMTO_ABORTIFHUNG, 50, &ret);    
               return;
            }
        }
        GetCaptureText(str);
        //TCHAR buf[256] = TEXT("This is the destination");
        //CopyMemory(str, buf, 256);
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserve)
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        g_hModule = hinstDLL;
        if(g_hSyncMutex == NULL){
            g_hSyncMutex = CreateMutex(NULL, FALSE, NULL);
            if (!g_hSyncMutex)
                return FALSE;
        }
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        WaitForSingleObject(g_hSyncMutex, 0);
        if (g_timerID) {
            KillTimer(NULL, g_timerID);
            g_timerID = NULL;
        }
        ReleaseMutex(g_hSyncMutex);
        CloseHandle(g_hSyncMutex);

        if (g_hTextOutHook) {
            FreeLibrary(g_hTextOutHook);
            g_hTextOutHook = NULL;
        }
        break;
    }
    return TRUE;
}
