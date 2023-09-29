#pragma once

#include "basewindow.h"

class CPageWndBase : public CBaseWindow
{
public:
	CPageWndBase()
	{
	}

	~CPageWndBase()
	{
	}

	virtual HRESULT InitData(PVOID)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT InitLayout(const RECT *prc)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT UpdateData(PVOID)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT InvokeCommand(UINT CmdId)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT QueryCmdState(UINT CmdId,UINT *State)
	{
		return E_NOTIMPL;
	}
};
