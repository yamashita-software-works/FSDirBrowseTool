#include "stdafx.h"
#include "dirbrowsehost.h"

HINSTANCE hInstance = NULL;

HINSTANCE _GetResourceInstance()
{
	return hInstance;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			hInstance = hModule;
			_MemInit();
			break;
		case DLL_PROCESS_DETACH:
			_MemEnd();
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}

EXTERN_C
HWND
WINAPI
CreateDirectoryBrowseTool(
	HWND hwnd
	)
{
	return DirectoryBrowseTool_CreateWindow(hwnd);
}

EXTERN_C
BOOL
WINAPI
InitDirectoryBrowseTool(
	HWND hwndDBT,
	PCWSTR pszPath,
	RECT *prc
	)
{
	WCHAR szCurPath[MAX_PATH];
	if( pszPath == NULL )
	{
		GetCurrentDirectory(MAX_PATH,szCurPath);
		pszPath = szCurPath;
	}
	DirectoryBrowseTool_InitData(hwndDBT,pszPath);
	DirectoryBrowseTool_InitLayout(hwndDBT,prc);
	return TRUE;
}
