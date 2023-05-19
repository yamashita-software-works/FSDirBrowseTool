#pragma once

#include "pagewbdbase.h"
#include "page_root.h"
#include "page_fileinfo.h"

interface IFileViewBaseWindow
{
	virtual HWND GetHWND() const = 0;
	virtual HRESULT Create(HWND hWnd,HWND *phWndFileList=NULL) = 0;
	virtual HRESULT Destroy() = 0;
	virtual HRESULT InitData() = 0;
	virtual HRESULT InitLayout(const RECT *prc) = 0;
	virtual HRESULT SelectData(SELECT_FILE *Path) = 0;
};

HRESULT FileViewBase_CreateObject(HINSTANCE hInstance,IFileViewBaseWindow **pObject);
