//****************************************************************************
//*                                                                          *
//*  Mem.h                                                                   *
//*                                                                          *
//*  PURPOSE:   Memory allocate function                                     *
//*                                                                          *
//*  HISTORY:   1999.06.19 Create                                            *
//*             2001.02.27 Appneded _MemReAlloc                              *
//*             2010.12.02 Appneded _MemAllocStringCat                       *
//****************************************************************************
//
//  Copyright (C) YAMASHITA Katsuhiro. All rights reserved.
//  Licensed under the MIT License.
//
#ifndef _MEM_
#define _MEM_

#ifdef _DEBUG
//
// Debug Version
//
VOID*   _MemAllocDebug(SIZE_T cb,LPSTR File,int Line);
PTSTR   _MemAllocStringDebug(PCTSTR sz,LPSTR File,int Line);
VOID*   _MemReAllocDebug(VOID *pv,SIZE_T cb,LPSTR File,int Line);
VOID*   _MemAllocZeroDebug(SIZE_T cb,LPSTR File,int Line);
PTSTR   _MemAllocStringBufferDebug(SIZE_T cch,LPSTR File,int Line);
PTSTR   _MemAllocStringCatDebug(PCTSTR psz1,PCTSTR psz2,LPSTR File,int Line);
#define _MemAlloc(cb)         _MemAllocDebug(cb,(LPSTR)__FILE__,__LINE__)
#define _MemAllocString(p)    _MemAllocStringDebug(p,(LPSTR)__FILE__,__LINE__)
#define _MemReAlloc(pv,cb)    _MemReAllocDebug(pv,cb,(LPSTR)__FILE__,__LINE__)
#define _MemAllocZero(cb)     _MemAllocZeroDebug(cb,(LPSTR)__FILE__,__LINE__)
#define _MemAllocStringBuffer(cch) _MemAllocStringBufferDebug(cch,(LPSTR)__FILE__,__LINE__)
#define _MemAllocStringCat(psz1,psz2)   _MemAllocStringCatDebug(psz1,psz2,(LPSTR)__FILE__,__LINE__)

void* __cdecl operator new(size_t nSize, LPCSTR lpszFileName, int nLine);
void operator delete(void *pMem, LPCSTR lpszFileName, int nLine);

#define DEBUG_NEW new(__FILE__,__LINE__)
#define new DEBUG_NEW

void  _MemDebugDumpMemoryLeaks();

#else
//
// Release Version
//
VOID*   _MemAlloc(SIZE_T cb);
PTSTR   _MemAllocString(PCTSTR sz);
VOID*   _MemReAlloc(VOID*,SIZE_T cb);
VOID*   _MemAllocZero(SIZE_T cb);
PTSTR   _MemAllocStringBuffer(SIZE_T cch);
PTSTR   _MemAllocStringCat(PCTSTR psz1,PCTSTR psz2);

#define _MemDebugDumpMemoryLeaks  NULL

#endif

VOID  _MemInit();
VOID  _MemEnd();
VOID  _MemFree(VOID *p);

#define _MemDupString _MemAllocString
#define _MemSafeFree(p) if(p) { _MemFree(p); p = NULL; }

#define _SafeMemFree(p)   if( p != NULL ) { _MemFree( (PVOID)p ); p = NULL; }
#define _SafeDelete(p) if( p != NULL ) { delete( p ); p = NULL; }
#define _SafeDeleteArray(p) if( p != NULL ) { delete( p ); p = NULL; }

#define SAFE_DELETE(p)        if( p != NULL ) {delete p; p=NULL;}
#define SAFE_DELETE_ARRAY(p)  if( p != NULL ) {delete[] p; p=NULL;}
#define SAFE_FREE(p)          if( p != NULL ) {_MemFree(p); p=NULL;}
#define SAFE_RELEASE(p)       if( p != NULL ) {p->Release(); p=NULL;}

#endif
