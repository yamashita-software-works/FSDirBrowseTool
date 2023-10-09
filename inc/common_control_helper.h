#pragma once

__forceinline LPARAM TreeView_GetItemData(HWND hwndTV,HTREEITEM hItem) {
	TVITEMEX tviex = {0};
	tviex.mask = TVIF_PARAM;
	tviex.hItem = hItem;
	if( TreeView_GetItem(hwndTV,&tviex) )
		return tviex.lParam;
	return 0;
}

__forceinline BOOL TreeView_GetItemText(HWND hwndTV,HTREEITEM hItem,PTSTR pszText,int cchTextMax) {
	TVITEMEX tviex = {0};
	tviex.mask = TVIF_TEXT;
	tviex.hItem = hItem;
	tviex.cchTextMax = cchTextMax;
	tviex.pszText = pszText;
	return TreeView_GetItem(hwndTV,&tviex);
}

__forceinline BOOL TreeView_SetChildren(HWND hwndTV,HTREEITEM hItem,int cChildren) {
	TVITEMEX tviex = {0};
	tviex.mask = TVIF_CHILDREN;
	tviex.hItem = hItem;
	tviex.cChildren = cChildren;
	return TreeView_SetItem(hwndTV,&tviex);
}

inline LPARAM ListViewEx_GetItemData(HWND hwndLV,int iItem)
{
	LVITEM lvi = {0};
	lvi.mask = LVIF_PARAM;
	lvi.iItem = iItem;
	ListView_GetItem(hwndLV,&lvi);
	return lvi.lParam;
}

inline BOOL ListViewEx_SetHeaderItemData(HWND hwndLV,int index, LPARAM lParam)
{
	HWND hwndHD = ListView_GetHeader(hwndLV);
	HDITEM hdi = {0};
	hdi.mask = HDI_LPARAM;
	hdi.lParam = lParam;
	return Header_SetItem(hwndHD,index,&hdi);
}

inline LPARAM ListViewEx_GetHeaderItemData(HWND hwndLV,int index)
{
	HWND hwndHD = ListView_GetHeader(hwndLV);
	HDITEM hdi = {0};
	hdi.mask = HDI_LPARAM;
	Header_GetItem(hwndHD,index,&hdi);
	return hdi.lParam;
}

inline int ListViewEx_GetColumnCount(HWND hwndLV)
{
	return Header_GetItemCount(ListView_GetHeader(hwndLV));
}

inline int ListViewEx_GetSubItem(HWND hwndLV,int iColumn)
{
	LVCOLUMN lvi = {0};
	lvi.mask = LVCF_SUBITEM;
	ListView_GetColumn(hwndLV,iColumn,&lvi);
	return lvi.iSubItem;
}

inline void ListViewEx_SetHeaderArrow(HWND hwndLV,int iSubItem,int iDirection)
{
	HWND h = ListView_GetHeader(hwndLV);

	HDITEM hdi = {0};
	DWORD f;
	hdi.mask = HDI_FORMAT;
	SendMessage(h,HDM_GETITEM,iSubItem,(LPARAM)&hdi);

	if( iDirection < 0 )
		f = HDF_SORTDOWN;
	else if( iDirection > 0 )
		f = HDF_SORTUP;
	else
		f = 0;

	hdi.fmt &= ~(HDF_SORTDOWN|HDF_SORTUP);
	hdi.fmt |= f;
	SendMessage(h,HDM_SETITEM,iSubItem,(LPARAM)&hdi);
}

inline int ListViewEx_HitTest(HWND hwndLV,POINT pt, UINT* pFlags)
{
	LVHITTESTINFO hti = {};
	hti.pt = pt;
	int nRes = (int)SendMessage(hwndLV, LVM_HITTEST, (WPARAM)-1, (LPARAM)&hti);
	if (pFlags != NULL)
		*pFlags = hti.flags;
	return nRes;
}

inline int ListViewEx_SubItemHitTest(HWND hwndLV,POINT pt, UINT* pFlags)
{
	LVHITTESTINFO hti = {};
	hti.pt = pt;
	int nRes = (int)SendMessage(hwndLV, LVM_SUBITEMHITTEST, (WPARAM)-1, (LPARAM)&hti);
	if (pFlags != NULL)
		*pFlags = hti.flags;
	return hti.iSubItem;
}

inline BOOL ListViewEx_SetItemState(HWND hwndLV,int iItem,UINT state,UINT mask)
{
	LVITEM lvi;
	lvi.stateMask = mask;
	lvi.state = state;
	return (BOOL)SendMessage(hwndLV,LVM_SETITEMSTATE,(WPARAM)(iItem),(LPARAM)&lvi);
}

inline BOOL ListViewEx_ClearSelectAll(HWND hwndLV,BOOL bClearFocus=FALSE)
{
	return ListViewEx_SetItemState(hwndLV,-1,0,LVIS_SELECTED|(bClearFocus ? LVNI_FOCUSED : 0));
}

inline VOID ListViewEx_AdjustWidthAllColumns(HWND hwndLV,BOOL UseHeader)
{
	int i,cHeaders = Header_GetItemCount(ListView_GetHeader(hwndLV));
	int cx = UseHeader ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE;
	for(i = 0; i < cHeaders; i++)
	{
		ListView_SetColumnWidth(hwndLV,i,cx);
	}
}

__forceinline int ListViewEx_GetCurSel(HWND hwndLV)
{
	return (int)ListView_GetNextItem(hwndLV,-1,LVNI_SELECTED|LVNI_FOCUSED);
}

template<class T,int N>
class CSimpleStack
{
	int m_pos;
	int m_max;
	T *m_stack;
public:
	CSimpleStack(bool ErrorAtThrow=false)
	{
		m_pos = 0;
		m_max = N;
		m_stack = new T[N];
		if( m_stack == NULL )
			if( ErrorAtThrow )
				throw (HRESULT)E_OUTOFMEMORY;
			else
				m_max = 0;
	}

	~CSimpleStack()
	{	
		if( m_stack )
			delete[] m_stack;
	}

	T Push(T a)
	{
		if( m_pos < m_max )
		{
			m_stack[m_pos++] = a;
			return a;
		}
		return (T)-1;
	}

	T Pop()
	{
		if( m_pos > 0 )
			return m_stack[--m_pos];
		return (T)-1;
	}

	int GetPos() const
	{
		return m_pos;
	}
};

class CWaitCursor
{
	HCURSOR m_hCursor;
public:
	CWaitCursor()
	{
		m_hCursor = SetCursor(LoadCursor(NULL,IDC_WAIT));
	}

	~CWaitCursor()
	{
		Resume();
	}

	VOID Resume()
	{
		SetCursor(m_hCursor);
	}
};
