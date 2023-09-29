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

static WCHAR *pszTitle = L"FSDirBrowseTool";
static WCHAR *pszWindowClass = L"MainFrameWindow";
static HINSTANCE hInst = NULL;
static HWND hWndMain = NULL;
static HWND hWndMDIClient = NULL;
static HWND hWndActiveMDIChild = NULL;
static HMENU hMainMenu = NULL;
static HMENU hMdiMenu = NULL;

bool __initialize_phase = false;

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

int
CALLBACK
BrowseCallbackProc(
	HWND hwnd,
	UINT uMsg,
	LPARAM lParam,
	LPARAM lpData
	)
{
	switch(uMsg)
	{
		case BFFM_INITIALIZED:
		{
			if( lpData != 0 && (*(LPCTSTR)lpData) != _T('\0') )
			{
				SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)lpData);
			}
			SetFocus(GetDlgItem(hwnd,100));
			break;
		}
		case BFFM_SELCHANGED:
		{
			WCHAR szFolder[MAX_PATH];
			if( SHGetPathFromIDList((LPCITEMIDLIST)lParam,szFolder) )
			{
				if( !PathIsDirectory(szFolder) )
				{
					SendMessage(hwnd,BFFM_ENABLEOK,0,0);
				}
			}
			break;
		}
	}

	return 0;
}

BOOL ChooseFolder(HWND hWnd,LPWSTR pszFolder,LPCWSTR pszCurrentFolder)
{
	LPITEMIDLIST pidl;

	BROWSEINFO bi = {0};
	WCHAR szDisplayName[MAX_PATH];
	bi.hwndOwner = hWnd;
	bi.ulFlags = BIF_DONTGOBELOWDOMAIN|
				 BIF_RETURNONLYFSDIRS|
				 BIF_NONEWFOLDERBUTTON|
				 BIF_EDITBOX|
				 BIF_NEWDIALOGSTYLE;

	bi.pszDisplayName = szDisplayName;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)pszCurrentFolder;
	bi.lpszTitle = L"Choose Folder";

	pidl = SHBrowseForFolder(&bi);

	if( pidl == NULL )
		return FALSE;

	SHGetPathFromIDList(pidl,pszFolder);

	ILFree(pidl);

	if( !PathIsDirectory(pszFolder) )
	{
		MessageBeep(-1);
		return FALSE;
	}

	return TRUE;
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
		WCHAR szRoot[MAX_PATH];
		
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

	HWND hwndMDIChild = CreateMDIChildFrame(hWndMDIClient,NULL,(LPARAM)&mcp,FALSE);

	if( hwndMDIChild )
	{
		if( hMdiMenu == NULL )
			hMdiMenu = LoadMenu(_GetResourceInstance(),MAKEINTRESOURCE(IDR_MDICHILDFRAME));

		SendMessage(hWndMDIClient,WM_MDISETMENU,(WPARAM)hMdiMenu,0);
		DrawMenuBar(hWnd);

		MDICLIENTWNDDATA *pd = (MDICLIENTWNDDATA *)GetWindowLongPtr(hwndMDIChild,GWLP_USERDATA);
		{
			pd->hWndView = CreateDirectoryBrowseTool(hwndMDIChild);

			RECT rc;
			GetClientRect(hwndMDIChild,&rc);
			SetWindowPos(pd->hWndView,NULL,0,0,rc.right-rc.left,rc.bottom-rc.top,SWP_NOZORDER);

			SendMessage(pd->hWndView,WM_CONTROL_MESSAGE,CTRL_SET_DIRECTORY,(LPARAM)mcp.pszInitialPath);
			SendMessage(pd->hWndView,WM_CONTROL_MESSAGE,CTRL_INIT_LAYOUT,(LPARAM)&rc);
		}
	}

	return hwndMDIChild;
}

//----------------------------------------------------------------------------
//
//  RegisterMDIFrameClass()
//
//  PURPOSE:
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

	// Load MainFrame menu
	hMainMenu = LoadMenu(_GetResourceInstance(),MAKEINTRESOURCE(IDR_MAINFRAME));
	SetMenu(hWnd,hMainMenu);

	__initialize_phase = true;

	extern int __argc;
	extern wchar_t **__wargv;

	if( __argc > 1 )
	{
		for(int i = 1; i < __argc; i++)
		{
			HWND hwndMDIChild;
			hwndMDIChild = OpenMDIChild(hWnd,__wargv[i]);
			if( hwndMDIChild )
			{
				//
				// Windows 7 Basicで実行時、ウィンドウ上端の端が黒く残る現象を回避する処置。
				//
				SetWindowPos(hwndMDIChild,0,0,0,0,0,SWP_SHOWWINDOW|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED|SWP_DRAWFRAME);
			}
		}
	}

	__initialize_phase = false;

	// Show frame window
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// Set focus on active MDI child window.
	hWndActiveMDIChild = MDIGetActiveChild(hWndMDIClient);

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
	if( hMainMenu )
		DestroyMenu(hMainMenu);
	if( hMdiMenu )
		DestroyMenu(hMdiMenu);
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
		case ID_FILE_OPEN:
		case ID_FILE_CHOOSE_DIRECTORY:
		case ID_ABOUT:
		case ID_EXIT:
			return UPDUI_ENABLED;
		case ID_FILE_CLOSE:
			if( hwndMDIChild != NULL )
				return UPDUI_ENABLED;
			break;
	}

	return UPDUI_DISABLED;
}

//----------------------------------------------------------------------------
//
//  OnFileNew()
//
//  PURPOSE: Open New MDI child window.
//
//----------------------------------------------------------------------------
INT_PTR OnFileNew(HWND hWnd,UINT uCmdId,UINT codeNotify,HWND hwndCtl) 
{
	WCHAR szCurPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,szCurPath);

	HWND hwndChild = OpenMDIChild(hWnd,szCurPath);
	if( hwndChild )
	{
		MDICLIENTWNDDATA *pd = (MDICLIENTWNDDATA *)GetWindowLongPtr(hwndChild,GWLP_USERDATA);
		SetFocus(pd->hWndView);
	}
	return 0;
}

//----------------------------------------------------------------------------
//
//  OnFileOpen()
//
//  PURPOSE: Open MDI child window with choose folder.
//
//----------------------------------------------------------------------------
INT_PTR OnFileOpen(HWND hWnd,UINT uCmdId,UINT codeNotify,HWND hwndCtl) 
{
	WCHAR szFolder[MAX_PATH];
	if( !ChooseFolder(hWnd,szFolder,NULL) )
		return 0;

	HWND hwndChild = OpenMDIChild(hWnd,szFolder);
	if( hwndChild )
	{
		MDICLIENTWNDDATA *pd = (MDICLIENTWNDDATA *)GetWindowLongPtr(hwndChild,GWLP_USERDATA);
		SetFocus(pd->hWndView);
	}
	return 0;
}

//----------------------------------------------------------------------------
//
//  OnFileChooseDirectory()
//
//  PURPOSE: Choose Directory.
//
//----------------------------------------------------------------------------
INT_PTR OnFileChooseDirectory(HWND hWnd,UINT uCmdId,UINT codeNotify,HWND hwndCtl) 
{
	WCHAR szFolder[MAX_PATH];
	if( ChooseFolder(hWnd,szFolder,NULL) )
	{
		HWND hwndMDIChild = MDIGetActiveChild(hWndMDIClient);
		if( hwndMDIChild )
		{
			MDICLIENTWNDDATA *pd = (MDICLIENTWNDDATA *)GetWindowLongPtr(hwndMDIChild,GWLP_USERDATA);
			SendMessage(pd->hWndView,WM_CONTROL_MESSAGE,CTRL_SET_DIRECTORY,(LPARAM)szFolder);
		}
	}
	return 0;
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
			UINT uCmdId,uNotifyCode;
			uCmdId      = LOWORD(wParam);
			uNotifyCode = HIWORD(wParam);
			switch(uCmdId)
			{
				case ID_FILE_NEW:
					OnFileNew(hWnd,uCmdId,uNotifyCode,(HWND)lParam);
					break;
				case ID_FILE_OPEN:
					OnFileOpen(hWnd,uCmdId,uNotifyCode,(HWND)lParam);
					break;
				case ID_FILE_CHOOSE_DIRECTORY:
					OnFileChooseDirectory(hWnd,uCmdId,uNotifyCode,(HWND)lParam);
					break;
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
			else
			{
				SendMessage(hWndMDIClient,WM_MDISETMENU,(WPARAM)hMdiMenu,0);
				DrawMenuBar(hWnd);
			}
			break;
		}
		case WM_INITMENUPOPUP:
		{
			INT RelativePosition = (INT)LOWORD(lParam);
			INT WindowMenu = HIWORD(lParam);
			if( !WindowMenu )
				UpdateUI_MenuItem((HMENU)wParam,&QueryCmdState,0);
			break;
		}
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

	_MemInit();

	SHFileIconInit(FALSE);

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

	_MemEnd();

	return (int) msg.wParam;
}
