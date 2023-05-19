#pragma once
//
//  page_fileinfolist.h
//
//  Create: 2022-04-04,2022-05-03
//
#include "stdafx.h"
#include "dirbrowseview.h"
#include "fileinfo.h"
#include "ntreparsepointtag.h"

//
// GetDisp Hanlder
//
template< class T > struct _ITEMDISP_HANDLER_DEF
{
	UINT tid;
	LRESULT (T::*pfn)(UINT,NMLVDISPINFO*);
};
#define ITEMDISP_HANDLER_MAP_DEF(tid,pfn) { tid, pfn }

//
// Group Id
//
enum {
	ID_GROUP_NAME=1,
	ID_GROUP_DATETIME,
	ID_GROUP_SIZE,
	ID_GROUP_ATTRIBUTES,
	ID_GROUP_ALTSTREAMS,
	ID_GROUP_EA,
	ID_GROUP_REPARSEPOINT,
	ID_GROUP_OBJECTID,
	ID_GROUP_OTHERS,
};

class CFileInfoView : public CPageWndBase
{
	HWND m_hWndList;

	FILE_INFORMATION_STRUCT *m_pFI;

	_ITEMDISP_HANDLER_DEF<CFileInfoView> *m_pch;

public:
	CFileInfoView()
	{
		m_hWndList = NULL;
		m_pFI = NULL;
		m_pch = NULL;
	}

	~CFileInfoView()
	{
		if( m_pFI )
			NTFile_FreeFileInformation( m_pFI );
		if( m_pch )
			delete m_pch;
	}

	LRESULT OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		m_hWndList = CreateWindow(WC_LISTVIEW, 
                              L"", 
                              WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_TABSTOP | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | LVS_SINGLESEL, 
                              0,0,0,0,
                              hWnd,
                              (HMENU)0,
                              GetModuleHandle(NULL), 
                              NULL); 

		InitList(m_hWndList);
		InitGroup();

		return 0;
	}

	LRESULT OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		int cx = GET_X_LPARAM(lParam);
		int cy = GET_Y_LPARAM(lParam);
		UpdateLayout(cx,cy);
		return 0;
	}

	LRESULT OnNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		NMHDR *pnmhdr = (NMHDR *)lParam;

		switch( pnmhdr->code )
		{
			case LVN_GETDISPINFO:
				return OnGetDispInfo(pnmhdr);
			case LVN_ITEMCHANGED:
				return OnItemChanged(pnmhdr);
			case LVN_DELETEITEM:
				return OnDeleteItem(pnmhdr);
			case LVN_ITEMACTIVATE:
				return OnItemActivate(pnmhdr);
			case LVN_KEYDOWN:
				return OnKeyDown(pnmhdr);
			case NM_SETFOCUS:
				return OnNmSetFocus(pnmhdr);
		}
		return 0;
	}

	LRESULT OnDeleteItem(NMHDR *pnmhdr)
	{
		NMLISTVIEW *pnmlv = (NMLISTVIEW *)pnmhdr;
		return 0;
	}

	LRESULT OnKeyDown(NMHDR *pnmhdr)
	{
		NMLVKEYDOWN *pnmkd = (NMLVKEYDOWN *)pnmhdr;

		if( pnmkd->wVKey == VK_SPACE || pnmkd->wVKey == VK_RETURN)
		{
			int iGroup = (int)ListView_GetFocusedGroup(pnmkd->hdr.hwndFrom);
			if( iGroup != -1 )
			{
				LVGROUP lvg = {0};
				lvg.cbSize = sizeof(lvg);
				lvg.mask = LVGF_GROUPID|LVGF_STATE;
				ListView_GetGroupInfoByIndex(pnmkd->hdr.hwndFrom,iGroup,&lvg);

				lvg.state = ListView_GetGroupState(pnmkd->hdr.hwndFrom,lvg.iGroupId,LVGS_COLLAPSED|LVGS_COLLAPSIBLE);

				lvg.mask = LVGF_STATE;
				lvg.state ^= LVGS_COLLAPSED;
				lvg.stateMask = LVGS_COLLAPSED;
				ListView_SetGroupInfo(pnmkd->hdr.hwndFrom,lvg.iGroupId,&lvg);
			}
		}
		
		return 0;
	}

	LRESULT OnItemActivate(NMHDR *pnmhdr)
	{
		NMITEMACTIVATE *pnmia = (NMITEMACTIVATE *)pnmhdr;
		return 0;
	}

	LRESULT OnNmSetFocus(NMHDR *pnmhdr)
	{
		SendMessage(GetParent(m_hWnd),WM_NOTIFY,0,(LPARAM)pnmhdr);
		return 0;
	}

	LRESULT OnDisp_Name(UINT,NMLVDISPINFO *pnmlvdi)
	{
		FILE_INFORMATION_STRUCT *pFI = m_pFI;
		pnmlvdi->item.pszText = PathFindFileName(pFI->Name);
		return 0;
	}

	LRESULT OnDisp_ShortName(UINT,NMLVDISPINFO *pnmlvdi)
	{
		pnmlvdi->item.pszText = L"-";
		FILE_INFORMATION_STRUCT *pFI = m_pFI;
		if( pFI->ShortName && *pFI->ShortName != UNICODE_NULL )
			pnmlvdi->item.pszText = PathFindFileName(pFI->ShortName);
		return 0;
	}

	LRESULT OnDisp_Extension(UINT,NMLVDISPINFO *pnmlvdi)
	{
		*pnmlvdi->item.pszText = 0;
		FILE_INFORMATION_STRUCT *pFI = m_pFI;
		if( (pFI->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			pnmlvdi->item.pszText = PathFindExtension(pFI->Name);
		if( *pnmlvdi->item.pszText == 0 )
			pnmlvdi->item.pszText = L"-";
		return 0;
	}

	LRESULT OnDisp_DateTime(UINT id,NMLVDISPINFO *pnmlvdi)
	{
		LONGLONG dt = 0;
		UINT fmt = 0;
		switch( id )
		{
			case TitleCreationTime:
				dt = m_pFI->CreationTime.QuadPart;
				fmt = 0;
				break;
			case TitleLastWriteTime:
				dt = m_pFI->LastWriteTime.QuadPart;
				fmt = 0;
				break;
			case TitleLastAccessTime:
				dt = m_pFI->LastAccessTime.QuadPart;
				fmt = 0;
				break;
			case TitleChangeTime:
				dt = m_pFI->ChangeTime.QuadPart;
				fmt = 0;
				break;
		}

		switch( fmt )
		{
			case 0:
				_GetDateTimeStringEx2(dt,pnmlvdi->item.pszText,pnmlvdi->item.cchTextMax,NULL,L"HH:mm:ss.nnn",0,1);
				break;
			case 1:	
				StringCchPrintf(pnmlvdi->item.pszText,pnmlvdi->item.cchTextMax,L"0x%016I64X",dt);
				break;
		}

		return 0;
	}

	LRESULT OnDisp_Size(UINT id,NMLVDISPINFO *pnmlvdi)
	{
		LONGLONG cb;

		if( id == TitleEndOfFile )
			cb = m_pFI->EndOfFile.QuadPart;
		else if( id == TitleAllocationSize )
			cb = m_pFI->AllocationSize.QuadPart;
		else if( id == TitleEAData )
			cb = m_pFI->EaSize;

		if( 0 )
		{
			StrFormatByteSizeW(cb,pnmlvdi->item.pszText,pnmlvdi->item.cchTextMax);
		}
		else
		{
			_CommaFormatString(cb,pnmlvdi->item.pszText);
		}

		return 0;
	}

	LRESULT OnDisp_Count(UINT id,NMLVDISPINFO *pnmlvdi)
	{
		LONGLONG cb;

		if( id == TitleNumberOfHardLink )
			cb = m_pFI->NumberOfLinks;
		else
			cb = 0;

		_CommaFormatString(cb,pnmlvdi->item.pszText);

		return 0;
	}

	LRESULT OnDisp_Boolean(UINT id,NMLVDISPINFO *pnmlvdi)
	{
		BOOLEAN b;

		if( id == TitleDirectory )
			b = m_pFI->Directory;
		else if( id == TitleDeletePending )
			b = m_pFI->DeletePending;

		pnmlvdi->item.pszText = b ? L"true" : L"false";

		return 0;
	}

	LRESULT OnDisp_Attributes(UINT id,NMLVDISPINFO *pnmlvdi)
	{
		NTFile_GetAttributeString(m_pFI->FileAttributes,pnmlvdi->item.pszText,pnmlvdi->item.cchTextMax);
		return 0;
	}

	LRESULT OnDisp_ObjectId(UINT id,NMLVDISPINFO *pnmlvdi)
	{
		FILE_INFORMATION_STRUCT *pFI = m_pFI;

		if( !pFI->State.ObjectId )
		{
			pnmlvdi->item.pszText = L"-";
			return 0;
		}

		PUCHAR pData = NULL;
		switch( id )
		{
			case TitleObjectId:
				pData = pFI->ObjectId.ObjectId;
				break;
			case TitleBirthVolumeId:
				pData = pFI->ObjectId.BirthVolumeId;
				break;
			case TitleBirthObjectId:
				pData = pFI->ObjectId.BirthObjectId;
				break;
			case TitleDomainId:
				pData = pFI->ObjectId.DomainId;
				break;
		}

		if( pData )
		{
			_StringFromGUID((const GUID *)pData,pnmlvdi->item.pszText,pnmlvdi->item.cchTextMax);
		}

		return 0;
	}

	LRESULT OnDisp_FileId(UINT id,NMLVDISPINFO *pnmlvdi)
	{
		FILE_INFORMATION_STRUCT *pFI = m_pFI;
		StringCchPrintf(pnmlvdi->item.pszText,pnmlvdi->item.cchTextMax,L"0x%016I64X",pFI->FileReferenceNumber.QuadPart);
		return 0;
	}
	
	LRESULT OnDisp_Location(UINT id,NMLVDISPINFO *pnmlvdi)
	{
		FILE_INFORMATION_STRUCT *pFI = m_pFI;
		pnmlvdi->item.pszText = L""; // todo:
		return 0;
	}

	LRESULT OnDisp_Wof(UINT id,NMLVDISPINFO *pnmlvdi)
	{
		FILE_INFORMATION_STRUCT *pFI = m_pFI;
		if( pFI->State.Wof )
		{
			if( pFI->Wof.ExternalInfo->Provider == WOF_PROVIDER_WIM )
			{
				StringCchPrintf(pnmlvdi->item.pszText,pnmlvdi->item.cchTextMax,L"WIM backing up: 0x%X 0x%X 0x%X",pFI->Wof.WimInfo->DataSourceId,pFI->Wof.WimInfo->ResourceHash,pFI->Wof.WimInfo->Flags);
			}
			else if( pFI->Wof.ExternalInfo->Provider == WOF_PROVIDER_FILE )
			{
				PWSTR psz = NULL;
				switch(pFI->Wof.FileInfo->Algorithm)
				{
					case FILE_PROVIDER_COMPRESSION_XPRESS4K:
						psz = L"XPRESS4K";
						break;
					case FILE_PROVIDER_COMPRESSION_LZX:
						psz = L"LZX";
						break;
					case FILE_PROVIDER_COMPRESSION_XPRESS8K:
						psz = L"XPRESS8K";
						break;
					case FILE_PROVIDER_COMPRESSION_XPRESS16K:
						psz = L"XPRESS16K";
						break;
				}
				if( psz )
					StringCchPrintf(pnmlvdi->item.pszText,pnmlvdi->item.cchTextMax,L"File backing up: %s",psz);
				else
					StringCchPrintf(pnmlvdi->item.pszText,pnmlvdi->item.cchTextMax,L"File backing up: 0x%X 0x%X",pFI->Wof.FileInfo->Algorithm,pFI->Wof.FileInfo->Flags);
			}
			else
			{
				StringCchPrintf(pnmlvdi->item.pszText,pnmlvdi->item.cchTextMax,L"Unknown Provider 0x%X",pFI->Wof.ExternalInfo->Provider);
			}
		}
		else
		{
			pnmlvdi->item.pszText = L"-";
		}
		return 0;
	}

	VOID InitDisplayHandler(_ITEMDISP_HANDLER_DEF<CFileInfoView>*& pch)
	{
		static _ITEMDISP_HANDLER_DEF<CFileInfoView> ch[] =
		{
			ITEMDISP_HANDLER_MAP_DEF(TitleNone,                   NULL),
			ITEMDISP_HANDLER_MAP_DEF(TitleName,                   &CFileInfoView::OnDisp_Name),
			ITEMDISP_HANDLER_MAP_DEF(TitleAttributes,             &CFileInfoView::OnDisp_Attributes),
			ITEMDISP_HANDLER_MAP_DEF(TitleLastWriteTime,          &CFileInfoView::OnDisp_DateTime),
			ITEMDISP_HANDLER_MAP_DEF(TitleCreationTime,           &CFileInfoView::OnDisp_DateTime),
			ITEMDISP_HANDLER_MAP_DEF(TitleLastAccessTime,         &CFileInfoView::OnDisp_DateTime),
			ITEMDISP_HANDLER_MAP_DEF(TitleChangeTime,             &CFileInfoView::OnDisp_DateTime),
			ITEMDISP_HANDLER_MAP_DEF(TitleLastWriteTimeDirEntry,  &CFileInfoView::OnDisp_DateTime),
			ITEMDISP_HANDLER_MAP_DEF(TitleCreationTimeDirEntry,   &CFileInfoView::OnDisp_DateTime),
			ITEMDISP_HANDLER_MAP_DEF(TitleLastAccessTimeDirEntry, &CFileInfoView::OnDisp_DateTime),
			ITEMDISP_HANDLER_MAP_DEF(TitleChangeTimeDirEntry,     &CFileInfoView::OnDisp_DateTime),
			ITEMDISP_HANDLER_MAP_DEF(TitleEndOfFile,              &CFileInfoView::OnDisp_Size),
			ITEMDISP_HANDLER_MAP_DEF(TitleAllocationSize,         &CFileInfoView::OnDisp_Size),
			ITEMDISP_HANDLER_MAP_DEF(TitleEndOfFileDirEntry,      &CFileInfoView::OnDisp_Size),
			ITEMDISP_HANDLER_MAP_DEF(TitleAllocationSizeDirEntry, &CFileInfoView::OnDisp_Size),
			ITEMDISP_HANDLER_MAP_DEF(TitleShortName,              &CFileInfoView::OnDisp_ShortName),
			ITEMDISP_HANDLER_MAP_DEF(TitleExtension,              &CFileInfoView::OnDisp_Extension),
			ITEMDISP_HANDLER_MAP_DEF(TitleNumberOfHardLink,       &CFileInfoView::OnDisp_Count),
			ITEMDISP_HANDLER_MAP_DEF(TitleDirectory,              &CFileInfoView::OnDisp_Boolean),
			ITEMDISP_HANDLER_MAP_DEF(TitleDeletePending,          &CFileInfoView::OnDisp_Boolean),
			ITEMDISP_HANDLER_MAP_DEF(TitleEAData,                 &CFileInfoView::OnDisp_Size),
			ITEMDISP_HANDLER_MAP_DEF(TitleObjectId,               &CFileInfoView::OnDisp_ObjectId),
			ITEMDISP_HANDLER_MAP_DEF(TitleBirthVolumeId,          &CFileInfoView::OnDisp_ObjectId),
			ITEMDISP_HANDLER_MAP_DEF(TitleBirthObjectId,          &CFileInfoView::OnDisp_ObjectId),
			ITEMDISP_HANDLER_MAP_DEF(TitleDomainId,               &CFileInfoView::OnDisp_ObjectId),
			ITEMDISP_HANDLER_MAP_DEF(TitleLocation,               &CFileInfoView::OnDisp_Location),
			ITEMDISP_HANDLER_MAP_DEF(TitleFileId,                 &CFileInfoView::OnDisp_FileId),
			ITEMDISP_HANDLER_MAP_DEF(TitleWofItem,                &CFileInfoView::OnDisp_Wof),
		};
		int cTableSize = TitleTableSize;
		pch = new _ITEMDISP_HANDLER_DEF<CFileInfoView>[ cTableSize ];
		ZeroMemory(pch,sizeof(_ITEMDISP_HANDLER_DEF<CFileInfoView>) * cTableSize);

		for(int i = 0; i < ARRAYSIZE(ch); i++)
		{
			pch[ ch[i].tid ] = ch[i];
		}
	}

	LRESULT OnGetDispInfo(NMHDR *pnmhdr)
	{
		NMLVDISPINFO *pdi = (NMLVDISPINFO*)pnmhdr;
		int idTitle = (int)ListViewEx_GetItemData(pnmhdr->hwndFrom,pdi->item.iItem);

		if( m_pch == NULL )
		{
			// initialize once only
			InitDisplayHandler( m_pch );
		}

		ASSERT( idTitle < TitleCount );

		if( pdi->item.mask & LVIF_TEXT )
		{
			if( m_pFI )
			{
				if( (idTitle != -1) && (idTitle < TitleCount) && m_pch[ idTitle ].pfn )
					return (this->*m_pch[ idTitle ].pfn)(idTitle,pdi);

				pdi->item.pszText = L"(No Text)";
			}
		}

		return 0;	
	}

	LRESULT OnItemChanged(NMHDR *pnmhdr)
	{
		NMLISTVIEW *pnmlv = (NMLISTVIEW *)pnmhdr;

		if( ((pnmlv->uOldState == 0)|| ((pnmlv->uNewState & (LVIS_SELECTED)) == (LVIS_SELECTED))) 
			 && ((pnmlv->uNewState & (LVIS_SELECTED|LVIS_FOCUSED)) == (LVIS_SELECTED|LVIS_FOCUSED)) )
		{
			SELECT_TITLE sp;
			sp.pFileInfo = m_pFI;
			sp.TitleId = (UINT)pnmlv->lParam;
			SendMessage(GetParent(m_hWnd),WM_CONTROL_MESSAGE,CODE_SELECT_ITEM,(LPARAM)&sp);
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
		}
		return CBaseWindow::WndProc(hWnd,uMsg,wParam,lParam);
	}

	VOID UpdateLayout(int cx,int cy)
	{
		if( m_hWndList )
		{
			int cxList = cx;
			int cyList = cy;

			SetWindowPos(m_hWndList,NULL,
					0,
					0,
					cxList,
					cyList,
					SWP_NOZORDER);
		}
	}

	int InsertGroup(HWND hWndList,int iGroupId,LPCWSTR pszHeaderText,BOOL fCollapsed=FALSE,int iImage=I_IMAGENONE,LPCWSTR pszSubTitle=NULL)
	{
		LVGROUP group = {0};

		group.cbSize      = sizeof(LVGROUP);
		group.mask        = LVGF_GROUPID|LVGF_TITLEIMAGE|LVGF_HEADER|LVGF_STATE;
		group.iTitleImage = iImage;
		group.pszHeader   = (LPWSTR)pszHeaderText;
		group.uAlign      = LVGA_HEADER_LEFT;
		group.iGroupId    = iGroupId;
		group.state       = LVGS_COLLAPSIBLE | (fCollapsed ? LVGS_COLLAPSED : 0);

		if( pszSubTitle )
		{
			group.mask |= LVGF_SUBTITLE;
			group.pszSubtitle = (LPWSTR)pszSubTitle;
		}

		return (int)ListView_InsertGroup(hWndList,-1,(PLVGROUP)&group);
	}

	typedef struct _GROUP_ITEM
	{
		int idGroup;
		UINT idGroupTitle;
		int fCollapsed;
		PCWSTR Text;
	} GROUP_ITEM;

	void InitGroup()
	{
		GROUP_ITEM Group[] = {
			{ ID_GROUP_NAME,         0, 0,L"Name" },
			{ ID_GROUP_DATETIME,     0, 0,L"Date/Time" },
			{ ID_GROUP_SIZE,         0, 0,L"Size"  },
			{ ID_GROUP_ATTRIBUTES,   0, 0,L"Attributes" },
			{ ID_GROUP_ALTSTREAMS,   0, 0,L"Stream" },
			{ ID_GROUP_OBJECTID,     0, 0,L"Object ID" },
			{ ID_GROUP_EA,           0, 0,L"Extended Attributes" },
			{ ID_GROUP_REPARSEPOINT, 0, 0,L"Reparse Point Data" },
			{ ID_GROUP_OTHERS,       0, 0,L"Others" },
		};
		int cGroupItem = ARRAYSIZE(Group);

		for(int i = 0; i < cGroupItem; i++)
		{
			InsertGroup(m_hWndList,Group[i].idGroup,Group[i].Text,Group[i].fCollapsed);
		}
	}

	HRESULT InitList(HWND hWndList)
	{
		_EnableVisualThemeStyle(hWndList);

		ListView_SetExtendedListViewStyle(hWndList,LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP);

		HIMAGELIST himl = ImageList_Create(6,24,ILC_COLOR32,1,1);
		ListView_SetImageList(hWndList,himl,LVSIL_SMALL);

		InitColumns(hWndList);

		ListView_EnableGroupView(hWndList,TRUE);
		ListView_SetSelectedColumn(hWndList,0);

		return S_OK;
	}

	BOOL InitColumns(HWND hWndList)
	{
		LVCOLUMN lvc = {0};

		lvc.mask    = LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_ORDER;
		lvc.fmt     = 0;
		lvc.cx      = 220;
		lvc.pszText = L"Item";
		lvc.iOrder  = 0;
		ListView_InsertColumn(hWndList,0,&lvc);

		lvc.pszText = L"Value";
		lvc.iOrder  = 1;
		lvc.cx      = 320;
		ListView_InsertColumn(hWndList,1,&lvc);

		return TRUE;
	}

	typedef struct _TITLE_NAME
	{
		PWSTR Name;
		UINT TitleId;
		UINT GroupId;
	} TITLE_NAME;

	VOID FillItemTitles()
	{
		TITLE_NAME tn[] =
		{
			{ L"Name",                     TitleName,               ID_GROUP_NAME },
			{ L"Short Name",               TitleShortName,          ID_GROUP_NAME },
			{ L"Extension",                TitleExtension,          ID_GROUP_NAME },
//			{ L"Location",                 TitleLocation,           ID_GROUP_NAME },

			{ L"File Attributes",          TitleAttributes,         ID_GROUP_ATTRIBUTES },

			{ L"End of File",              TitleEndOfFile,          ID_GROUP_SIZE },
			{ L"Allocation Size",          TitleAllocationSize,     ID_GROUP_SIZE },

			{ L"Last Write Time",          TitleLastWriteTime,      ID_GROUP_DATETIME },
			{ L"Creation Time",            TitleCreationTime,       ID_GROUP_DATETIME },
			{ L"Last Access Time",         TitleLastAccessTime,     ID_GROUP_DATETIME },
			{ L"Change Time",              TitleChangeTime,         ID_GROUP_DATETIME },

			{ L"Object Id",                TitleObjectId,           ID_GROUP_OBJECTID },
			{ L"Birth Volume Id",          TitleBirthVolumeId,      ID_GROUP_OBJECTID },
			{ L"Birth Object Id",          TitleBirthObjectId,      ID_GROUP_OBJECTID },
			{ L"Domain Id",                TitleDomainId,           ID_GROUP_OBJECTID },

			{ L"File Reference Number",    TitleFileId,             ID_GROUP_OTHERS },
			{ L"EA Data",                  TitleEAData,             ID_GROUP_OTHERS },
			{ L"Number of HardLink",       TitleNumberOfHardLink,   ID_GROUP_OTHERS },
			{ L"Directory",                TitleDirectory,          ID_GROUP_OTHERS },
			{ L"Delete Pending",           TitleDeletePending,      ID_GROUP_OTHERS },
			{ L"Overlay File",             TitleWofItem,            ID_GROUP_OTHERS },
		};


		LVITEM lvi = {0};
		lvi.mask     = LVIF_TEXT|LVIF_IMAGE|LVIF_INDENT|LVIF_PARAM|LVIF_GROUPID;
		lvi.iImage   = I_IMAGENONE;
		lvi.iIndent  = 1;
		lvi.lParam   = 0;

		for(int i = 0; i < ARRAYSIZE(tn); i++)
		{
			lvi.iItem    = i;
			lvi.iGroupId = 0;
			lvi.pszText  = tn[i].Name;
			lvi.iGroupId = tn[i].GroupId;
			lvi.lParam   = tn[i].TitleId;
			ListView_InsertItem(m_hWndList,&lvi);

			ListView_SetItemText(m_hWndList,i,1,LPSTR_TEXTCALLBACK);
		}
	}

	VOID DeleteGroupItems(int iGroupId)
	{
		LVITEM lvi = {0};
		int cItems = ListView_GetItemCount(m_hWndList);
		for(int i = (cItems-1); i >= 0; i--)
		{
			lvi.mask = LVIF_GROUPID;
			lvi.iItem = i;
			ListView_GetItem(m_hWndList,&lvi);
			if( lvi.iGroupId == iGroupId )
			{
				ListView_DeleteItem(m_hWndList,i);
			}
		}
	}

	BOOL IsGroupVisible(int iGroupId)
	{
		return ListView_GetGroupState(m_hWndList,iGroupId,LVGS_HIDDEN);
	}

	INT GetGroupItemCount(int iGroupId)
	{
		LVGROUP lvg = {0};
		lvg.cbSize = sizeof(lvg);
		lvg.mask = LVGF_ITEMS|LVGF_STATE|LVGF_GROUPID;
		lvg.iGroupId = iGroupId;
		lvg.stateMask = LVGS_HIDDEN;
		if( ListView_GetGroupInfo(m_hWndList,iGroupId,&lvg) == -1 )
			return -1;
		return lvg.cItems;
	}

	BOOL SetGroupVisible(int iGroupId,BOOL bVisible)
	{
		LVGROUP lvg = {0};
		lvg.cbSize = sizeof(lvg);
		lvg.mask = LVGF_STATE;
		lvg.state = bVisible ? LVGS_NORMAL : LVGS_HIDDEN;
		lvg.stateMask = LVGS_HIDDEN;
		return (ListView_SetGroupInfo(m_hWndList,iGroupId,&lvg) != -1);
	}

	HRESULT FillItems(SELECT_FILE *pFile)
	{
		SetRedraw(m_hWndList,FALSE);

		if( ListView_GetItemCount(m_hWndList) == 0 )
		{
			FillItemTitles();
		}

		//
		// Open File
		//
		HANDLE hFile;

		ULONG DesiredAccess = 0;
		ULONG ShareAccess = 0;
		ULONG OpenOptions = 0;

		// FILE_READ_DATA             file
		// FILE_LIST_DIRECTORY        directory
		// FILE_WRITE_DATA            file
		// FILE_ADD_FILE              directory
		// FILE_APPEND_DATA           file
		// FILE_ADD_SUBDIRECTORY      directory
		// FILE_READ_EA               file & directory
		// FILE_WRITE_EA              file & directory
		// FILE_EXECUTE               file
		// FILE_TRAVERSE              directory
		// FILE_DELETE_CHILD          directory
		// FILE_READ_ATTRIBUTES       all
		// FILE_WRITE_ATTRIBUTES      all
		// FILE_ALL_ACCESS           (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x1FF)
		// DELETE                           (0x00010000L)
		// READ_CONTROL                     (0x00020000L)
		// WRITE_DAC                        (0x00040000L)
		// WRITE_OWNER                      (0x00080000L)
		// SYNCHRONIZE                      (0x00100000L)
		// ACCESS_SYSTEM_SECURITY           (0x01000000L)
		// MAXIMUM_ALLOWED                  (0x02000000L)
		// GENERIC_READ                     (0x80000000L)
		// GENERIC_WRITE                    (0x40000000L)
		// GENERIC_EXECUTE                  (0x20000000L)
		// GENERIC_ALL                      (0x10000000L)
		// STANDARD_RIGHTS_REQUIRED         (0x000F0000L)

		DesiredAccess = FILE_READ_ATTRIBUTES|FILE_READ_EA|READ_CONTROL|SYNCHRONIZE;

		ShareAccess = FILE_SHARE_READ|FILE_SHARE_WRITE;

		OpenOptions = FILE_OPEN_FOR_BACKUP_INTENT|FILE_SYNCHRONOUS_IO_NONALERT|FILE_OPEN_REPARSE_POINT;

		if( NTFile_OpenFile(&hFile,pFile->pszPath,DesiredAccess,ShareAccess,OpenOptions) == S_OK )
		{
			NTFile_FreeFileInformation( m_pFI );

			NTFile_GatherFileInformation(hFile,&m_pFI);

			NTFile_CloseFile(hFile);
		}
		else
		{
			NTFile_FreeFileInformation( m_pFI );
			m_pFI = NULL;
		}

		//
		// Open Directory
		//
#if 0 // Reserved
		HANDLE hDirectory;

		DesiredAccess = FILE_READ_ATTRIBUTES|FILE_TRAVERSE|SYNCHRONIZE;

		OpenOptions = FILE_OPEN_FOR_BACKUP_INTENT|FILE_SYNCHRONOUS_IO_NONALERT|FILE_OPEN_REPARSE_POINT|FILE_DIRECTORY_FILE;

		if( NTFile_OpenFile(&hDirectory,pFile->pszLocation,DesiredAccess,ShareAccess,OpenOptions) == S_OK )
		{
			NTFile_CloseFile(hDirectory);
		}
#endif
		if( GetGroupItemCount(ID_GROUP_ALTSTREAMS) > 0 )
			DeleteGroupItems(ID_GROUP_ALTSTREAMS);
		if( GetGroupItemCount(ID_GROUP_REPARSEPOINT) > 0 )
			DeleteGroupItems(ID_GROUP_REPARSEPOINT);
		if( GetGroupItemCount(ID_GROUP_EA) > 0 )
			DeleteGroupItems(ID_GROUP_EA);

		if( m_pFI )
		{
			if( m_pFI->AltStream.cAltStreamName > 0 )
			{
				InsertAltStreams();
			}

			if( m_pFI->EaSize > 0 )
			{
				InsertEaItems();
			}

			if( m_pFI->State.ReparsePoint )
			{
				InsertReparsePoint();
			}
		}

		SetRedraw(m_hWndList,TRUE);

		return S_OK;
	}

	void InsertAltStreams()
	{
		LVITEM lvi = {0};
		lvi.mask     = LVIF_TEXT|LVIF_IMAGE|LVIF_INDENT|LVIF_PARAM|LVIF_GROUPID;
		lvi.iImage   = I_IMAGENONE;
		lvi.iIndent  = 1;
		lvi.lParam   = 0;
		lvi.iGroupId = ID_GROUP_ALTSTREAMS;
		lvi.iItem    = ListView_GetItemCount(m_hWndList);

		WCHAR szTitle[64];
		int num = 1;
		for(int i = 0; i < m_pFI->AltStream.cAltStreamName; i++)
		{
			if( m_pFI->AltStream.AltStreamName[i].Name[0] == L':' &&
				m_pFI->AltStream.AltStreamName[i].Name[1] == L':' )
			{
				StringCchPrintf(szTitle,ARRAYSIZE(szTitle),L"Data Stream");
			}
			else
			{
				StringCchPrintf(szTitle,ARRAYSIZE(szTitle),L"Alternate Stream #%d",num++);
			}

			lvi.pszText  = szTitle;
			ListView_InsertItem(m_hWndList,&lvi);

			ListView_SetItemText(m_hWndList,lvi.iItem,1,m_pFI->AltStream.AltStreamName[i].Name);

			lvi.iItem++;
		}
	}

	void InsertEaItems()
	{
		WCHAR szTitle[64];
		LVITEM lvi = {0};
		lvi.mask     = LVIF_TEXT|LVIF_IMAGE|LVIF_INDENT|LVIF_PARAM|LVIF_GROUPID;
		lvi.iImage   = I_IMAGENONE;
		lvi.iIndent  = 1;
		lvi.lParam   = 0;
		lvi.iGroupId = ID_GROUP_EA;
		lvi.iItem    = ListView_GetItemCount(m_hWndList);

		int num = 1;
		for(ULONG i = 0; i < m_pFI->EaBuffer->EaCount; i++)
		{
			StringCchPrintf(szTitle,ARRAYSIZE(szTitle),L"EA #%d",i+1);
			lvi.pszText  = szTitle;
			ListView_InsertItem(m_hWndList,&lvi);

			StringCchPrintf(szTitle,ARRAYSIZE(szTitle),L"%S",m_pFI->EaBuffer->Ea[i].Name);
			ListView_SetItemText(m_hWndList,lvi.iItem,1,szTitle);

			lvi.iItem++;
		}
	}

	void InsertReparsePoint()
	{
		WCHAR szTag[32];

		LVITEM lvi = {0};
		lvi.mask     = LVIF_TEXT|LVIF_IMAGE|LVIF_INDENT|LVIF_PARAM|LVIF_GROUPID;
		lvi.iImage   = I_IMAGENONE;
		lvi.iIndent  = 1;
		lvi.lParam   = 0;
		lvi.iGroupId = ID_GROUP_REPARSEPOINT;
		lvi.iItem    = ListView_GetItemCount(m_hWndList);

		lvi.pszText  = L"Rearse Tag";
		ListView_InsertItem(m_hWndList,&lvi);

		switch( m_pFI->ReparsePointInfo.ReparseTag )
		{
			case IO_REPARSE_TAG_SYMLINK:
			{
				StringCchPrintf(szTag,ARRAYSIZE(szTag),L"0x%08X (Symbolic Link)",m_pFI->ReparsePointInfo.ReparseTag);
				ListView_SetItemText(m_hWndList,lvi.iItem,1,szTag);
				lvi.iItem++;

				lvi.pszText  = L"Target Path";
				ListView_InsertItem(m_hWndList,&lvi);
				ListView_SetItemText(m_hWndList,lvi.iItem,1,m_pFI->ReparsePointInfo.SymLink.TargetPath);
				lvi.iItem++;

				lvi.pszText  = L"Print Path";
				ListView_InsertItem(m_hWndList,&lvi);
				ListView_SetItemText(m_hWndList,lvi.iItem,1,m_pFI->ReparsePointInfo.SymLink.PrintPath);
				lvi.iItem++;
				break;
			}
			case IO_REPARSE_TAG_MOUNT_POINT:
			{
				StringCchPrintf(szTag,ARRAYSIZE(szTag),L"0x%08X (Mount Point)",m_pFI->ReparsePointInfo.ReparseTag);
				ListView_SetItemText(m_hWndList,lvi.iItem,1,szTag);

				lvi.pszText  = L"Target Path";
				ListView_InsertItem(m_hWndList,&lvi);
				ListView_SetItemText(m_hWndList,lvi.iItem,1,m_pFI->ReparsePointInfo.MountPoint.TargetPath);
				lvi.iItem++;

				lvi.pszText  = L"Print Path";
				ListView_InsertItem(m_hWndList,&lvi);
				ListView_SetItemText(m_hWndList,lvi.iItem,1,m_pFI->ReparsePointInfo.MountPoint.PrintPath);
				lvi.iItem++;
				break;
			}
			case IO_REPARSE_TAG_APPEXECLINK:
			{
				StringCchPrintf(szTag,ARRAYSIZE(szTag),L"0x%08X (AppExecLink)",m_pFI->ReparsePointInfo.ReparseTag);
				ListView_SetItemText(m_hWndList,lvi.iItem,1,szTag);

				lvi.pszText  = L"Package ID";
				ListView_InsertItem(m_hWndList,&lvi);
				ListView_SetItemText(m_hWndList,lvi.iItem,1,m_pFI->ReparsePointInfo.AppExecLink.PackageID);
				lvi.iItem++;

				lvi.pszText  = L"Entry Point";
				ListView_InsertItem(m_hWndList,&lvi);
				ListView_SetItemText(m_hWndList,lvi.iItem,1,m_pFI->ReparsePointInfo.AppExecLink.EntryPoint);
				lvi.iItem++;

				lvi.pszText  = L"Executable";
				ListView_InsertItem(m_hWndList,&lvi);
				ListView_SetItemText(m_hWndList,lvi.iItem,1,m_pFI->ReparsePointInfo.AppExecLink.Executable);
				lvi.iItem++;

				lvi.pszText  = L"Application Type";
				ListView_InsertItem(m_hWndList,&lvi);
				ListView_SetItemText(m_hWndList,lvi.iItem,1,m_pFI->ReparsePointInfo.AppExecLink.ApplicType);
				lvi.iItem++;
				break;
			}
			default:
			{
				StringCchPrintf(szTag,ARRAYSIZE(szTag),L"0x%08X",m_pFI->ReparsePointInfo.ReparseTag);
				ListView_SetItemText(m_hWndList,lvi.iItem,1,szTag);
				break;
			}
		}
	}

	virtual HRESULT UpdateData(PVOID pFile)
	{
		return FillItems((SELECT_FILE*)pFile);
	}	
};
