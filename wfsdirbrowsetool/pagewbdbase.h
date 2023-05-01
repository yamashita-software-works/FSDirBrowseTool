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
};
