// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <winternl.h> // WinSDK 7.1
#include <shlobj.h>


#if _MSC_VER <= 1500
#define nullptr NULL
#endif

#pragma warning(disable : 4995)

#include "debug.h"
#include "mem.h"

#include <winioctl.h>
#include "..\wfslib\wfslib.h"
#include "..\wfsdirbrowsetool\wfsdirbrowsetool.h"
#include "..\libntwdk\libntwdk.h"
#include "..\libmisc\libmisc.h"

#define SetRedraw(h,f)	SendMessage(h,WM_SETREDRAW,(WPARAM)f,0)

#define GETINSTANCE(hWnd)   (HINSTANCE)GetWindowLongPtr(hWnd,GWLP_HINSTANCE)
#define GETCLASSBRUSH(hWnd) (HBRUSH)GetClassLongPtr(hWnd,GCLP_HBRBACKGROUND)

HINSTANCE _GetResourceInstance();
HWND _GetMainWnd();
extern bool __initialize_phase;
