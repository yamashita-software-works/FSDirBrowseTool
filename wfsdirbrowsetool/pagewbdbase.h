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

	virtual HRESULT UpdateData(PVOID)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT CommandHandler(UINT CmdId)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT QueryCmdState(UINT CmdId,UINT *State)
	{
		return E_NOTIMPL;
	}
};
