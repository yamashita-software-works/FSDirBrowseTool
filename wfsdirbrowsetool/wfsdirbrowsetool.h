#pragma once

// Win32 build header

EXTERN_C
HRESULT
WINAPI
ExtraDialog(
	HWND hWnd,
	UINT Type,
	UINT Flags,
	PCWSTR pszPath,
	RECT *pRect,
	WPARAM wParam,
	LPARAM lParam,
	PVOID pParam,
	PVOID Context
	);

EXTERN_C
HWND
WINAPI
CreateDirectoryBrowseTool(
	HWND hwnd
	);

EXTERN_C
BOOL
WINAPI
InitDirectoryBrowseTool(
	HWND hwndDBT,
	PCWSTR pszPath,
	RECT *prc
	);
