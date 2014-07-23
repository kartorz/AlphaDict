/*
 * Copyright (c) 2014  AlphaDict.
 *
 * Interface for All int
 *
 */
#ifndef _HOOKAPI_H_
#define _HOOKAPI_H_

#include <Windows.h>

extern BOOL HookAPI(LPCSTR szImportModule, 
             LPCSTR szHookFunc,
             PROC pHookFunc,
             PROC* pOrigFuncPtr);

#endif
