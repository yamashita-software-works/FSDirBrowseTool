#pragma once

interface IViewBaseWindow
{
	virtual HWND GetHWND() const = 0;
	virtual HRESULT Create(HWND hWnd,HWND *phWndFileList=NULL) = 0;
	virtual HRESULT Destroy() = 0;
	virtual HRESULT InitLayout(const RECT *prc) = 0;
	virtual HRESULT SelectPage(SELECT_ITEM *Path) = 0;
	virtual HRESULT InitData(SELECT_ITEM *Sel) = 0;
	virtual HRESULT UpdateData(SELECT_ITEM *Sel) = 0;
	virtual HRESULT QueryCmdState(UINT CmdId,UINT *State) = 0;
	virtual HRESULT InvokeCommand(UINT CmdId) = 0;
};

interface IItemTree
{
	virtual HWND GetHWND() const = 0;
	virtual HRESULT Create(HWND hWnd,HWND *phWndFileList=NULL) = 0;
	virtual HRESULT Destroy() = 0;
	virtual HRESULT InitLayout(const RECT *prc) = 0;
	virtual HRESULT FillTreeItems(UINT,PVOID,PCWSTR pszIniFilePath) = 0;
	virtual HWND SetNotifyWnd(HWND hwnd) = 0;
	virtual HWND GetNotifyWnd() = 0;
};
