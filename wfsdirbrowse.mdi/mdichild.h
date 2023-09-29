#pragma once

#define MDICHILD_CLASSNAME  L"MDIChild";
#define CBWNDEXTRA   256

ATOM RegisterMDIChildFrameClass(HINSTANCE hInstance);
HWND CreateMDIClient(HWND hWnd);
HWND CreateMDIChildFrame(HWND hWndMDIClient,PCWSTR pszTitle,LPARAM lParam,BOOL bMaximize);

__inline HWND MDIGetActiveChild(HWND hwndMDIClient) { return (HWND)SendMessage(hwndMDIClient,WM_MDIGETACTIVE,0,0); }

typedef struct _MDICREATEPARAM
{
	PCWSTR pszInitialPath;
} MDICREATEPARAM;

typedef struct _MDICLIENTWNDDATA
{
	HWND hWndView;
	HWND hWndFocus;
} MDICLIENTWNDDATA;
