#include <Windows.h>
#include <Shlwapi.h>

#define MSG_TIMEOUT   50
#define CAPMODE_MOUSE_OVER       0x01
#define CAPMODE_MOUSE_SELECTION  0x02

#define TEXTOUT_HOOK

#pragma data_seg(".HKT")
HHOOK g_hMouseHook = NULL;
HWND  g_hHookServer = 0;
TCHAR g_szDllPath[MAX_PATH] = {'\0'};

// 0x01: mouse over
// 0x02: mouse selection
int   g_iCapMode = 0;
//DWORD g_dllCount = 0;
#pragma data_seg()
#pragma comment(linker, "/section:.HKT,rws")

const int MOUSEOVER_INTERVAL = 300;

enum {
    WM_CW_ERROR = WM_USER,
    WM_CW_TEXTA,
    WM_CW_TEXTW,
    WM_CW_LBUTTON,
    WM_CW_SELECTION,
    WM_CW_DEBUG,
};

HINSTANCE g_hModule = NULL;
HANDLE    g_hSyncMutex = 0;
HINSTANCE g_hTextOutHook = NULL;
UINT_PTR  g_timerID = 0; //if hWnd parameter is NULL, timerID is integer not a nonzero integer.
BOOL      g_timerValid = false;

POINT     g_pMouse = {-1, -1};
POINT     g_pLastMouse = {-1, -1};
int       g_selectionStatus = 0;
BOOL      g_bSelectionText = FALSE;
BOOL      g_bLButtonDown = FALSE;
BOOL      g_bLoadTextOutHook = FALSE;
BOOL      g_bDetaching = FALSE;

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
    DWORD result = WaitForSingleObject(g_hSyncMutex, 0);
    if (result == WAIT_OBJECT_0 || result == WAIT_ABANDONED) {
    __try {
        KillTimer(NULL, g_timerID);
        g_timerID = 0;
        g_timerValid = false;

        if (g_iCapMode == 0 || g_bDetaching)
            __leave;

        HWND hWnd = WindowFromPoint(g_pMouse);
        if (g_bSelectionText) {
            if (hWnd) {
                POINT p = g_pMouse;
                ScreenToClient(hWnd, &p);
                //SendMessage(g_hHookServer, WM_CW_DEBUG, p.x, p.y); 
                if (p.y >= 0) {
                    //SendMessage(g_hHookServer, WM_CW_SELECTION, 0, 0);
                    //SendMessage(hWnd, WM_COPY, 0, 0);
                    SendCtrlCKeyCode();
                }
            }
            g_bSelectionText = FALSE;
            __leave;
        }
    #ifdef TEXTOUT_HOOK
        if ((!g_hTextOutHook) && (!g_bLoadTextOutHook)) {
            g_bLoadTextOutHook = true;
            g_hTextOutHook = LoadLibrary(g_szDllPath);
            if (!g_hTextOutHook) {
                DWORD  ret;
                DWORD  wParam = GetLastError();
                SendMessageTimeout(g_hHookServer, WM_CW_ERROR, (WPARAM)wParam, 0, SMTO_ABORTIFHUNG, MSG_TIMEOUT, (PDWORD_PTR)&ret);
            }
        }

        if (g_hTextOutHook) {
            if (!_CaptureTextEnable) {
               _CaptureTextEnable = (CaptureTextEnable_t)GetProcAddress(g_hTextOutHook, "CaptureTextEnable");
               if (!_CaptureTextEnable) {
                   __leave;
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
							SendMessageTimeout(g_hHookServer, msgID, 0, 0, SMTO_ABORTIFHUNG, 50, (PDWORD_PTR)&ret);
                        }
                    }
                }
            }
        } // g_hTextOutHook
    #endif
    } __finally {
        ReleaseMutex(g_hSyncMutex);
    }
    }
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0 || g_bDetaching) {
        return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);
    }

    if (!g_iCapMode) {
        if (g_bSelectionText)  g_bSelectionText = false;
        if (g_bLButtonDown)    g_bLButtonDown = false;
        if (g_selectionStatus) g_selectionStatus = 0;
        return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);
    }

    if (nCode == HC_ACTION && g_hSyncMutex) {
    DWORD result = WaitForSingleObject(g_hSyncMutex, 0);
    if (result == WAIT_OBJECT_0 || result == WAIT_ABANDONED) {
        if (wParam == WM_LBUTTONDOWN || wParam == WM_NCLBUTTONDOWN ) {
            POINT pMouse = ((PMOUSEHOOKSTRUCT)lParam)->pt;
            DWORD  ret;
			SendMessageTimeout(g_hHookServer, WM_CW_LBUTTON, pMouse.x, pMouse.y, SMTO_ABORTIFHUNG, 20, (PDWORD_PTR)&ret);
            if (g_iCapMode & CAPMODE_MOUSE_SELECTION) {
                    g_pMouse = pMouse;
                    g_selectionStatus = 1;
            }
            g_bLButtonDown = true;
        } else if (wParam == WM_MOUSEMOVE || wParam == WM_NCMOUSEMOVE) {
            POINT pMouse = ((PMOUSEHOOKSTRUCT)lParam)->pt;
            if (abs(pMouse.x - g_pMouse.x) > 1) {
                g_pMouse = pMouse;
                if ((g_iCapMode & CAPMODE_MOUSE_OVER) && (!g_bLButtonDown)) {
                    if (!g_bSelectionText /* wait processing seletion timer.*/) {
                    #ifdef TEXTOUT_HOOK
                        g_timerID = SetTimer(NULL, g_timerID, MOUSEOVER_INTERVAL, (TIMERPROC)TimerFunc);
                        g_timerValid = true;
                     #endif
                     }
                } else if (g_iCapMode & CAPMODE_MOUSE_SELECTION){
                    if (g_selectionStatus == 1) {
                        g_selectionStatus = 2;
                    }
                }
            }
		} else if (wParam == WM_LBUTTONUP || wParam == WM_NCLBUTTONUP) {
            g_bLButtonDown = false;
            if (g_iCapMode & CAPMODE_MOUSE_SELECTION){
                if (g_selectionStatus == 2) {
                    g_selectionStatus = 3;
                    g_bSelectionText = TRUE;
                    g_pMouse = ((PMOUSEHOOKSTRUCT)lParam)->pt;
					g_timerID = SetTimer(NULL, g_timerID, MOUSEOVER_INTERVAL, (TIMERPROC)TimerFunc);
                    g_timerValid = true;
                }
            }
        }else if (wParam == WM_LBUTTONDBLCLK || wParam == WM_NCLBUTTONDBLCLK) {
			g_bSelectionText = TRUE;
			g_pMouse = ((PMOUSEHOOKSTRUCT)lParam)->pt;
			g_timerID = SetTimer(NULL, g_timerID, MOUSEOVER_INTERVAL, (TIMERPROC)TimerFunc);
			g_timerValid = true;
		}
        ReleaseMutex(g_hSyncMutex);
    }
    }
    return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) void InjectTextOutDriver(HWND hServer)
{
    if (!g_hMouseHook) {
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
}

extern "C" __declspec(dllexport) void UninjectTextOutDriver()
{
    if (g_hMouseHook) {
        UnhookWindowsHookEx(g_hMouseHook);

        DWORD result = WaitForSingleObject(g_hSyncMutex, 5000/*INFINITE*/);
        // reinit shared data.
        g_iCapMode = 0;
        g_hMouseHook = NULL;
        g_hHookServer = 0;

        if (g_timerValid) {
            KillTimer(NULL, g_timerID);
            g_timerID = 0;
            g_timerValid = false;
        }
        ReleaseMutex(g_hSyncMutex);
    }
}

extern "C" __declspec(dllexport) void GetCaptureText(CHAR *str, int *pos, int *cbString)
{
    DWORD result = WaitForSingleObject(g_hSyncMutex, 5000);
    if (g_hTextOutHook && g_hMouseHook) {
        if (!_GetCaptureText) {
            _GetCaptureText = (GetCaptureText_t)GetProcAddress(g_hTextOutHook, "GetCaptureText");
            if (!_GetCaptureText) {
                ReleaseMutex(g_hSyncMutex);   
                return;
            }
        }
        _GetCaptureText(str, pos, cbString);
    }
    ReleaseMutex(g_hSyncMutex);
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
        if(g_hSyncMutex == 0){
            g_hSyncMutex = CreateMutex(NULL, FALSE, NULL);
            if (!g_hSyncMutex)
                return FALSE;
        }
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        if (g_hSyncMutex) {
        DWORD result = WaitForSingleObject(g_hSyncMutex, 5000);
        //g_iCapMode = 0; // This is a shared data, effect all the processes.
        g_bDetaching = TRUE; // Tell MouseHookProc stopping work.
        if (g_timerValid/* may g_timerID == 0*/) {
            KillTimer(NULL, g_timerID);
            g_timerID = 0;
            g_timerValid = false;
        }
        {
            MSG msg ;
            while (PeekMessage (&msg, 0, WM_TIMER, WM_TIMER, PM_REMOVE)) {}
	}

        if (g_hTextOutHook) {
             FreeLibrary(g_hTextOutHook);
             g_hTextOutHook = NULL;
        }

        ReleaseMutex(g_hSyncMutex);
        CloseHandle(g_hSyncMutex);
        g_hSyncMutex = 0;
        break;
    }
    }
    return TRUE;
}
