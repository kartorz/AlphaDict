/*
 * Copyright (c) 2014  AlphaDict.
 *
 * I google(d) and referred some articles especially "API hooking revealed",
 * and took some code from web.
 *
 */

#define _WIN32_WINNT 0x0501
#include "HookAPI.h"

#include <crtdbg.h>
#include <tlhelp32.h>

//#include <WinDef.h> 

#define MakePtr(cast, base, off) (cast)((DWORD)(base)+(DWORD)(off))

typedef struct tag_HOOKFUNCDESC
{
    LPCSTR szFunc;  // The name of the function to hook.
    PROC pProc;     // The procedure to blast in.
} HOOKFUNCDESC , * LPHOOKFUNCDESC;

static BOOL IsNT()
{
    OSVERSIONINFO stOSVI;
    BOOL bRet;

    memset(&stOSVI, 0, sizeof(OSVERSIONINFO));
    stOSVI.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    bRet = GetVersionEx(&stOSVI);
    _ASSERT(TRUE == bRet);
    if (FALSE == bRet)
        return FALSE;
    return (VER_PLATFORM_WIN32_NT == stOSVI.dwPlatformId);
}

/* WINNT.H
* typedef struct _IMAGE_DOS_HEADER {  // DOS .EXE header
*    USHORT e_magic;         // Magic number
*    USHORT e_cblp;          // Bytes on last page of file
*    USHORT e_cp;            // Pages in file
*    USHORT e_crlc;          // Relocations
*    USHORT e_cparhdr;       // Size of header in paragraphs
*    USHORT e_minalloc;      // Minimum extra paragraphs needed
*    USHORT e_maxalloc;      // Maximum extra paragraphs needed
*    USHORT e_ss;            // Initial (relative) SS value
*    USHORT e_sp;            // Initial SP value
*    USHORT e_csum;          // Checksum
*    USHORT e_ip;            // Initial IP value
*    USHORT e_cs;            // Initial (relative) CS value
*    USHORT e_lfarlc;        // File address of relocation table
*    USHORT e_ovno;          // Overlay number
*    USHORT e_res[4];        // Reserved words
*    USHORT e_oemid;         // OEM identifier (for e_oeminfo)
*    USHORT e_oeminfo;       // OEM information; e_oemid specific
*    USHORT e_res2[10];      // Reserved words
*    LONG   e_lfanew;        // File address of new exe header
* } IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

* typedef struct _IMAGE_OPTIONAL_HEADER {
*    //
*    // Standard fields.
*    //
*    USHORT  Magic;
*    UCHAR   MajorLinkerVersion;
*    UCHAR   MinorLinkerVersion;
*    ULONG   SizeOfCode;
*    ULONG   SizeOfInitializedData;
*    ULONG   SizeOfUninitializedData;
*    ULONG   AddressOfEntryPoint;
*    ULONG   BaseOfCode;
*    ULONG   BaseOfData;
*    //
*    // NT additional fields.
*    //
*    ULONG   ImageBase;
*    ULONG   SectionAlignment;
*    ULONG   FileAlignment;
*    USHORT  MajorOperatingSystemVersion;
*    USHORT  MinorOperatingSystemVersion;
*    USHORT  MajorImageVersion;
*    USHORT  MinorImageVersion;
*    USHORT  MajorSubsystemVersion;
*    USHORT  MinorSubsystemVersion;
*    ULONG   Reserved1;
*    ULONG   SizeOfImage;
*    ULONG   SizeOfHeaders;
*    ULONG   CheckSum;
*    USHORT  Subsystem;
*    USHORT  DllCharacteristics;
*    ULONG   SizeOfStackReserve;
*    ULONG   SizeOfStackCommit;
*    ULONG   SizeOfHeapReserve;
*    ULONG   SizeOfHeapCommit;
*    ULONG   LoaderFlags;
*    ULONG   NumberOfRvaAndSizes;
*    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
* } IMAGE_OPTIONAL_HEADER, *PIMAGE_OPTIONAL_HEADER;
*/


static PIMAGE_IMPORT_DESCRIPTOR 
GetNamedImportDescriptor(HMODULE hModule, LPCSTR szImportModule)
{
    if ((szImportModule == NULL) || (hModule == NULL)) {
        _ASSERT(FALSE);
        SetLastErrorEx(ERROR_INVALID_PARAMETER, SLE_ERROR);
        return NULL;
    }
    /*   The Portable Executable File Format 
     * http://www.csn.ul.ie/~caolan/publink/winresdump/winresdump/doc/pefile2.html
     *     ----------------------
     *     |     MS-DOS          |   
     *     |   MZ Header         |
     *     -----------------------
     *     | MS-DOS Real-Mode    | 
     *     |   Stub Program      |
     *     -----------------------
     *     | PE File Signature   |
     *     -----------------------
     *     | PE File Header      |
     *     -----------------------
     *     |      PE File        |
     *     | Optional Header     |
     *     -----------------------
     *     |.text Section Header |
     *     -----------------------
     *     |.bss Section Header  |
     *     -----------------------
     *     |.rdata Section Header|
     *     -----------------------
     *     |       ...           |
     *     ----------------------
     *     |   .text Section     |
     *     -----------------------
     *     |   .bss Section      |
     *     -----------------------
     *     |   .rdata Section    |
     *     -----------------------
     *     |     ...             |
     *         
     */
    PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER) hModule;
    // check MZ header
    if (IsBadReadPtr(pDOSHeader, sizeof(IMAGE_DOS_HEADER)) ||
        (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)) {
        _ASSERT(FALSE);
        SetLastErrorEx(ERROR_INVALID_PARAMETER, SLE_ERROR);
        return NULL;
    }

    // check PE header
    PIMAGE_NT_HEADERS pNTHeader = MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);
    if (IsBadReadPtr(pNTHeader, sizeof(IMAGE_NT_HEADERS)) ||
        (pNTHeader->Signature != IMAGE_NT_SIGNATURE)) {
        _ASSERT(FALSE);
        SetLastErrorEx(ERROR_INVALID_PARAMETER, SLE_ERROR);
        return NULL;
    }

    // check .idata section
    if (pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress == 0)
        return NULL;
    PIMAGE_IMPORT_DESCRIPTOR pImportDesc = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, pDOSHeader,
    pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    while (pImportDesc->Name) {
        PSTR szCurrMod = MakePtr(PSTR, pDOSHeader, pImportDesc->Name);
        if (lstrcmpiA(szCurrMod, szImportModule) == 0)
            break;
        pImportDesc++;
    }

    if (pImportDesc->Name == NULL)
        return NULL;
    return pImportDesc;
}

/*  This function and  GetNamedImportDescriptor, I took from web, 
 *  lots of webpage containing the code */
static BOOL ImportHookFunction(HMODULE hModule, LPCSTR szImportModule, 
                          LPCSTR szHookFunc, PROC pHookFunc, PROC* pOrigFuncPtr)
{
    if (szImportModule == NULL || szHookFunc == NULL) {
        _ASSERT(FALSE);
        SetLastErrorEx(ERROR_INVALID_PARAMETER, SLE_ERROR);
        return FALSE;
    }

    // The address above 0x80000000  is shared by all processes.
    if (!IsNT() && ((DWORD)hModule >= 0x80000000)) {
        _ASSERT(FALSE);
        SetLastErrorEx(ERROR_INVALID_HANDLE, SLE_ERROR);
        return FALSE;
    }

    if (pOrigFuncPtr) {
        //memset(pOrigFuncPtr, NULL, sizeof(PROC));
    }

    PIMAGE_IMPORT_DESCRIPTOR pImportDesc = GetNamedImportDescriptor(hModule, szImportModule);
    if (pImportDesc == NULL)
        return FALSE;

    // Seek to OriginalFirstThunk
    PIMAGE_THUNK_DATA pOrigThunk = MakePtr(PIMAGE_THUNK_DATA, hModule, pImportDesc->OriginalFirstThunk);
    PIMAGE_THUNK_DATA pRealThunk = MakePtr(PIMAGE_THUNK_DATA, hModule, pImportDesc->FirstThunk);
    while (pOrigThunk->u1.Function) {
        if (IMAGE_ORDINAL_FLAG != (pOrigThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)) {
            PIMAGE_IMPORT_BY_NAME pByName = MakePtr(PIMAGE_IMPORT_BY_NAME, 
                                                    hModule,
                                                    pOrigThunk->u1.AddressOfData);
            if ('/0' == pByName->Name[0])
                goto NEXT_THUNK;

            BOOL bDoHook = FALSE;

            /*if ((szHookFunc[0] == pByName->Name[0])
                && (_strcmpi(szHookFunc, (char*)pByName->Name) == 0))*/
            if (lstrcmpiA(szHookFunc, (char*)pByName->Name) == 0)  // not case-sensitive
            {
                if (pHookFunc)
                    bDoHook = TRUE;
            }
            if (bDoHook) {
                // change  mem's protection state.
                MEMORY_BASIC_INFORMATION mbi_thunk;
                VirtualQuery(pRealThunk, &mbi_thunk, sizeof(MEMORY_BASIC_INFORMATION));
                VirtualProtect(mbi_thunk.BaseAddress,
                               mbi_thunk.RegionSize,
                               PAGE_READWRITE,
                               &mbi_thunk.Protect);

                if (pOrigFuncPtr)
                    *pOrigFuncPtr = (PROC)(pRealThunk->u1.Function);
                // replace the origin function address with our hook
                pRealThunk->u1.Function = (DWORD)(pHookFunc);

                DWORD dwOldProtect;
                VirtualProtect(mbi_thunk.BaseAddress,
                               mbi_thunk.RegionSize,
                               mbi_thunk.Protect,
                               &dwOldProtect);
                SetLastError(ERROR_SUCCESS);
                return TRUE;
            } // bDoHook
        }// if (IMAGE_ORDINAL_FLAG

NEXT_THUNK:
        pOrigThunk++;
        pRealThunk++;
    }
    return TRUE;
}


BOOL HookAPI(LPCSTR szImportModule, LPCSTR szHookFunc, PROC pHookFunc, PROC* pOrigFuncPtr)
{
    HANDLE hSnapshot;
    MODULEENTRY32 me;
    me.dwSize = sizeof(MODULEENTRY32);
    BOOL bOk;
    if ((szImportModule == NULL) || (szHookFunc == NULL)) {
        return FALSE;
    }

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
    bOk = Module32First(hSnapshot,&me);
    while (bOk) {
        ImportHookFunction(me.hModule, szImportModule, szHookFunc, pHookFunc, pOrigFuncPtr);
	bOk = Module32Next(hSnapshot,&me);
    }
    CloseHandle(hSnapshot);
    return TRUE;
}
