//***************************************************************************
//*                                                                         *
//*  wfsdirbrowse.cpp                                                       *
//*                                                                         *
//*  Author: YAMASHITA Katsuhiro                                            *
//*                                                                         *
//*  Create: 2023.04.24 Create                                              *
//*                                                                         *
//***************************************************************************
//
//  Copyright (C) YAMASHITA Katsuhiro. All rights reserved.
//  Licensed under the MIT License.
//
#include "stdafx.h"
#include "wfsdirbrowse.h"
#include "mdichild.h"

 #define _ENABLE_INITIALIZE_ASYNC_OPEN  0

static WCHAR *pszTitle = L"Directory Browse Tool";
static WCHAR *pszWindowClass = L"DirectoryBrowseToolWindow";

static HINSTANCE hInst = NULL;

HWND hWndMain = NULL;
HWND hWndMDIClient = NULL;
static HWND hWndActiveMDIChild = NULL;
#if 0
static HWND hWndFocus = NULL;
#endif
static HMENU hMainMenu = NULL;
static HMENU hMdiMenu = NULL;

bool __initialize_phase = false;

#if  _ENABLE_INITIALIZE_ASYNC_OPEN
#define PM_OPEN_MDICHILD     (WM_APP+101)
#endif

HINSTANCE _GetResourceInstance()
{
	return hInst;
}

HWND _GetMainWnd()
{
	return hWndMain;
}

/////////////////////////////////////////////////////////////////////////////////
#include "..\build.h"

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_INITDIALOG:
		{
			_CenterWindow(hDlg,GetActiveWindow());
			WCHAR *psz = new WCHAR[64];
			StringCchPrintf(psz,128,L"%u.%u.%u.%u Preview",MEJOR_VERSION,MINOR_VERSION,BUILD_NUMBER,PATCH_NUMBER);
			SetDlgItemText(hDlg,IDC_TEXT,psz);
			delete[] psz;
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}

/////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//  PathCheck()
//
//  PURPOSE: Input path validation check.
//
//----------------------------------------------------------------------------
BOOL PathCheck(PCWSTR pszPath)
{
	if( NtPathIsNtDevicePath(pszPath) )
	{
		CStringBuffer szRoot(MAX_PATH);
		
		if( !NtPathGetRootDirectory(pszPath,szRoot,WIN32_MAX_PATH) )
		{
			return FALSE; // volume name only
		}

		if( !NtPathFileExists(pszPath) )
		{
			return FALSE;
		}
	}
	else
	{
		size_t cch = wcslen(pszPath);
		if( cch < 3 )
			return FALSE; // length too short.
		if( pszPath[1] == L':' && pszPath[1] == L'\0' )
			return FALSE;  // drive name only
		if( PathIsRelative(pszPath) )
			return FALSE;
		if( !PathFileExists(pszPath) )
			return FALSE;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
//
//  OpenMDIChild()
//
//  PURPOSE: Open MDI child window.
//
//----------------------------------------------------------------------------
HWND OpenMDIChild(HWND hWnd,PCWSTR pszPath)
{
	if( pszPath )
	{
		if( !PathCheck(pszPath) )
		{
			return NULL;
		}
	}

	MDICREATEPARAM mcp;
	if( pszPath )
		mcp.pszInitialPath = pszPath;
	else
		mcp.pszInitialPath = NULL;

	HWND hwndMDIChild = CreateMDIChildFrame(hWndMDIClient,NULL,(LPARAM)&mcp);

	if( hwndMDIChild )
	{
		if( hMdiMenu == NULL )
			hMdiMenu = LoadMenu(_GetResourceInstance(),MAKEINTRESOURCE(IDR_MDICHILDFRAME));

		SendMessage(hWndMDIClient,WM_MDISETMENU,(WPARAM)hMdiMenu,0);
		DrawMenuBar(hWnd);

		MDICLIENTWNDDATA *pd = (MDICLIENTWNDDATA *)GetWindowLongPtr(hwndMDIChild,GWLP_USERDATA);
		{
			pd->hWndView = CreateDirectoryBrowseTool(hwndMDIChild);
			InitDirectoryBrowseTool(pd->hWndView,mcp.pszInitialPath,NULL);

			RECT rc;
			GetClientRect(hwndMDIChild,&rc);
			SetWindowPos(pd->hWndView,NULL,0,0,rc.right-rc.left,rc.bottom-rc.top,SWP_NOZORDER);
		}
	}

	return hwndMDIChild;
}

//----------------------------------------------------------------------------
//
//  FUNCTION: RegisterMDIFrameClass()
//
//  PURPOSE: Register main frame window class.
//
//----------------------------------------------------------------------------
ATOM RegisterMDIFrameClass(HINSTANCE hInstance)
{
	extern LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	WNDCLASSEX wcex = {0};

	wcex.cbSize        = sizeof(wcex);
	wcex.style         = CS_HREDRAW|CS_VREDRAW;
	wcex.lpfnWndProc   = WndProc;
	wcex.cbClsExtra	   = 0;
	wcex.cbWndExtra	   = 0;
	wcex.hInstance	   = hInstance;
	wcex.hIcon		   = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
	wcex.hCursor	   = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName  = NULL;
	wcex.lpszClassName = pszWindowClass;
	wcex.hIconSm	   = (HICON)LoadImage(wcex.hInstance, MAKEINTRESOURCE(IDI_MAIN),IMAGE_ICON,16,16,LR_DEFAULTSIZE);

	return RegisterClassEx(&wcex);
}

//----------------------------------------------------------------------------
//
//  InitInstance()
//
//  PURPOSE: Initialize this instance.
//
//----------------------------------------------------------------------------
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance;

	hWnd = CreateWindow(pszWindowClass, pszTitle, WS_OVERLAPPEDWINDOW,
				  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	hMainMenu = LoadMenu(_GetResourceInstance(),MAKEINTRESOURCE(IDR_MAINFRAME));
	SetMenu(hWnd,hMainMenu);

	__initialize_phase = true;

	extern int __argc;
	extern wchar_t **__wargv;

	if( __argc > 1 )
	{
		for(int i = 1; i < __argc; i++)
		{
#if _ENABLE_INITIALIZE_ASYNC_OPEN
			PostMessage(hWnd,PM_OPEN_MDICHILD,0,(LPARAM)__wargv[i]); // aync open
#else
			HWND hwndMDIChild;
			hwndMDIChild = OpenMDIChild(hWnd,__wargv[i]);
			if( hwndMDIChild )
			{
				//
				// Windows 7 Basicで実行時、ウィンドウ上端の端が黒く残る現象を回避する処置。
				//
				SetWindowPos(hwndMDIChild,0,0,0,0,0,SWP_SHOWWINDOW|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED|SWP_DRAWFRAME);
			}
#endif
		}
	}


#if !_ENABLE_INITIALIZE_ASYNC_OPEN
	__initialize_phase = false;
#endif

	// Show frame window
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// Set focus on active MDI child window.
//	hWndActiveMDIChild = MDIGetActiveChild(hWndMDIClient);
//	SetFocus( hWndActiveMDIChild );

#if _ENABLE_INITIALIZE_ASYNC_OPEN
	if( __argc > 1 )
		PostMessage(hWnd,PM_OPEN_MDICHILD,0,0); // end of aync open
#endif

	return hWnd;
}

//----------------------------------------------------------------------------
//
//  ExitInstance()
//
//  PURPOSE: Exit this instance.
//
//----------------------------------------------------------------------------
VOID ExitInstance()
{
	;
}

//----------------------------------------------------------------------------
//
//  QueryCmdState()
//
//  PURPOSE: Query command status.
//
//----------------------------------------------------------------------------
INT CALLBACK QueryCmdState(UINT CmdId,PVOID,LPARAM)
{
	HWND hwndMDIChild = MDIGetActiveChild(hWndMDIClient);
	if( hwndMDIChild )
	{
		MDICLIENTWNDDATA *pd = (MDICLIENTWNDDATA *)GetWindowLongPtr(hwndMDIChild,GWLP_USERDATA);
		UINT State = 0;
		if( SendMessage(pd->hWndView,WM_QUERY_CMDSTATE,MAKEWPARAM(CmdId,0),(LPARAM)&State) )
		{
			return State;
		}
	}

	switch( CmdId )
	{
		case ID_FILE_NEW:
		case ID_FILE_CLOSE:
		case ID_ABOUT:
		case ID_EXIT:
			return UPDUI_ENABLED;
	}

	return UPDUI_DISABLED;
}

//----------------------------------------------------------------------------
//
//  WndProc()
//
//  PURPOSE: Main frame window procedure.
//
//----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE: 
		{
			hWndMDIClient = CreateMDIClient(hWnd);
			break; 
		} 
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		case WM_SIZE:
		{
			// Resizes the MDI client window to fit in the new frame window's client area. 
			// If the frame window procedure sizes the MDI client window to a different size, 
			// it should not pass the message to the DefWindowProc function.
			int cx = GET_X_LPARAM(lParam);
			int cy = GET_Y_LPARAM(lParam);
			SetWindowPos(hWndMDIClient,NULL,0,0,cx,cy,SWP_NOZORDER);
			break;
		}
		case WM_COMMAND:
		{
			int wmId, wmEvent;
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			switch (wmId)
			{
				case ID_FILE_NEW:
				{
					HWND hwndChild = OpenMDIChild(hWnd,NULL);
					if( hwndChild )
					{
						MDICLIENTWNDDATA *pd = (MDICLIENTWNDDATA *)GetWindowLongPtr(hwndChild,GWLP_USERDATA);
						SetFocus(pd->hWndView);
					}
					break;
				}
				case ID_FILE_CLOSE:
					SendMessage(hWndMDIClient, WM_MDIDESTROY, (WPARAM)MDIGetActiveChild(hWndMDIClient), 0L); 
					break;
				case ID_ABOUT:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				case ID_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
				{
					HWND hwndMDIChild = MDIGetActiveChild(hWndMDIClient);
					if( hwndMDIChild )
					{
						MDICLIENTWNDDATA *pd = (MDICLIENTWNDDATA *)GetWindowLongPtr(hwndMDIChild,GWLP_USERDATA);
						SendMessage(pd->hWndView,WM_COMMAND,wParam,lParam);
					}
					break;
				}
			}
			break;
		}
		case WM_MDIACTIVATE:
		{
			hWndActiveMDIChild = (HWND)lParam;

			if( hWndActiveMDIChild == NULL )
			{
				SendMessage(hWndMDIClient,WM_MDISETMENU,(WPARAM)hMainMenu,0);
				DrawMenuBar(hWnd);
			}
			break;
		}
#if _ENABLE_INITIALIZE_ASYNC_OPEN
		case PM_OPEN_MDICHILD:
		{
			if( lParam )
				OpenMDIChild(hWnd,(PCWSTR)lParam);
			else
				__initialize_phase = false;
			return 0;
		}
#endif
	}

	return DefFrameProc(hWnd, hWndMDIClient, message, wParam, lParam);
}

//----------------------------------------------------------------------------
//
//  WinMain()
//
//  PURPOSE: Main procedure.
//
//----------------------------------------------------------------------------
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	_wsetlocale(LC_ALL,L"");

	RegisterMDIFrameClass(hInstance);
	RegisterMDIChildFrameClass(hInstance);

	if( (hWndMain = InitInstance (hInstance, nCmdShow)) == NULL )
	{
		return FALSE;
	}

	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));

	MSG msg;
	INT ret;
	while ((ret = GetMessage(&msg, (HWND) NULL, 0, 0)) != 0)
	{
		if( ret == -1 )
	    {
		    break;// handle the error and possibly exit
	    }
		else 
	    { 
			if( (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) && (msg.wParam == VK_RETURN) )
			{
				// prevent ring beep when press enter key on tree-view.
				DispatchMessage(&msg);
				continue;
			}

		    if (!TranslateMDISysAccel(hWndMDIClient, &msg) && 
			    !TranslateAccelerator(hWndMain, hAccelTable, &msg))
	        { 
				if( hWndActiveMDIChild )
				{
					if( IsDialogMessage(hWndActiveMDIChild,&msg) )
						continue;
				}
		        TranslateMessage(&msg); 
			    DispatchMessage(&msg); 
			} 
		} 
	}

	ExitInstance();

	return (int) msg.wParam;
}
