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
#include "wfsdirbrowsetool.h"
#include "dirinfowindow.h"
#include "dirinfoview.h"
#include "dirtraverser.h"

class CDirectoryBrowserHost : public CBaseWindow
{
public:
	HWND m_hWndTreeBase;

	IViewBaseWindow *m_pFileViewWnd;
	int m_cxSplitPos;

	HWND m_hWndCtrlFocus;

	CDirectoryBrowserHost()
	{
		m_hWnd = NULL;
		m_hWndTreeBase = NULL;
		m_pFileViewWnd = NULL;
		m_cxSplitPos = 320;
		m_hWndCtrlFocus = NULL;
	}

	LRESULT OnCreate(HWND hWnd,UINT,WPARAM,LPARAM)
	{
		m_hWnd = hWnd;

		// Create item tree view window
		DirectoryTraverser_CreateWindow(hWnd,&m_hWndTreeBase,DTS_DIRECTORYNAMES|DTS_FILENAMES);

		// Create information view host frame window
		FileViewBase_CreateObject(GETINSTANCE(m_hWnd),&m_pFileViewWnd);
		m_pFileViewWnd->Create(hWnd);

		//++todo:
	    SHSTOCKICONINFO sii = {0};
		sii.cbSize = sizeof(sii);
		SHGetStockIconInfo(SIID_FOLDEROPEN,SHGSI_ICON|SHGSI_SMALLICON|SHGSI_SHELLICONSIZE,&sii);
		DestroyIcon((HICON)SendMessage(GetParent(m_hWnd),WM_GETICON,ICON_SMALL,0));
		SendMessage(GetParent(m_hWnd),WM_SETICON,ICON_SMALL,(LPARAM)sii.hIcon);
		//--todo:

		m_hWndCtrlFocus = m_hWndTreeBase;

		return 0;
	}

	LRESULT OnDestroy(HWND,UINT,WPARAM,LPARAM)
	{
		//++todo:
		DestroyIcon((HICON)SendMessage(GetParent(m_hWnd),WM_GETICON,ICON_SMALL,0));
		//--todo:

		m_pFileViewWnd->Destroy();
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
			default:
				if( m_hWndCtrlFocus == m_pFileViewWnd->GetHWND() )
					m_pFileViewWnd->InvokeCommand(LOWORD(wParam));
				break;
		}
		return 0;
	}

	LRESULT OnQueryCmdState(HWND,UINT,WPARAM wParam,LPARAM lParam)
	{
		if( m_hWndCtrlFocus == m_pFileViewWnd->GetHWND() )
		{
			ASSERT( lParam != NULL );
			if( lParam )
			{
				if( m_pFileViewWnd->QueryCmdState((UINT)LOWORD(wParam),(UINT*)lParam) == S_OK )
					return TRUE;
			}
		}
		return 0;
	}

	LRESULT OnNotifyMessage(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch( LOWORD(wParam) )
		{
			case CTRL_DIRECTORY_CHANGED:
				OnNotifyChangeDirectory( (SELECT_ITEM*)lParam );
				break;
			case CTRL_PATH_SELECTED:
				OnUpdateInformationView( (SELECT_ITEM*)lParam );
				break;
		}
		return 0;
	}

	LRESULT OnControlMessage(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch( LOWORD(wParam) )
		{
			case CTRL_SET_DIRECTORY:
				return OnSetDirectory(m_hWnd,0,0,lParam);
		}
		return 0;
	}

	LRESULT OnSetDirectory(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		PCWSTR pszDirectoryPath = (PCWSTR)lParam;
		PWSTR pszPath;
		if( IsNtDevicePath(pszDirectoryPath) )
			pszPath = DuplicateString(pszDirectoryPath);
		else
			pszPath = DosPathNameToNtPathName(pszDirectoryPath);

		if( PathFileExists_W(pszPath,NULL) )
		{
			InitData(pszPath);
		}

		FreeMemory(pszPath);

		return 0;
	}

	LRESULT OnInitLayout(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		InitLayout((const RECT *)lParam);
		return 0;
	}

	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(uMsg)
	    {
			case WM_NOTIFY:
				return OnNotify(hWnd,uMsg,wParam,lParam);
		    case WM_SIZE:
				return OnSize(hWnd,uMsg,wParam,lParam);
			case WM_QUERY_CMDSTATE:
				return OnQueryCmdState(hWnd,uMsg,wParam,lParam);
			case WM_NOTIFY_MESSAGE:
				return OnNotifyMessage(hWnd,uMsg,wParam,lParam);
			case WM_CONTROL_MESSAGE:
				return OnControlMessage(hWnd,uMsg,wParam,lParam);
			case WM_COMMAND:
				return OnCommand(hWnd,uMsg,wParam,lParam);
			case WM_SETFOCUS:
				return OnSetFocus(hWnd,uMsg,wParam,lParam);
			case WM_DESTROY:
				return OnDestroy(hWnd,uMsg,wParam,lParam);
		    case WM_CREATE:
				return OnCreate(hWnd,uMsg,wParam,lParam);
		}
		return CBaseWindow::WndProc(hWnd,uMsg,wParam,lParam);
	}

	VOID UpdateLayout(int cx,int cy)
	{
		HDWP hdwp = BeginDeferWindowPos(2);

		if( m_hWndTreeBase )
			DeferWindowPos(hdwp,m_hWndTreeBase,NULL,0,0,m_cxSplitPos,cy,SWP_NOZORDER);

		if( m_pFileViewWnd )
			DeferWindowPos(hdwp,m_pFileViewWnd->GetHWND(),NULL,m_cxSplitPos,0,cx-m_cxSplitPos,cy,SWP_NOZORDER);

		EndDeferWindowPos(hdwp);
	}

	VOID OnUpdateInformationView(SELECT_ITEM* pFile)
	{
		m_pFileViewWnd->SelectPage(pFile);
	}

	VOID OnNotifyChangeDirectory(SELECT_ITEM* pFile)
	{
		PWSTR pszPath;

		if( wcscmp(pFile->pszName,L"..") == 0 )
		{
			pszPath = _MemAllocString(pFile->pszCurDir);
			RemoveFileSpec(pszPath);
		}
		else
		{
			pszPath = _MemAllocString(pFile->pszPath);
		}

		// update MDI child title
		SetWindowText( GetParent(m_hWnd), PathFindFileName(pszPath) ); // todo:

		_MemFree(pszPath);
	}

	VOID InitData(PCWSTR pszDirectoryPath)
	{
		DirectoryTraverser_InitData(m_hWndTreeBase,m_hWnd);
		FillTraversetItems(pszDirectoryPath);
	}

	VOID InitLayout(const RECT *prcDesktopWorkArea)
	{
		DirectoryTraverser_InitLayout(m_hWndTreeBase,NULL);
		m_pFileViewWnd->InitLayout(NULL);
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

	return pView->Create(hWndParent,0,L"DirectoryFilePropertyBrowserWnd",WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,WS_EX_CONTROLPARENT);
}
