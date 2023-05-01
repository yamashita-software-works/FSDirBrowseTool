//***************************************************************************
//*                                                                         *
//*  mainframe.cpp                                                          *
//*                                                                         *
//*  WfsDirBrowseTool MainFrame                                             *
//*                                                                         *
//*  Create: 2023-03-29                                                     *
//*                                                                         *
//*  Author: YAMASHITA Katsuhiro                                            *
//*                                                                         *
//*  Copyright (C) YAMASHITA Katsuhiro. All rights reserved.                *
//*  Licensed under the MIT License.                                        *
//*                                                                         *
//***************************************************************************
#include "stdafx.h"
#include "resource.h"
#include "wfsdirbrowsetool.h"
#include "dirbrowsehost.h"

#if _ENABLE_MODAL_DIALOG_SUPPORT
extern HINSTANCE hInstance;
static HWND g_hWndView = NULL;
static HWND g_hWndFocus = NULL;
static PCWSTR Title = L"Directory File Information Browse Tool";
static PCWSTR pszMainFrameClassName = L"DirectoryBrowseTool_WndClass";

//---------------------------------------------------------------------------
//
//  FUNCTION: SaveFocusControl()
//
//  PURPOSE:
//
//---------------------------------------------------------------------------
BOOL SaveFocusControl()
{
	// save focus window if focus is on this window's controls
	HWND hWndFocus = GetFocus();

	if (hWndFocus != NULL )
	{
		g_hWndFocus = hWndFocus;
		return TRUE;
	}
	return FALSE;
}

//---------------------------------------------------------------------------
//  Window Message Handler
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//
//  FUNCTION: OnCreate()
//
//  PURPOSE: WM_CREATE message handler
//
//---------------------------------------------------------------------------
LRESULT OnCreate(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	g_hWndView = DirectoryBrowseTool_CreateWindow(hWnd);
	HMENU h = GetSystemMenu(hWnd,FALSE);
	DestroyMenu( h );
	return 0;
}

//---------------------------------------------------------------------------
//
//  FUNCTION: OnDestroy()
//
//  PURPOSE: WM_DESTROY message handler
//
//---------------------------------------------------------------------------
LRESULT OnDestroy(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PostQuitMessage(0);
	return 0;
}

//---------------------------------------------------------------------------
//
//  FUNCTION: OnSize()
//
//  PURPOSE: WM_SIZE message handler
//
//---------------------------------------------------------------------------
LRESULT OnSize(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int cx = GET_X_LPARAM(lParam);
	int cy = GET_Y_LPARAM(lParam);
	SetWindowPos(g_hWndView,NULL,0,0,cx,cy,SWP_NOZORDER);
	return 0;
}

//---------------------------------------------------------------------------
//
//  FUNCTION: OnActivate()
//
//  PURPOSE: WM_ACTIVATE message handler
//
//---------------------------------------------------------------------------
LRESULT OnActivate(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int nState = (int)wParam;

	if (nState == WA_INACTIVE)
	{
		SaveFocusControl(); // save focus when frame loses activation
	}

	return 0;
}

//---------------------------------------------------------------------------
//
//  FUNCTION: OnSetFocus()
//
//  PURPOSE: WM_SETFOCUS message handler
//
//---------------------------------------------------------------------------
LRESULT OnSetFocus(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!::IsWindow(g_hWndFocus) || !::IsChild(hWnd, g_hWndFocus))
	{
		// invalid or unknown focus window... let windows handle it
		g_hWndFocus = NULL;
		DefWindowProc(hWnd,uMsg,wParam,lParam);
		return 0;
	}

	// otherwise, set focus to the last known focus window
	if( g_hWndFocus != NULL )
	{
		SetFocus(g_hWndFocus);
		g_hWndFocus = NULL;
	}
	return 0;
}

//---------------------------------------------------------------------------
//
//  FUNCTION: OnCommand()
//
//  PURPOSE: WM_COMMAND message handler
//
//---------------------------------------------------------------------------
LRESULT OnCommand(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if( g_hWndView )
	{
		return SendMessage(g_hWndView,uMsg,wParam,lParam);
	}
	return 0;
}
//---------------------------------------------------------------------------
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//---------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_SIZE:
			return OnSize(hWnd,message,wParam,lParam);
		case WM_ACTIVATE:
			return OnActivate(hWnd,message,wParam,lParam);
		case WM_SETFOCUS:
			return OnSetFocus(hWnd,message,wParam,lParam);
		case WM_COMMAND:
			return OnCommand(hWnd,message,wParam,lParam);
		case WM_CREATE:
			return OnCreate(hWnd,message,wParam,lParam);
		case WM_DESTROY:
			return OnDestroy(hWnd,message,wParam,lParam);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

//---------------------------------------------------------------------------
//
//  FUNCTION: RegisterMainWndClass()
//
//  PURPOSE: Registers the window class.
//
//---------------------------------------------------------------------------
ATOM RegisterMainWndClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style          = 0;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = 0;
	wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = pszMainFrameClassName;
	wcex.hIconSm        = 0;

	return RegisterClassExW(&wcex);
}

//---------------------------------------------------------------------------
//
//  FUNCTION: InitInstance(HINSTANCE, int)
//
//  PURPOSE: Initialize instance and creates main window.
//
//  COMMENTS:
//        In this function, we save the instance handle in a global variable
//        and create and display the main program window.
//
//---------------------------------------------------------------------------
HWND InitInstance(HINSTANCE hInstance, int nCmdShow, PCWSTR pszPath)
{
	//
	// Create frame window
	//
	DWORD dwStyle = WS_OVERLAPPED|WS_CAPTION|WS_THICKFRAME|WS_SYSMENU|WS_DLGFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX;
	HWND hWnd = CreateWindowExW(WS_EX_WINDOWEDGE|WS_EX_DLGMODALFRAME,pszMainFrameClassName, Title, dwStyle,
					CW_USEDEFAULT,
					0,
					CW_USEDEFAULT,
					0,
					nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	RECT rcDesktopWorkArea;
	_GetDesktopWorkArea(hWnd,&rcDesktopWorkArea);

	SetWindowPos(hWnd,NULL,0,0,_DPI_Adjust_X(1080),_DPI_Adjust_Y(700),SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOREDRAW);
	_CenterWindow(hWnd,GetDesktopWindow());

	DirectoryBrowseTool_InitData(g_hWndView,pszPath);
	DirectoryBrowseTool_InitLayout(g_hWndView,&rcDesktopWorkArea);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

//---------------------------------------------------------------------------
//
//  FUNCTION: ExitInstance()
//
//  PURPOSE:
//
//---------------------------------------------------------------------------
VOID ExitInstance(HINSTANCE)
{

}

#endif

//---------------------------------------------------------------------------
//  Extern Function
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//
//  FUNCTION: ExtraDialog()
//
//  PURPOSE:
//
//---------------------------------------------------------------------------
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
	)
{
#if _ENABLE_MODAL_DIALOG_SUPPORT
	RegisterMainWndClass(hInstance);

	HWND hwnd = InitInstance (hInstance, SW_SHOW , pszPath);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(hwnd, hAccelTable, &msg))
		{
			if( (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) && (msg.wParam == VK_RETURN) )
			{
				// prevent ring beep when press enter key on tree-view.
				DispatchMessage(&msg);
				continue;
			}
			else
			{
				if( IsDialogMessage(hwnd,&msg) )
					continue;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	ExitInstance(hInstance);

	return S_OK;
#else
	return E_NOTIMPL;
#endif
}
