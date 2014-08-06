#include <Windows.h>
#include <Shlwapi.h>

#define MSG_TIMEOUT   50
#define CAPMODE_MOUSE_OVER       0x01
#define CAPMODE_MOUSE_SELECTION  0x02

#pragma data_seg(".HKT")
HHOOK g_hMouseHook = NULL;
HWND  g_hHookServer = NULL;
TCHAR g_szDllPath[MAX_PATH] = {'\0'};

// 0x01: mouse over
// 0x02: mouse selection
int   g_iCapMode = 0;
//DWORD g_dllCount = 0;
#pragma data_seg()
#pragma comment(linker, "/section:.HKT,rws")

const int MOUSEOVER_INTERVAL = 200;

enum {
    WM_CW_ERROR = WM_USER,
    WM_CW_TEXTA,
    WM_CW_TEXTW,
    WM_CW_LBUTTON,
    WM_CW_DEBUG,
};

HINSTANCE g_hModule = NULL;
HANDLE    g_hSyncMutex = NULL;
HINSTANCE g_hTextOutHook = NULL;
UINT_PTR  g_timerID = NULL;
POINT     g_pMouse = {-1, -1};
POINT     g_pLastMouse = {-1, -1};
int       g_selectionStatus = 0;
BOOL      g_bSelectionText = FALSE;
BOOL      g_bLButtonDown = FALSE;

typedef BOOL (*CaptureTextEnable_t)(HWND, HWND, POINT, BOOL *);
CaptureTextEnable_t  _CaptureTextEnable = NULL;

typedef void (*GetCaptureText_t)(CHAR *,  int *, int *);
GetCaptureText_t  _GetCaptureText = NULL;

static void SendCtrlCKeyCode()
{
#define VK_C 0x43
    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    ip.ki.wVk = VK_CONTROL;
    ip.ki.dwFlags = 0;
    SendInput(1, &ip, sizeof(INPUT));
   
    ip.ki.wVk = VK_C;
    SendInput(1, &ip, sizeof(INPUT));

    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));

    ip.ki.wVk = VK_CONTROL;
    SendInput(1, &ip, sizeof(INPUT));
}
void CALLBACK TimerFunc(HWND hWnd,UINT nMsg,UINT nTimerid,DWORD dwTime)
{
    DWORD result = WaitForSingleObject(g_hSyncMutex, INFINITE);

    if (result == WAIT_OBJECT_0 || result == WAIT_ABANDONED) {
    __try {
        KillTimer(NULL, g_timerID);
        g_timerID = NULL;

        HWND hWnd = WindowFromPoint(g_pMouse);

        if (g_bSelectionText) {
            if (hWnd) {
                //SendMessage(hWnd, WM_COPY, 0, 0);
                SendCtrlCKeyCode();
            }
            g_bSelectionText = FALSE;
            ReleaseMutex(g_hSyncMutex);
            return;
        }

        if (g_hTextOutHook) {
            if (!_CaptureTextEnable) {
               _CaptureTextEnable = (CaptureTextEnable_t)GetProcAddress(g_hTextOutHook, "CaptureTextEnable");
               if (!_CaptureTextEnable) {
                   ReleaseMutex(g_hSyncMutex);  
                   return;
               }
            }
            
            if (hWnd) {
                TCHAR tIdentifier[128];
                //GetWindowText : "Capture Word"
                //GetClassName  :  "Qt5QWindowPopupDropShadowSaveBits"
                if (GetWindowText(hWnd, tIdentifier, sizeof(tIdentifier) / sizeof(TCHAR))) {
                //if (GetClassName(hWnd, tIdentifier, sizeof(tIdentifier) / sizeof(TCHAR))) {
                    if (lstrcmp(tIdentifier, TEXT("Capture Word")) != 0) {
                        DWORD ret;
                        DWORD msgID = WM_CW_TEXTA;
                        BOOL  isWChr;
                        if (_CaptureTextEnable(g_hHookServer, hWnd, g_pMouse, &isWChr)) {
                            if (isWChr)  msgID++;
                            SendMessageTimeout(g_hHookServer, msgID, 0, 0, SMTO_ABORTIFHUNG, 50, &ret);
                        }
                    }
                }
            }
        } // g_hTextOutHook
    } __finally {
        ReleaseMutex(g_hSyncMutex);
    }
    }
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (!g_iCapMode)
        return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);
    
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
            POINT pMouse = ((PMOUSEHOOKSTRUCT)lParam)->pt;
            DWORD  ret;
            SendMessageTimeout(g_hHookServer, WM_CW_LBUTTON, pMouse.x, pMouse.y, SMTO_ABORTIFHUNG, 20, &ret);
            if (g_iCapMode & CAPMODE_MOUSE_SELECTION) {
                if (!g_bSelectionText) {
                    g_pMouse = pMouse;
                    g_selectionStatus = 1;
                }
            }
            g_bLButtonDown = true;
        } else if (wParam == WM_MOUSEMOVE || wParam == WM_NCMOUSEMOVE) {
            POINT pMouse = ((PMOUSEHOOKSTRUCT)lParam)->pt;
            if (abs(pMouse.x - g_pMouse.x) > 1) {
                g_pMouse = pMouse;
                if ((g_iCapMode & CAPMODE_MOUSE_OVER) && (!g_bLButtonDown)) {
                    if (!g_bSelectionText) // wait processing seletion timer.
                        g_timerID = SetTimer(NULL, g_timerID, MOUSEOVER_INTERVAL, TimerFunc);
                } else if (g_iCapMode & CAPMODE_MOUSE_SELECTION){
                    if (g_selectionStatus == 1) {
                        g_selectionStatus = 2;
                    }
                }
            }
        } else if (wParam == WM_LBUTTONUP || WM_NCLBUTTONUP) {
            g_bLButtonDown = false;
            if (g_iCapMode & CAPMODE_MOUSE_SELECTION){
                if (g_selectionStatus == 2) {
                    g_selectionStatus = 3;
                    g_bSelectionText = TRUE;
                    g_pMouse = ((PMOUSEHOOKSTRUCT)lParam)->pt;
                    g_timerID = SetTimer(NULL, g_timerID, MOUSEOVER_INTERVAL, TimerFunc);
                }
            }
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

extern "C" __declspec(dllexport) void CaptureTextEnable(int iCapMode)
{
    g_iCapMode = iCapMode;
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
        g_iCapMode = 0;
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
