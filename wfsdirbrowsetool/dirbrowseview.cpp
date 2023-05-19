//****************************************************************************
//
//  dirbrowseview.cpp
//
//  Implements the file information view base window.
//
//  Auther: YAMASHITA Katsuhiro
//
//  Create: 2023.03.17
//
//****************************************************************************
//
//  Copyright (C) YAMASHITA Katsuhiro. All rights reserved.
//  Licensed under the MIT License.
//
#include "stdafx.h"
#include "resource.h"
#include "dirbrowseview.h"

enum {
	INFOVIEW_ROOT = 0,
	INFOVIEW_FILEINFO,
	MAX_INFO_VIEW_TYPE,
};

class CFileViewBase : 
	public CBaseWindow,
	public IFileViewBaseWindow
{
	CPageWndBase *m_pBase;
	CPageWndBase *m_pViewTable[MAX_INFO_VIEW_TYPE];

public:
	CFileViewBase()
	{
		m_hWnd = NULL;
		m_pBase = NULL;
		memset(m_pViewTable,0,sizeof(m_pViewTable));
	}

	virtual ~CFileViewBase()
	{
	}

	VOID UpdateLayout(int cx=0,int cy=0)
	{
		if( cx == 0 && cy == 0 )
		{
			RECT rc;
			GetClientRect(m_hWnd,&rc); // todo: GetClientSize
			cx = rc.right - rc.left;
			cy = rc.bottom - rc.top;
		}

		if( m_pBase && m_pBase->GetHwnd() )
			SetWindowPos(m_pBase->GetHwnd(),NULL,0,0,cx,cy,SWP_NOZORDER|SWP_NOMOVE|SWP_FRAMECHANGED);
	}

	LRESULT OnCreate(HWND hWnd,UINT,WPARAM,LPARAM lParam)
	{
		SetWindowText(hWnd,L"CFileViewBase");
		return 0;
	}

	LRESULT OnDestroy(HWND hWnd,UINT,WPARAM,LPARAM)
	{
		return 0;
	}

	LRESULT OnSize(HWND,UINT,WPARAM,LPARAM lParam)
	{
		UpdateLayout(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
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
		SendMessage(GetParent(m_hWnd),WM_NOTIFY,0,(LPARAM)pnmhdr);
		return 0;
	}

	template <class T>
	CPageWndBase *GetOrAllocWndObjct(int wndId)
	{
		CPageWndBase *pobj;
		if( m_pViewTable[ wndId ] == NULL )
		{
			pobj = (CPageWndBase*)new T ;
			m_pViewTable[ wndId ] = pobj;
			pobj->Create(m_hWnd,wndId,0,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,WS_EX_CONTROLPARENT);
		}
		else
		{
			pobj = m_pViewTable[ wndId ];
		}
		return pobj;
	}

	INT SelectView(int nView)
	{
		CPageWndBase* pNew = NULL;

		switch( nView )
		{
			case INFOVIEW_ROOT:
			{
				pNew = GetOrAllocWndObjct<CRootView>(INFOVIEW_ROOT);
				break;
			}
			case INFOVIEW_FILEINFO:
			{
				pNew = GetOrAllocWndObjct<CFileInfoView>(INFOVIEW_FILEINFO);
				break;
			}
			default:
				return -1;
		}

		if( m_pBase == pNew )
		{
			return nView;
		}
	
		EnableWindow(pNew->m_hWnd,TRUE);
		ShowWindow(pNew->m_hWnd,SW_SHOWNA);
		if( m_pBase )
		{
			ShowWindow(m_pBase->m_hWnd,SW_HIDE);
			EnableWindow(m_pBase->m_hWnd,FALSE);
		}

		m_pBase = pNew;

		UpdateLayout();

		return nView;
	}

	VOID UpdateData(SELECT_FILE *pFile)
	{
		ASSERT( m_pBase != NULL );

		m_pBase->UpdateData(pFile);
	}

	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(uMsg)
	    {
			case WM_NOTIFY:
				return OnNotify(hWnd,uMsg,wParam,lParam);
			case WM_SIZE:
				return OnSize(hWnd,uMsg,wParam,lParam);
		    case WM_CREATE:
				return OnCreate(hWnd,uMsg,wParam,lParam);
			case WM_DESTROY:
				return OnDestroy(hWnd,uMsg,wParam,lParam);
			case WM_CONTROL_MESSAGE:
				return SendMessage(GetParent(m_hWnd),WM_CONTROL_MESSAGE,wParam,lParam);
		}
		return CBaseWindow::WndProc(hWnd,uMsg,wParam,lParam);
	}

	//
	// IFileInfoBaseWindow
	//
public:
	HWND GetHWND() const
	{
		return m_hWnd;
	}

	HRESULT Create(HWND hWnd,HWND *phWndFileList=NULL)
	{
		HWND hwnd = CBaseWindow::Create(hWnd,0,0,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,WS_EX_CONTROLPARENT);
		if( phWndFileList )
			*phWndFileList = hwnd;
		return S_OK;
	}

	HRESULT Destroy() { return E_NOTIMPL; }
	HRESULT InitData() { return E_NOTIMPL; }
	HRESULT InitLayout(const RECT *prc) { return E_NOTIMPL; }

	HRESULT SelectData(SELECT_FILE *Path) 
	{
		SELECT_FILE *pSel = (SELECT_FILE *)Path;
		ASSERT(pSel != NULL);

		switch( pSel->Type )
		{
			case ITEM_FOLDER_ROOT:
				SelectView( INFOVIEW_ROOT );
				break;
			case ITEM_FOLDER_PATH:
			case ITEM_FOLDER_NAME:
				SelectView( INFOVIEW_FILEINFO );
				break;
		}

		UpdateData(Path);

		UpdateLayout();

		return S_OK;
	}
};

//////////////////////////////////////////////////////////////////////////////

//
//  C style functions
//

HRESULT FileViewBase_CreateObject(HINSTANCE hInstance,IFileViewBaseWindow **pObject)
{
	CFileViewBase *pWnd = new CFileViewBase;

	CFileViewBase::RegisterClass(hInstance);

	*pObject = static_cast<IFileViewBaseWindow *>(pWnd);

	return S_OK;
}
