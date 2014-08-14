/*
 *  Fix me : There is a radom crash when uninstall hook, the cause should be that
 *           The TextOutHooks are called when being uninstalled. 
 */

#include "HookAPI.h"

#pragma data_seg(".STRBUF")
CHAR  g_strbuf[256] = {0};
BOOL  g_bCapture = FALSE;
POINT g_pMouse;
HWND  g_hHookServer = 0;
BOOL  g_bWChr = false;
int   g_tPos = -1;
int   g_cbString = -1;
#pragma data_seg()
#pragma comment(linker, "/section:.STRBUF,rws")

#define WM_CW_DEBUG  WM_USER+4

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

#define GetTextExtentPoint32X(hdc, lpszString, cbString, tSize, bWChr) \
do { \
if (bWChr) \
    GetTextExtentPoint32W(hdc, (LPCWSTR)lpszString, cbString, tSize); \
else  \
    GetTextExtentPoint32A(hdc, lpszString, cbString, tSize); \
} while(0)

static BOOL 
CaptureTextOut(HDC hdc, int nXStart, int nYStart, LPCSTR lpszString, int cbString, BOOL bWChr)
{
    SIZE  tSize;
    POINT ptTextPos;
    RECT  Rect;
    BOOL  bInsideRect = FALSE;
    POINT pMouse = g_pMouse;
    
    GetTextExtentPoint32X(hdc, lpszString, cbString, &tSize, bWChr);

    ptTextPos.x = nXStart;
    ptTextPos.y = nYStart;
    LPtoDP(hdc, &ptTextPos, 1);

    Rect.left = ptTextPos.x;
    Rect.right = ptTextPos.x + tSize.cx;
    Rect.top = ptTextPos.y;
    Rect.bottom = ptTextPos.y + tSize.cy;
     /*	
      * On some situation, RECT will be [0,4,-100,-83] or [36,114,-9,8],
      * while mouse point be [47, 100]. It seems 'ptTexPos.y' being wrong.
      */
#if 0
    DWORD  ret;
    SendMessageTimeoutA(g_hHookServer, WM_CW_DEBUG, -1, cbString, SMTO_ABORTIFHUNG, 50, &ret);
    SendMessageTimeoutA(g_hHookServer, WM_CW_DEBUG, Rect.left, Rect.right, SMTO_ABORTIFHUNG, 50, &ret);
    SendMessageTimeoutA(g_hHookServer, WM_CW_DEBUG, Rect.top,  Rect.bottom, SMTO_ABORTIFHUNG, 50, &ret);
#endif
    
    if (Rect.top < 0) {
        if (Rect.left <= pMouse.x && Rect.right >= pMouse.x)
            bInsideRect = TRUE;
    } else {
        if (PtInRect(&Rect, pMouse)) 
            bInsideRect = TRUE;
    }

    if (bInsideRect) {
        int pos;
        SIZE  iterSize;
        pos =(int)((pMouse.x - Rect.left) * cbString /((float)tSize.cx)+0.5);   
        GetTextExtentPoint32X(hdc, lpszString, pos+1, &iterSize, bWChr);
        if(iterSize.cx < pMouse.x - ptTextPos.x) {
            do {
                pos++;
                GetTextExtentPoint32X(hdc, lpszString, pos+1, &iterSize, bWChr);
            }while((iterSize.cx < pMouse.x - ptTextPos.x) && (pos < cbString-1));
        } else if (iterSize.cx > pMouse.x - ptTextPos.x) {
            do {
                pos--;
                GetTextExtentPoint32X(hdc, lpszString, pos+1, &iterSize, bWChr);
            }while((iterSize.cx > pMouse.x - ptTextPos.x) && (pos > 0));
        }
        g_tPos = pos;
        int size = cbString;
        if (bWChr)
            size = size * sizeof(WCHAR);

        size =  size > 253 ?  253 : size;  // ending with '/0'
        CopyMemory(g_strbuf, lpszString, size);
        g_strbuf[size] = '\0';
        g_strbuf[size+1] = '\0';
        return TRUE;
    }
    return FALSE;
}

static void CaptureTextOutA(HDC hdc, int nXStart, int nYStart, LPCSTR lpszString, int cbString)
{
    if (g_bCapture) {
        cbString = cbString > 256  ? 256 : cbString;
        if (CaptureTextOut(hdc, nXStart, nYStart, lpszString, cbString, FALSE)) {
            g_bWChr = false;
            g_bCapture = false;
            g_cbString = cbString; 
        }
    }
}

static void CaptureTextOutW(HDC hdc, int nXStart, int nYStart, LPCWSTR lpszString, int cbString)
{
    if (g_bCapture) {
        cbString = cbString > 128 ? 128 : cbString;
        if (CaptureTextOut(hdc, nXStart, nYStart, (LPCSTR)lpszString, cbString, TRUE)) {
            g_bWChr = true;
            g_bCapture = false;
            g_cbString = cbString;
        }
    }
}

/* Take this code form GolenDict which is a open source software */
static WCHAR * GlyphindexToUnicode(HDC hdc, LPCWSTR lpszString, int cbString)
{
    LPGLYPHSET ranges;
    WCHAR * allChars, * ptr, * restoredString;
    WORD * allIndices;
    unsigned x;

    // Here we have to decode glyph indices back to chars. We do this
    // by tedious and ineffective iteration.
    //
    ranges = (LPGLYPHSET)malloc( GetFontUnicodeRanges( hdc, 0 ) );
    GetFontUnicodeRanges( hdc, ranges );

    // Render up all available chars into one ridiculously big string
    allChars   = (WCHAR *)malloc( ( ranges->cGlyphsSupported ) * sizeof( WCHAR ) );
    allIndices = (WORD *)malloc( ( ranges->cGlyphsSupported ) * sizeof( WORD ) );

    ptr = allChars;

    for( x = 0; x < ranges->cRanges; ++x ) {
        WCHAR c = ranges->ranges[x].wcLow;
        unsigned y = ranges->ranges[ x ].cGlyphs;
        while( y-- )
          *ptr++ = c++;
    }

    // Amazing. Now get glyph indices for this one nice string.
    GetGlyphIndicesW(hdc, allChars, ranges->cGlyphsSupported, allIndices,
                     GGI_MARK_NONEXISTING_GLYPHS );

    // Fascinating. Now translate our original input string back into
    // its readable form.
    restoredString = (WCHAR *)malloc(cbString * sizeof( WCHAR ));
    for( x = 0; x < cbString; ++x ) {
        unsigned y;
        WORD idx = lpszString[x];
        for( y = 0; y < ranges->cGlyphsSupported; ++y ) {
            if (allIndices[y] == idx ) {
                restoredString[x] = allChars[ y ];
                break;
            }
            if ( y == ranges->cGlyphsSupported ) {
                // Not found
                restoredString[ x ] = L'?';
            }
        }
    }
    // And we're done.

    free(allIndices);
    free(allChars);
    free(ranges);
    return restoredString;
}

BOOL WINAPI TextOutAHook(HDC hdc, int nXStart, int nYStart, LPCSTR lpszString, int cbString)
{
    if (g_bCapture)
        CaptureTextOutA(hdc, nXStart, nYStart, lpszString, cbString);  
    return TextOutAOri(hdc, nXStart, nYStart, lpszString, cbString);
}

BOOL WINAPI TextOutWHook(HDC hdc, int nXStart, int nYStart, LPCWSTR lpszString, int cbString)
{
    if (g_bCapture)
        CaptureTextOutW(hdc, nXStart, nYStart, lpszString, cbString);
    return TextOutWOri(hdc, nXStart, nYStart, lpszString, cbString);
}

BOOL WINAPI ExtTextOutAHook(HDC hdc, int nXStart, int nYStart, UINT fuOptions,
    CONST RECT *lprc, LPCSTR lpszString, UINT cbString, CONST INT *lpDx)
{
    if (g_bCapture)
        CaptureTextOutA(hdc, nXStart, nYStart, lpszString, cbString);
    return ExtTextOutAOri(hdc, nXStart, nYStart, fuOptions, lprc, lpszString, cbString, lpDx);
}

BOOL WINAPI ExtTextOutWHook(HDC hdc, int nXStart, int nYStart, UINT fuOptions,
    CONST RECT *lprc, LPCWSTR lpszString, UINT cbString, CONST INT *lpDx)
{
    if (g_bCapture) {
        if (fuOptions & ETO_GLYPH_INDEX) {
            WCHAR * lpszUString = GlyphindexToUnicode(hdc, lpszString, cbString);
            CaptureTextOutW(hdc, nXStart, nYStart, lpszUString, cbString);
            free((void *)lpszUString);
        } else {
            CaptureTextOutW(hdc, nXStart, nYStart, lpszString, cbString);
        }
    }
    return ExtTextOutWOri(hdc, nXStart, nYStart, fuOptions, lprc, lpszString, cbString, lpDx);
}

static void InstallTextOutHooks()
{
    TextOutAOri = NULL;
    HookAPI("gdi32.dll", "TextOutA",    (PROC)TextOutAHook,    (PROC*)&TextOutAOri);
    
    TextOutWOri = NULL;
    HookAPI("gdi32.dll", "TextOutW",    (PROC)TextOutWHook,    (PROC*)&TextOutWOri);
    
    ExtTextOutAOri = NULL;
    HookAPI("gdi32.dll", "ExtTextOutA", (PROC)ExtTextOutAHook, (PROC*)&ExtTextOutAOri);
    
    ExtTextOutWOri = NULL;
    HookAPI("gdi32.dll", "ExtTextOutW", (PROC)ExtTextOutWHook, (PROC*)&ExtTextOutWOri);
}

static void UninstallTextOutHooks()
{
    if (TextOutAOri != NULL) {
        HookAPI("gdi32.dll", "TextOutA", (PROC)TextOutAOri, NULL);
    }

    if (TextOutWOri != NULL) {
        HookAPI("gdi32.dll", "TextOutW", (PROC)TextOutWOri, NULL);
    }

    if (ExtTextOutAOri != NULL) {
        HookAPI("gdi32.dll", "ExtTextOutA", (PROC)ExtTextOutAOri, NULL);
    }

    if (ExtTextOutWOri != NULL) {
        HookAPI("gdi32.dll", "ExtTextOutW", (PROC)ExtTextOutWOri, NULL);
        //ExtTextOutWOri = NULL; /* May ExtTextOutWHook is being called.  */
    }

    Sleep(20); // Wait TextOutHook series functions exit -- don't know if necessay.
    /* Don't know exactly what happens if TextOutHook series are called 
       when TextOutHooks are being uninstall. It seems Sleep(20) not help. */

}

extern "C" __declspec(dllexport) BOOL 
CaptureTextEnable(HWND hHookServer, HWND hWnd, POINT pMouse, BOOL *isWChr)
{
    ScreenToClient(hWnd, &pMouse);
    g_hHookServer = hHookServer;
    g_pMouse = pMouse;
#if 0
    DWORD ret;
    SendMessageTimeoutA(g_hHookServer, WM_CW_DEBUG, -2, -2, SMTO_ABORTIFHUNG, 50, &ret);
    SendMessageTimeoutA(g_hHookServer, WM_CW_DEBUG, g_pMouse.x, g_pMouse.y, SMTO_ABORTIFHUNG, 50, &ret);
#endif
    if (pMouse.y >= 0) {
        RECT UpdateRect;
	GetClientRect(hWnd, &UpdateRect);
	UpdateRect.top = pMouse.y;
	UpdateRect.bottom = pMouse.y + 1;
        g_bCapture = TRUE;
        g_tPos = -1;
        g_cbString = -1;
	InvalidateRect(hWnd, &UpdateRect, FALSE);
        //InvalidateRect(hWnd, NULL, FALSE);
	UpdateWindow(hWnd);
        *isWChr = g_bWChr;
        g_bCapture = FALSE;
        if (g_tPos != -1)
            return TRUE;
    }
    return FALSE;
}

extern "C" __declspec(dllexport) void GetCaptureText(CHAR *str, int *pos, int *cbString)
{
    CopyMemory(str, g_strbuf, 256);
    *pos = g_tPos;
    *cbString = g_cbString;
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
        break;

    case DLL_PROCESS_DETACH:
	UninstallTextOutHooks();
        break;
    default:
        break;
    }

    return TRUE; /* Loader will check this return code. */
}