#include <Windows.h>
#include <Shlwapi.h>

#define MSG_TIMEOUT   50

#pragma data_seg(".HKT")
HHOOK g_hMouseHook = NULL;
HWND  g_hHookServer = NULL;
TCHAR g_szDllPath[MAX_PATH] = {'\0'};
//DWORD g_dllCount = 0;
#pragma data_seg()
#pragma comment(linker, "/section:.HKT,rws")

const int MOUSEOVER_INTERVAL = 200;

enum {
    WM_CW_ERROR = WM_USER,
    WM_CW_TEXTA,
    WM_CW_TEXTW,
    WM_CW_LBUTTON,
};

HINSTANCE g_hModule = NULL;
HANDLE    g_hSyncMutex = NULL;
HINSTANCE g_hTextOutHook = NULL;
UINT_PTR  g_timerID = NULL;
POINT     g_pMouse = {-1, -1};

typedef BOOL (*CaptureTextEnable_t)(HWND, HWND, POINT, BOOL *);
CaptureTextEnable_t  CaptureTextEnable = NULL;

typedef void (*GetCaptureText_t)(CHAR *,  int *, int *);
GetCaptureText_t  _GetCaptureText = NULL;

void CALLBACK TimerFunc(HWND hWnd,UINT nMsg,UINT nTimerid,DWORD dwTime)
{
    DWORD result = WaitForSingleObject(g_hSyncMutex, INFINITE);

    if (result == WAIT_OBJECT_0 || result == WAIT_ABANDONED) {
    __try {
        KillTimer(NULL, g_timerID);
        g_timerID = NULL;

        if (g_hTextOutHook) {
            if (!CaptureTextEnable) {
               CaptureTextEnable = (CaptureTextEnable_t)GetProcAddress(g_hTextOutHook, "CaptureTextEnable");
               if (!CaptureTextEnable) {
                   ReleaseMutex(g_hSyncMutex);  
                   return;
               }
            }

            HWND hWnd = WindowFromPoint(g_pMouse);
            if (hWnd) {
                BOOL  isWChr;
                DWORD ret;
                DWORD msgID = WM_CW_TEXTA;
                BOOL got = CaptureTextEnable(g_hHookServer, hWnd, g_pMouse, &isWChr);
                if (got) {
                    if (isWChr)  msgID++;
                    SendMessageTimeout(g_hHookServer, msgID, 0, 0, SMTO_ABORTIFHUNG, 50, &ret);
                }
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
        if (!g_hTextOutHook) {
            DWORD  ret;
            DWORD  wParam = GetLastError();
            SendMessageTimeout(g_hHookServer, WM_CW_ERROR, (WPARAM)wParam, 0, SMTO_ABORTIFHUNG, MSG_TIMEOUT, &ret);
            //UnhookWindowsHookEx(g_hMouseHook);
            return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);
        }
    }

    if (nCode == HC_ACTION) {
        if (wParam == WM_LBUTTONDOWN || wParam == WM_NCLBUTTONDOWN ) {
            DWORD  ret;
            SendMessageTimeout(g_hHookServer, WM_CW_LBUTTON, g_pMouse.x, g_pMouse.y, SMTO_ABORTIFHUNG, 50, &ret);
        } else if (wParam == WM_MOUSEMOVE || wParam == WM_NCMOUSEMOVE) {
            g_pMouse = ((PMOUSEHOOKSTRUCT)lParam)->pt;
            g_timerID = SetTimer(NULL, g_timerID, MOUSEOVER_INTERVAL, TimerFunc);
        }
    }
    return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) void InjectTextOutDriver(HWND hServer)
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

extern "C" __declspec(dllexport) void UninjectTextOutDriver()
{
    if (g_hMouseHook) {
        UnhookWindowsHookEx(g_hMouseHook);
        g_hMouseHook = NULL;
    }
}

extern "C" __declspec(dllexport) void GetCaptureText(CHAR *str, int *pos, int *cbString)
{
    if (g_hTextOutHook) {
        if (!_GetCaptureText) {
            _GetCaptureText = (GetCaptureText_t)GetProcAddress(g_hTextOutHook, "GetCaptureText");
            if (!_GetCaptureText) {   
               return;
            }
        }
        _GetCaptureText(str, pos, cbString);
    }
}

extern "C" __declspec(dllexport) int GetDllCount()
{
    return 0;//g_dllCount;
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
        DWORD result = WaitForSingleObject(g_hSyncMutex, INFINITE);
        if (result == WAIT_OBJECT_0 || result == WAIT_ABANDONED) {
            if (g_timerID) {
                KillTimer(NULL, g_timerID);
                g_timerID = NULL;
            }
            ReleaseMutex(g_hSyncMutex);
            CloseHandle(g_hSyncMutex);
            g_hSyncMutex = NULL;
        }

        if (g_hTextOutHook) {
            FreeLibrary(g_hTextOutHook);
            g_hTextOutHook = NULL;
        }
        break;
    }
    return TRUE;
}
