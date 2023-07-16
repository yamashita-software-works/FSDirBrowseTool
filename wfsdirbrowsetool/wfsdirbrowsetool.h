#pragma once

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
