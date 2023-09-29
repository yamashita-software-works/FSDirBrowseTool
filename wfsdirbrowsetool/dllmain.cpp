#include "stdafx.h"
#include "dirinfowindow.h"

HINSTANCE hInstance = NULL;

HINSTANCE _GetResourceInstance()
{
	return hInstance;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			hInstance = hModule;
			_MemInit();
			break;
		case DLL_PROCESS_DETACH:
			_MemEnd();
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}


EXTERN_C
HWND
WINAPI
CreateDirectoryBrowseTool(
	HWND hwnd
	)
{
	return DirectoryBrowseTool_CreateWindow(hwnd);
}

// module global
static HIMAGELIST m_himl = NULL;
static int m_iImageUpDir = I_IMAGENONE;

INT GetUpDirImageIndex()
{
	return m_iImageUpDir;
}

HIMAGELIST GetGlobalShareImageList()
{
	if( m_himl == NULL )
	{
		//
		// The image lists retrieved through this function are 
		// global system image lists;
		// do not call ImageList_Destroy using them.
		//
		Shell_GetImageLists(NULL,&m_himl);

#ifdef _DEBUG
		int cImages;
		cImages = ImageList_GetImageCount(m_himl);
		_TRACE("System image count=%u\n",cImages);
#endif

		int cx,cy;
		ImageList_GetIconSize(m_himl,&cx,&cy);

		HICON hIcon = (HICON)LoadImage(GetModuleHandle(L"shell32"), MAKEINTRESOURCE(46), IMAGE_ICON, cx, cy, 0);
		m_iImageUpDir = ImageList_AddIcon(m_himl,hIcon);
		DestroyIcon(hIcon);
	}
	return m_himl;
}
