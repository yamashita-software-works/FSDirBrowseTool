//****************************************************************************
//
//  dirbrowsetraverser.cpp
//
//  Implements the directory traverser window.
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
#include "basewindow.h"
#include "dirbrowsetraverser.h"

#define WC_FOLDERTREEWINDOW L"DirectoryTraverserWnd"

#define ID_TREEVIEW 1

class CDirectoryTraverser : public CBaseWindow
{
public:
	HWND m_hWndTree;
	HWND m_hWndNotice;

	VOID SetNotifyWnd(HWND hwnd) { m_hWndNotice = hwnd; }
	HWND GetNotifyWnd() { return m_hWndNotice; }

	PWSTR m_pszDirectoryPath;

	BOOL m_bPreventAction;

	HIMAGELIST m_himl;
	int m_iImageUpDir;
	BOOL m_bGetUniqueIcon;

	CDirectoryTraverser()
	{
		m_hWndTree = NULL;
		m_hWndNotice = NULL;
		m_pszDirectoryPath = NULL;
		m_bPreventAction = FALSE;
		m_himl = NULL;
		m_iImageUpDir = I_IMAGENONE;
		m_bGetUniqueIcon = FALSE;
	}

	~CDirectoryTraverser()
	{
		_SafeMemFree( m_pszDirectoryPath );
	}

	typedef struct _TREEITEM
	{
		TREEITEMTYPE ItemType;
		PWSTR DisplayName;
		PWSTR FileName;
		PWSTR Path;
		ULONG FileAttributes;
	} TREEITEM, *PTREEITEM;

	HTREEITEM
	AddItem(
		TREEITEMTYPE ItemType,
		PCWSTR pszDisplayName,
		PCWSTR pszFileName,
		PCWSTR pszPath,
		ULONG FileAttributes,
		HTREEITEM hItem = TVI_ROOT,
		HTREEITEM hAfter = TVI_LAST,
		int HasChild = 0
		)
	{
		TVINSERTSTRUCT tvins = {0};

		tvins.hParent = hItem;
		tvins.hInsertAfter = hAfter;
		tvins.itemex.mask = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM|TVIF_CHILDREN;
		tvins.itemex.pszText = (PWSTR)pszDisplayName;
		tvins.itemex.cChildren = HasChild;

		TREEITEM *pItem = new TREEITEM;
		pItem->ItemType = ItemType;
		pItem->DisplayName = _MemAllocString(pszDisplayName);

		if( pszFileName )
			pItem->FileName = _MemAllocString(pszFileName);
		else
			pItem->FileName = NULL;

		if( pszPath )
			pItem->Path = _MemAllocString(pszPath);
		else
			pItem->Path = NULL;

		pItem->FileAttributes = FileAttributes;

		tvins.itemex.lParam = (LPARAM)pItem;

		tvins.itemex.iImage = I_IMAGECALLBACK;
		tvins.itemex.iSelectedImage = tvins.itemex.iImage;

		return TreeView_InsertItem(m_hWndTree,&tvins);
	}

	int GetImageListIndex(PCWSTR pszPath,PCWSTR pszFileName,DWORD dwFileAttributes)
	{
		SHFILEINFO sfi = {0};
		int iImage = I_IMAGENONE;
		if( pszPath )
		{
			if( pszPath[0] == L'\\' && pszPath[1] == L'?' && pszPath[2] == L'?' && pszPath[3] == L'\\' &&
				iswalpha(pszPath[4]) && pszPath[5] == L':' )
			{
				SHGetFileInfo(&pszFileName[4],dwFileAttributes,&sfi,sizeof(sfi),SHGFI_SMALLICON|SHGFI_SYSICONINDEX);
				iImage = sfi.iIcon;
			}
		}
		else
		{
			if( wcscmp(pszFileName,L"..") == 0 )
			{
				iImage = m_iImageUpDir;
			}
			else if( m_bGetUniqueIcon )
			{
				if( m_pszDirectoryPath[0] == L'\\' && m_pszDirectoryPath[1] == L'?' && m_pszDirectoryPath[2] == L'?' && m_pszDirectoryPath[3] == L'\\' &&
					iswalpha(m_pszDirectoryPath[4]) && m_pszDirectoryPath[5] == L':' )
				{
					PWSTR pszFullPath;
					pszFullPath = CombinePath(&m_pszDirectoryPath[4],pszFileName);
					if( pszFullPath )
					{
						if( SHGetFileInfo(pszFullPath,dwFileAttributes,&sfi,sizeof(sfi),SHGFI_SMALLICON|SHGFI_SYSICONINDEX) )
							iImage = sfi.iIcon;
						FreeMemory(pszFullPath);
					}
				}
			}
		}
		if( iImage == I_IMAGENONE )
		{
			if( _IsRootDirectory(pszFileName) )
			{
				SHSTOCKICONINFO sii = {sizeof(sii)};
				SHGetStockIconInfo(SIID_DRIVEFIXED,SHGSI_SYSICONINDEX|SHGSI_SMALLICON|SHGSI_SHELLICONSIZE|SHGSI_LINKOVERLAY,&sii);
				iImage = sii.iSysImageIndex;
			}
			else
			{
				SHGetFileInfo(PathFindFileName(pszFileName),dwFileAttributes,&sfi,sizeof(sfi),SHGFI_SMALLICON|SHGFI_SYSICONINDEX|SHGFI_USEFILEATTRIBUTES);
				iImage = sfi.iIcon;
			}
		}

#if 0
		{
			WCHAR sz[64];
			StringCchPrintf(sz,ARRAYSIZE(sz),L"Image Index=%d\n",iImage);
			_TRACEW(sz);
		}
#endif

		return iImage;
	}

	HTREEITEM InsertBlank(HTREEITEM hParent,HTREEITEM hInsert)
	{
		TREEITEM *pItem = new TREEITEM;
		memset(pItem,0,sizeof(TREEITEM));

		pItem->ItemType = ITEM_BLANK;

		TVINSERTSTRUCT tvi = {0};
		tvi.hInsertAfter = TVI_LAST;
		tvi.hParent =  TVI_ROOT;
		tvi.itemex.mask = TVIF_TEXT|TVIF_IMAGE|TVIF_PARAM|TVIF_INTEGRAL;
		tvi.itemex.pszText = L"";
		tvi.itemex.lParam = (LPARAM)pItem;
		tvi.itemex.iImage = tvi.itemex.iExpandedImage = I_IMAGENONE;
		tvi.itemex.iIntegral = 1;  // blank line height
		tvi.itemex.uStateEx = 0;
		HTREEITEM h = TreeView_InsertItem(m_hWndTree, &tvi);
		TVITEMEX tviex = {0};
		tviex.mask = TVIF_STATEEX;
		tviex.hItem = h;
		tviex.uStateEx = TVIS_EX_DISABLED;
		TreeView_SetItem( m_hWndTree, &tviex );
		return h;
	}

	DWORD UpdateView( PCWSTR pszDirectoryPath )
	{
		SetRedraw(m_hWndTree,FALSE);
		m_bPreventAction = TRUE;

		DWORD dwError;

		PtrArray<CFileItem> pa;
		dwError = EnumFileItems(pszDirectoryPath,pa);

		if( dwError == ERROR_SUCCESS )
		{
			HTREEITEM hSelect;

			_SafeMemFree( m_pszDirectoryPath );

			// Prevent repeated notification message of selection item at next DeleteAllItems.
			TreeView_Select(m_hWndTree,NULL,TVGN_CARET);
			TreeView_DeleteAllItems( m_hWndTree );

			//
			// Allocate new 'current' directory path string.
			//
			m_pszDirectoryPath = _MemAllocString( pszDirectoryPath );

			InsertBlank(TVI_ROOT,TVI_LAST);

			//
			// Insert 'current' directory path.
			//
			ULONG FileAttributes = 0;
			PathFileExists_W(pszDirectoryPath,&FileAttributes);
//			hSelect = AddItem(ITEM_FOLDER_ROOT,pszDirectoryPath,pszDirectoryPath,pszDirectoryPath,FileAttributes,TVI_ROOT,TVI_SORT,0);
			hSelect = AddItem(ITEM_FOLDER_ROOT,PathFindFileName(pszDirectoryPath),pszDirectoryPath,pszDirectoryPath,FileAttributes,TVI_ROOT,TVI_SORT,0);

			InsertBlank(TVI_ROOT,TVI_LAST);

			//
			// Fill tree items directory contents.
			//
			FillTreeItems(pa);

			FreeFileItems(pa);

			TreeView_Select(m_hWndTree,hSelect,TVGN_CARET);
		}

		m_bPreventAction = FALSE;
		SetRedraw(m_hWndTree,TRUE);

		return dwError;
	}

	DWORD EnumFileItems(PCWSTR pszDirectoryPath,PtrArray<CFileItem>& pa)
	{
		CWaitCursor wait;

		pa.Create( 4096 );

		return WinEnumFiles(pszDirectoryPath,NULL,0,&EnumProc,&pa);
	}

	static HRESULT CALLBACK EnumProc(ULONG CallType,PVOID FileInfo,PFSDIRENUMCALLBACKINFO DirEnumInfo,PVOID Context)
	{
		FILE_ID_BOTH_DIR_INFORMATION *pfibdi = (FILE_ID_BOTH_DIR_INFORMATION *)FileInfo;

		WCHAR filename[MAX_PATH];
		memcpy_s(filename,sizeof(filename),pfibdi->FileName,pfibdi->FileNameLength);
		filename[pfibdi->FileNameLength/sizeof(WCHAR)] = UNICODE_NULL;

		CFileItem *pFI = new CFileItem(NULL,filename);

		pFI->FileAttributes = pfibdi->FileAttributes;
		pFI->LastWriteTime  = pfibdi->LastWriteTime;
		pFI->CreationTime   = pfibdi->CreationTime;
		pFI->LastAccessTime = pfibdi->LastAccessTime;
		pFI->ChangeTime     = pfibdi->ChangeTime;
		pFI->EndOfFile      = pfibdi->EndOfFile;
		pFI->AllocationSize = pfibdi->AllocationSize;
		pFI->EaSize         = pfibdi->EaSize;
	    pFI->FileId         = pfibdi->FileId;
		pFI->FileNameLength = pfibdi->FileNameLength;
		pFI->FileIndex      = pfibdi->FileIndex;

		memcpy(pFI->ShortName,pfibdi->ShortName,pfibdi->ShortNameLength);
		pFI->ShortName[pfibdi->ShortNameLength/sizeof(WCHAR)] = UNICODE_NULL;

		PtrArray<CFileItem> *pa = (PtrArray<CFileItem> *)Context;
		pa->Add( pFI );	

		return S_OK;
	}

	VOID FreeFileItems(PtrArray<CFileItem>& pa)
	{
		int cFiles = 0;
		int i,cItems = pa.GetCount();

		for(i = 0; i < cItems; i++)
		{
			delete pa.Get(i);
		}		

		pa.DeleteAll();		
	}

	VOID FillTreeItems(PtrArray<CFileItem>& pa)
	{
		int cFiles = 0;
		int i,cItems = pa.GetCount();

		for(i = 0; i < cItems; i++)
		{
			CFileItem *pItem = pa.Get(i);

			if( wcscmp(pItem->hdr.FileName,L".") == 0 ) // exclude curdir
				continue;

			if( pItem->FileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				AddItem(ITEM_FOLDER_PATH,pItem->hdr.FileName,pItem->hdr.FileName,NULL,pItem->FileAttributes,TVI_ROOT,TVI_LAST,0);
			else
				cFiles++;
		}

		if( cFiles > 0 )
		{
			InsertBlank(TVI_ROOT,TVI_LAST);

			for(i = 0; i < cItems; i++)
			{
				CFileItem *pItem = pa.Get(i);

				if( (pItem->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 )
					AddItem(ITEM_FOLDER_PATH,pItem->hdr.FileName,pItem->hdr.FileName,NULL,pItem->FileAttributes,TVI_ROOT,TVI_LAST,0);
			}
		}
	}

	BOOL SelectFolder(PCWSTR pszFolderName)
	{
		HTREEITEM hItem;

		hItem = FindItem(pszFolderName);

		if( hItem )
		{
			if( wcscmp(pszFolderName,L"..") == 0 )
				MoveToDirectory(m_hWndTree,hItem);
			else
				TreeView_Select(m_hWndTree,hItem,TVGN_CARET);
		}
		return TRUE;
	}

	HTREEITEM FindItem(PCWSTR pszName)
	{
		WCHAR szItem[MAX_PATH];

		HTREEITEM hItem = TreeView_GetRoot(m_hWndTree);
		while( hItem )
		{
			if( TreeView_GetItemText(m_hWndTree,hItem,szItem,ARRAYSIZE(szItem)) )
			{
				if( wcsicmp(szItem,pszName) == 0 )
				{
					break;
				}
			}
			hItem = TreeView_GetNextSibling(m_hWndTree,hItem);
		}
		return hItem;
	}

	VOID NotifyHost(USHORT code,PCWSTR pszLocation,PCWSTR pszText,PCWSTR pszFullPath,TREEITEM *pItem)
	{
		WPARAM wParam;
		wParam = MAKEWPARAM(code,0); // hiword:reserved

		SELECT_FILE path;
		path.pszPath = (PWSTR)pszFullPath;
		path.pszName = (PWSTR)pszText; 
		path.pszLocation = (PWSTR)pszLocation;
		path.Type = pItem->ItemType;

		SendMessage(m_hWndNotice,WM_CONTROL_MESSAGE,wParam,(LPARAM)&path);
	}

	HWND CreateFolderTreeView(HWND hwndParent)
	{ 
		HWND hwndTreeView;

		hwndTreeView = CreateWindowEx(0,
							WC_TREEVIEW,
							TEXT("DirectoryTraverser"),
							WS_VISIBLE|WS_CHILD|WS_TABSTOP|TVS_DISABLEDRAGDROP|//TVS_NOHSCROLL|
							TVS_HASBUTTONS|TVS_LINESATROOT|TVS_INFOTIP|TVS_FULLROWSELECT|TVS_NONEVENHEIGHT|TVS_SHOWSELALWAYS, 
							0, 0, 0, 0,
							hwndParent, 
							(HMENU)ID_TREEVIEW, 
							GETINSTANCE(hwndParent),
							NULL); 

		TreeView_SetExtendedStyle(hwndTreeView,
					TVS_EX_DOUBLEBUFFER,
					TVS_EX_DOUBLEBUFFER
					);

		_EnableVisualThemeStyle(hwndTreeView);


		HIMAGELIST himl;
		Shell_GetImageLists(NULL,&himl);
		m_himl = himl;

		TreeView_SetImageList(hwndTreeView,himl,TVSIL_NORMAL);

		int cx,cy;
		ImageList_GetIconSize(himl,&cx,&cy);

		HICON hIcon = (HICON)LoadImage(GetModuleHandle(L"shell32"), MAKEINTRESOURCE(46), IMAGE_ICON, cx, cy, 0);
		m_iImageUpDir = ImageList_AddIcon(m_himl,hIcon);

		TreeView_SetIndent(hwndTreeView,cx/2);
		TreeView_SetItemHeight(hwndTreeView, cy + 8);

		return hwndTreeView;
	}

	/////////////////////////////////////////////////////////////////////////////

	//
	// Message Handlers
	//
	LRESULT OnCreate(HWND hWnd,UINT,WPARAM,LPARAM)
	{
		m_hWndTree = CreateFolderTreeView(hWnd);

		SendMessage(m_hWndTree,TVM_SETBKCOLOR,0,(LPARAM)RGB(250,250,250));

		return 0;
	}

	LRESULT OnDestroy(HWND,UINT,WPARAM,LPARAM)
	{
		return 0;
	}

	LRESULT OnSetFocus(HWND,UINT,WPARAM,LPARAM)
	{
		SetFocus(m_hWndTree);
		return 0;
	}

	LRESULT OnSize(HWND,UINT,WPARAM,LPARAM lParam)
	{
		if( m_hWndTree )
		{
			int cx = GET_X_LPARAM(lParam);
			int cy = GET_Y_LPARAM(lParam);
			SetWindowPos(m_hWndTree,NULL,0,0,cx,cy,SWP_NOZORDER|SWP_NOMOVE);
		}
		return 0;
	}

	LRESULT OnDeleteItem(LPNMTREEVIEW pnmtv)
	{
		TREEITEM *pItem = (TREEITEM *)pnmtv->itemOld.lParam;
		_SafeMemFree(pItem->DisplayName);
		_SafeMemFree(pItem->FileName);
		_SafeMemFree(pItem->Path);
		delete pItem;
		return 0;
	}

	LRESULT OnGetDispInfo(LPNMTVDISPINFO ptvdi)
	{
		TREEITEM *pItem = (TREEITEM *)ptvdi->item.lParam;

		if( ptvdi->item.mask & TVIF_IMAGE )
		{
			ptvdi->item.iImage = 
			ptvdi->item.iSelectedImage = GetImageListIndex(pItem->Path,pItem->FileName,pItem->FileAttributes);
			ptvdi->item.mask |= TVIF_DI_SETITEM;
		}

		return 0;
	}

	LRESULT OnSelChanged(LPNMTREEVIEW pnmtv)
	{
		if( pnmtv->itemNew.state & TVIS_SELECTED )
		{
			TREEITEM *pItem = (TREEITEM *)pnmtv->itemNew.lParam;

			if( pItem )
			{
				if( pItem->Path != NULL )
				{
					NotifyHost(CODE_SELECT_PATH,pItem->Path,pItem->FileName,pItem->Path,pItem);
				}
				else
				{
					if( m_pszDirectoryPath )
					{
						PWSTR pszFullPath;
						pszFullPath = CombinePath(m_pszDirectoryPath,pItem->FileName);
						{
							NotifyHost(CODE_SELECT_PATH,m_pszDirectoryPath,pItem->FileName,pszFullPath,pItem);
							FreeMemory(pszFullPath);
						}
					}
				}
			}
		}
		return 0;
	}

	LRESULT OnKeyDown(LPNMTVKEYDOWN pnmhdr)
	{
		if( pnmhdr->wVKey == VK_RETURN )
			OnNmDblClk((LPNMHDR)pnmhdr);
		return 0;
	}

	LRESULT OnNmDblClk(LPNMHDR pnmhdr)
	{
		if( m_bPreventAction )
			return 0;

		HTREEITEM hItem = TreeView_GetSelection(pnmhdr->hwndFrom);
		
		MoveToDirectory(pnmhdr->hwndFrom,hItem);

		return 0;
	}

	LRESULT OnNmSetFocus(NMHDR *pnmhdr)
	{
		SendMessage(GetParent(m_hWnd),WM_NOTIFY,0,(LPARAM)pnmhdr);
		return 0;
	}

	VOID MoveToDirectory(HWND hwndTreeView,HTREEITEM hItem)
	{
		if( hItem != NULL )
		{
			TREEITEM *pItem = (TREEITEM *)TreeView_GetItemData(hwndTreeView,hItem);

			if( pItem->ItemType == ITEM_FOLDER_ROOT )
			{
				; // todo: show folder tree dialog?
			}
			else
			{
				if(pItem->FileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				{
					PWSTR pszFullPath;
					if( wcscmp(pItem->FileName,L"..") == 0 )
						pszFullPath = DuplicateString(m_pszDirectoryPath);
					else
						pszFullPath = CombinePath(m_pszDirectoryPath,pItem->FileName);

					if( pszFullPath )
					{
						NotifyHost(CODE_CHANGE_DIRECTORY,m_pszDirectoryPath,pItem->FileName,pszFullPath,pItem);
						FreeMemory(pszFullPath);
					}
				}
			}
		}
	}

	LRESULT OnCustomDraw(LPNMLVCUSTOMDRAW pcd)
	{
// toto:
//		if( IsXpThemeEnabled() )
//			return CDRF_DODEFAULT;

		//
		// classic mode only
		//
		if( pcd->nmcd.dwDrawStage == CDDS_PREPAINT )
		{
			return CDRF_NOTIFYITEMDRAW;
		}

		TREEITEM *pItem = (TREEITEM *)pcd->nmcd.lItemlParam;

		if( pItem->ItemType != ITEM_BLANK )
			return CDRF_DODEFAULT;

		if( pcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
		{
			pcd->clrTextBk = TreeView_GetBkColor(m_hWndTree);

			return CDRF_NEWFONT;
		}

		return CDRF_DODEFAULT;
	}

	LRESULT OnSetCursor(LPNMMOUSE lpnmm)
	{
		// lpnmm->pt��"A POINT structure that contains the client coordinates of 
		// the mouse when the click occurred."�Ɛ�������Ă��邪�APOINT�Ƃ͎v���Ȃ�
		// �s���Ȓl�������Ă���B
		// ���ׁ̈AGetCursorPos�Ŏ擾���������Ă���B
		GetCursorPos(&lpnmm->pt);

		TVHITTESTINFO tvht = {0};
		tvht.pt    = lpnmm->pt;
		ScreenToClient(m_hWndTree,&tvht.pt);
		tvht.flags = TVHT_ONITEM|TVHT_ONITEMSTATEICON|TVHT_ONITEMRIGHT;
		tvht.hItem = NULL;
		HTREEITEM hItem = TreeView_HitTest(m_hWndTree,&tvht);
		TREEITEM *pItem = (TREEITEM *)TreeView_GetItemData(m_hWndTree,hItem);

		if( pItem && pItem->ItemType == ITEM_BLANK )
		{
			SetCursor( LoadCursor(NULL,IDC_ARROW) );
			return 1;
		}
		return 0;
	}

	LRESULT OnSelChanging(NMTREEVIEW *pnmtv)
	{
		TVITEMEX tiex={0};
		tiex.mask = TVIF_STATEEX;
		tiex.hItem = pnmtv->itemNew.hItem;
		TreeView_GetItem(m_hWndTree,&tiex);
		if( tiex.uStateEx & TVIS_EX_DISABLED )
		{
			if( pnmtv->action == 2 )
			{
				BOOL bPageKey = FALSE;

				BYTE keystate[256] = {0};
				GetKeyboardState(keystate);

				SetRedraw(m_hWndTree,FALSE);

				if( (keystate[VK_HOME] & 0x80) || (keystate[VK_PRIOR] & 0x80) || (keystate[VK_NEXT] & 0x80) )
					bPageKey = TRUE;

				HTREEITEM hNextItem = NULL;

				if( bPageKey )
				{
					TreeView_Select(m_hWndTree,(hNextItem = TreeView_GetNextVisible(m_hWndTree,pnmtv->itemNew.hItem)),TVGN_CARET);
				}
				else
				{
					if( GetKeyState(VK_DOWN) < 0 )
						hNextItem = TreeView_GetNextVisible(m_hWndTree,pnmtv->itemNew.hItem);
					else
						hNextItem = TreeView_GetPrevVisible(m_hWndTree,pnmtv->itemNew.hItem);

					if( hNextItem )
						TreeView_Select(m_hWndTree,hNextItem,TVGN_CARET);
				}

				if( hNextItem )
				{
					TREEITEM *pItem = (TREEITEM *)TreeView_GetItemData(m_hWndTree,hNextItem);
					if( pItem->ItemType == ITEM_FOLDER_ROOT )
					{
						TreeView_EnsureVisible(m_hWndTree, TreeView_GetPrevSibling(m_hWndTree,hNextItem) );
					}
					else
					{
						if( TreeView_GetLastVisible(m_hWndTree) == hNextItem )
						{
							TreeView_EnsureVisible(m_hWndTree, hNextItem );
						}
					}
				}
				SetRedraw(m_hWndTree,TRUE);
			}
			return TRUE;
		}
		return 0;
	}

	LRESULT OnNotify(HWND,UINT,WPARAM,LPARAM lParam)
	{
		NMHDR *pnmhdr = (NMHDR *)lParam;

		switch( pnmhdr->code )
		{
			case NM_CUSTOMDRAW:
				return OnCustomDraw((LPNMLVCUSTOMDRAW)pnmhdr);
			case NM_SETCURSOR:
				return OnSetCursor((LPNMMOUSE)pnmhdr);
			case TVN_GETDISPINFO:
				return OnGetDispInfo((LPNMTVDISPINFO)pnmhdr);
			case TVN_DELETEITEM:
				return OnDeleteItem((LPNMTREEVIEW)pnmhdr);
			case TVN_SELCHANGING:
				return OnSelChanging((LPNMTREEVIEW)pnmhdr);
			case TVN_SELCHANGED:
				return OnSelChanged((LPNMTREEVIEW)pnmhdr);
			case TVN_KEYDOWN:
				return OnKeyDown((LPNMTVKEYDOWN)pnmhdr);
			case NM_DBLCLK:
				return OnNmDblClk((LPNMHDR)pnmhdr);
			case NM_SETFOCUS:
				return OnNmSetFocus((LPNMHDR)pnmhdr);
		}
		
		return 0;
	}

	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(uMsg)
	    {
	        case WM_SIZE:
				return OnSize(hWnd,uMsg,wParam,lParam);
	        case WM_NOTIFY:
				return OnNotify(hWnd,uMsg,wParam,lParam);
		    case WM_CREATE:
				return OnCreate(hWnd,uMsg,wParam,lParam);
	        case WM_DESTROY:
				return OnDestroy(hWnd,uMsg,wParam,lParam);
			case WM_SETFOCUS:
				return OnSetFocus(hWnd,uMsg,wParam,lParam);
			default:
				return DefWindowProc(hWnd,uMsg,wParam,lParam);
		}
		return 0;
	}
};

/////////////////////////////////////////////////////////////////////////////

//
//  CreateFolderTreeWindow()
//
//  PURPOSE : 
//
HRESULT DirectoryTraverser_CreateWindow(HWND hWnd,HWND *phWnd)
{
    HINSTANCE hInstance;
    hInstance = GETINSTANCE(hWnd);

	CDirectoryTraverser *pWnd = new CDirectoryTraverser;

	CDirectoryTraverser::RegisterClass(hInstance);

	HWND hwnd = pWnd->Create(hWnd,0,NULL,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,WS_EX_CONTROLPARENT);

	*phWnd = hwnd;

	return S_OK;
}

HRESULT DirectoryTraverser_InitData(HWND hWnd,HWND hWndNotify)
{
	CDirectoryTraverser *pObj = (CDirectoryTraverser *)GetBaseWindowObject(hWnd);
	if( pObj )
	{
		pObj->SetNotifyWnd(hWndNotify);
	}
	return S_OK;
}

HRESULT DirectoryTraverser_InitLayout(HWND hWnd,RECT *prc)
{
	return S_OK;
}

HRESULT DirectoryTraverser_SetNotifyWnd(HWND hWnd,HWND hWndNotify)
{
	CDirectoryTraverser *pObj = (CDirectoryTraverser *)GetBaseWindowObject(hWnd);
	if( pObj )
	{
		pObj->SetNotifyWnd(hWndNotify);
	}
	return S_OK;
}

HRESULT DirectoryTraverser_GetNotifyWnd(HWND hWnd,HWND *phWnd)
{
	CDirectoryTraverser *pObj = (CDirectoryTraverser *)GetBaseWindowObject(hWnd);
	if( pObj )
	{
		*phWnd = pObj->GetNotifyWnd();
	}
	return S_OK;
}

HRESULT DirectoryTraverser_FillItems(HWND hWnd,PCWSTR pszDirectoryPath)
{
	CDirectoryTraverser *pObj = (CDirectoryTraverser *)GetBaseWindowObject(hWnd);
	if( pObj )
	{
		return HRESULT_FROM_WIN32( pObj->UpdateView(pszDirectoryPath) );
	}
	return E_FAIL;
}

HRESULT DirectoryTraverser_SelectFolder(HWND hWnd, PCWSTR pszFolderName, UINT Reserved )
{
	CDirectoryTraverser *pObj = (CDirectoryTraverser *)GetBaseWindowObject(hWnd);
	if( pObj )
	{
		pObj->SelectFolder(pszFolderName);
	}
	return E_FAIL;
}
