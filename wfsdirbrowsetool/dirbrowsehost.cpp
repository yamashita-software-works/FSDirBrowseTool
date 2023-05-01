//****************************************************************************
//
//  dirbrowsehost.cpp
//
//  Implements the directory browser view host window.
//
//  Auther: YAMASHITA Katsuhiro
//
//  Create: 2023.03.31
//
//****************************************************************************
//
//  Copyright (C) YAMASHITA Katsuhiro. All rights reserved.
//  Licensed under the MIT License.
//
#include "stdafx.h"
#include "resource.h"
#include "dirbrowsehost.h"
#include "dirbrowsetraverser.h"
#include "dirbrowseview.h"

class CDirectoryBrowserHost : public CBaseWindow
{
public:
	HWND m_hWndTreeBase;

	IFileInfoBaseWindow *m_pFileInfoWnd;
	int m_cxSplitPos;

	HWND m_hWndCtrlFocus;

	CDirectoryBrowserHost()
	{
		m_hWnd = NULL;
		m_hWndTreeBase = NULL;
		m_pFileInfoWnd = NULL;
		m_cxSplitPos = 320;
		m_hWndCtrlFocus = NULL;
	}

	LRESULT OnCreate(HWND hWnd,UINT,WPARAM,LPARAM)
	{
		m_hWnd = hWnd;

		// Create item tree view window
		DirectoryTraverser_CreateWindow(hWnd,&m_hWndTreeBase);

		// Create information view host frame window
		FileInfoBase_CreateObject(GETINSTANCE(m_hWnd),&m_pFileInfoWnd);
		m_pFileInfoWnd->Create(hWnd);

#if _ENABLE_SUBPANE
		m_pSubPane = new CSubPane;
		m_pSubPane->Create(m_hWnd,0,0,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,WS_EX_CONTROLPARENT);
#endif
		return 0;
	}

	LRESULT OnDestroy(HWND,UINT,WPARAM,LPARAM)
	{
		m_pFileInfoWnd->Destroy();
		return 0;
	}

	LRESULT OnSize(HWND,UINT,WPARAM,LPARAM lParam)
	{
		int cx = GET_X_LPARAM(lParam);
		int cy = GET_Y_LPARAM(lParam);
		UpdateLayout(cx,cy);
		return 0;
	}

	LRESULT OnSetFocus(HWND,UINT,WPARAM,LPARAM lParam)
	{
		if( m_hWndCtrlFocus == NULL )
			m_hWndCtrlFocus = m_hWndTreeBase;

		SetFocus(m_hWndCtrlFocus);

		return 0;
	}

	LRESULT OnNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		NMHDR *pnmhdr = (NMHDR *)lParam;
		switch( pnmhdr->code )
		{
			case NM_SETFOCUS:
				return OnNmSetFocus(pnmhdr);
		}
		return 0;
	}

	LRESULT OnNmSetFocus(NMHDR *pnmhdr)
	{
		m_hWndCtrlFocus = pnmhdr->hwndFrom;
		return 0;
	}

	LRESULT OnCommand(HWND,UINT,WPARAM wParam,LPARAM)
	{
		switch( LOWORD(wParam) )
		{
			case ID_UP_DIR:
				DirectoryTraverser_SelectFolder(m_hWndTreeBase,L"..",0);
				break;
		}
		return 0;
	}

	LRESULT OnControlMessage(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch( LOWORD(wParam) )
		{
			case CODE_SELECT_PATH:
				OnUpdateInformationView( (SELECT_FILE*)lParam );
				break;
			case CODE_CHANGE_DIRECTORY:
				OnChangeDirectory( (SELECT_FILE*)lParam );
				break;
			case CODE_ASYNC_UPDATE_PATH:
				OnAsyncUpdatePath( (PWSTR)lParam );
				break;
		}
		return 0;
	}

	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(uMsg)
	    {
		    case WM_CREATE:
				return OnCreate(hWnd,uMsg,wParam,lParam);
	        case WM_COMMAND:
				return OnCommand(hWnd,uMsg,wParam,lParam);
			case WM_DESTROY:
				return OnDestroy(hWnd,uMsg,wParam,lParam);
		    case WM_SIZE:
				return OnSize(hWnd,uMsg,wParam,lParam);
			case WM_CONTROL_MESSAGE:
				return OnControlMessage(hWnd,uMsg,wParam,lParam);
			case WM_SETFOCUS:
				return OnSetFocus(hWnd,uMsg,wParam,lParam);
			case WM_NOTIFY:
				return OnNotify(hWnd,uMsg,wParam,lParam);
		}
		return CBaseWindow::WndProc(hWnd,uMsg,wParam,lParam);
	}

	VOID UpdateLayout(int cx,int cy)
	{
		HDWP hdwp = BeginDeferWindowPos(2);

		if( m_hWndTreeBase )
			DeferWindowPos(hdwp,m_hWndTreeBase,NULL,0,0,m_cxSplitPos,cy,SWP_NOZORDER);

		if( m_pFileInfoWnd )
			DeferWindowPos(hdwp,m_pFileInfoWnd->GetHWND(),NULL,m_cxSplitPos,0,cx-m_cxSplitPos,cy,SWP_NOZORDER);

		EndDeferWindowPos(hdwp);
	}

	VOID OnUpdateInformationView(SELECT_FILE* pFile)
	{
		m_pFileInfoWnd->SelectData(pFile);
	}

	VOID OnChangeDirectory(SELECT_FILE* pFile)
	{
		PWSTR pszPath;

		if( wcscmp(pFile->pszName,L"..") == 0 )
		{
			pszPath = _MemAllocString(pFile->pszLocation);
			RemoveFileSpec(pszPath);
		}
		else
			pszPath = _MemAllocString(pFile->pszPath);

		PostMessage(m_hWnd,WM_CONTROL_MESSAGE,CODE_ASYNC_UPDATE_PATH,(LPARAM)pszPath);
	}

	VOID OnAsyncUpdatePath( PWSTR pszPath )
	{
		if( pszPath )
		{
			FillTraversetItems(pszPath);
			_MemFree(pszPath); // free queued path buffer
		}
	}

	VOID InitData(PCWSTR pszDirectoryPath)
	{
		DirectoryTraverser_InitData(m_hWndTreeBase,m_hWnd);
		FillTraversetItems(pszDirectoryPath);
	}

	VOID InitLayout(const RECT *prcDesktopWorkArea)
	{
		DirectoryTraverser_InitLayout(m_hWndTreeBase,NULL);
		m_pFileInfoWnd->InitLayout(NULL);
	}

	VOID FillTraversetItems(PCWSTR pszDirectoryPath)
	{
		if( DirectoryTraverser_FillItems(m_hWndTreeBase,pszDirectoryPath) == S_OK )
		{
			// update MDI child title
			SetWindowText( GetParent(m_hWnd), PathFindFileName(pszDirectoryPath) ); // todo:
		}
	}
};

//////////////////////////////////////////////////////////////////////////////

HWND DirectoryBrowseTool_CreateWindow(HWND hWndParent)
{
	CDirectoryBrowserHost::RegisterClass(GETINSTANCE(hWndParent));

	CDirectoryBrowserHost *pView = new CDirectoryBrowserHost;

	return pView->Create(hWndParent,0,L"DirectoryBrowseToolHostWnd",WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,WS_EX_CONTROLPARENT);
}

VOID DirectoryBrowseTool_InitData(HWND hWndViewHost,PCWSTR pszDirectoryPath)
{
	CDirectoryBrowserHost *pWnd = (CDirectoryBrowserHost *)GetBaseWindowObject(hWndViewHost);
	if( pWnd )
	{
		PWSTR pszPath;
		if( IsNtDevicePath(pszDirectoryPath) )
			pszPath = DuplicateString(pszDirectoryPath);
		else
			pszPath = DosPathNameToNtPathName(pszDirectoryPath);

		if( PathFileExists_W(pszPath,NULL) )
		{
			pWnd->InitData(pszPath);
		}

		FreeMemory(pszPath);
	}
}

VOID DirectoryBrowseTool_InitLayout(HWND hWndViewHost,const RECT *prcDesktopWorkArea)
{
	CDirectoryBrowserHost *pWnd = (CDirectoryBrowserHost *)GetBaseWindowObject(hWndViewHost);
	if( pWnd )
		pWnd->InitLayout(prcDesktopWorkArea);
}
