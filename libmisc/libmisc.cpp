//***************************************************************************
//*                                                                         *
//*  libmisc.cpp                                                            *
//*                                                                         *
//*  Create: 2023-04-27                                                     *
//*                                                                         *
//*  Author: YAMASHITA Katsuhiro                                            *
//*                                                                         *
//*  Copyright (C) YAMASHITA Katsuhiro. All rights reserved.                *
//*  Licensed under the MIT License.                                        *
//*                                                                         *
//***************************************************************************
#include "stdafx.h"
#include "libmisc.h"

INT APIENTRY _initialize_libmisc()
{
    return 0;
}

INT APIENTRY _uninitialize_libmisc()
{
    return 0;
}

VOID WINAPI _EnableVisualThemeStyle(HWND hWnd)
{
	HINSTANCE h;
	h = LoadLibrary( L"UxTheme.dll" );

	if( h )
	{
		BOOL (WINAPI*SetWindowTheme)(HWND,LPCWSTR,LPCWSTR);

		(FARPROC&)SetWindowTheme = GetProcAddress( h, "SetWindowTheme" );

		if( SetWindowTheme )
		{
			SetWindowTheme(hWnd,L"Explorer",NULL);
		}

		FreeLibrary( h );
	}
}

VOID WINAPI _DisableVisualThemeStyle(HWND hWnd)
{
	HINSTANCE h;
	h = LoadLibrary( L"UxTheme.dll" );

	if( h )
	{
		BOOL (WINAPI*SetWindowTheme)(HWND,LPCWSTR,LPCWSTR);

		(FARPROC&)SetWindowTheme = GetProcAddress( h, "SetWindowTheme" );

		if( SetWindowTheme )
		{
			SetWindowTheme(hWnd,NULL,L"");
		}

		FreeLibrary( h );
	}
}

DWORD WINAPI _GetDesktopWorkArea(HWND hwnd,RECT *prc)
{
	HMONITOR hMonitor;
    MONITORINFO mi;
	RECT rc;
    GetWindowRect(hwnd,&rc);

	hMonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
	mi.cbSize = sizeof(mi);
	GetMonitorInfo(hMonitor,&mi);

	if( prc )
	{
		prc->left   = mi.rcWork.left;
		prc->top    = mi.rcWork.top;
	    prc->right  = mi.rcWork.right;
		prc->bottom = mi.rcWork.bottom;
	}

	return (DWORD)MAKELONG( (mi.rcWork.right-mi.rcWork.left),(mi.rcWork.bottom-mi.rcWork.top) );
}

BOOL WINAPI _SetProcessDPIAware()
{
	BOOL bSuccess = FALSE;

	HINSTANCE h;
	h = LoadLibrary( _T("USER32.dll") );

	if( h )
	{
		BOOL (WINAPI*pfnSetProcessDPIAware)(VOID);

		(FARPROC&)pfnSetProcessDPIAware = GetProcAddress( h, "SetProcessDPIAware" );

		if( pfnSetProcessDPIAware )
		{
			bSuccess = pfnSetProcessDPIAware();
		}

		FreeLibrary( h );
	}
	
	return bSuccess;
}

int WINAPI _DPI_Adjust_X(int x)
{
	HDC hdc = GetDC(NULL);
	if (hdc)
	{
		int _dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
		ReleaseDC(NULL,hdc);
		return MulDiv(x,_dpiX,96);
	}
	return x;
}

int WINAPI _DPI_Adjust_Y(int y)
{
	HDC hdc = GetDC(NULL);
	if (hdc)
	{
		int _dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(NULL,hdc);
		return MulDiv(y,_dpiY,96);
	}
	return y;
}

void WINAPI _DPI_Adjust_XY(int *px,int *py)
{
	HDC hdc = GetDC(NULL);
	if (hdc)
	{
		int _dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
		int _dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(NULL,hdc);
		*px = MulDiv(*px,_dpiX,96);
		*py = MulDiv(*py,_dpiY,96);
	}
}

SIZE WINAPI GetLogicalPixels(HWND hWnd)
{
	SIZE size;
	HDC hdc = GetWindowDC(hWnd);
	size.cx = GetDeviceCaps(hdc,LOGPIXELSX);
	size.cy = GetDeviceCaps(hdc,LOGPIXELSY);
	ReleaseDC(hWnd,hdc);
	return size;
}

int WINAPI GetLogicalPixelsY(HWND hWnd)
{
	HDC hdc = GetWindowDC(hWnd);
	int cy = GetDeviceCaps(hdc,LOGPIXELSY);
	ReleaseDC(hWnd,hdc);
	return cy;
}

int WINAPI GetLogicalPixelsX(HWND hWnd)
{
	HDC hdc = GetWindowDC(hWnd);
	int cx = GetDeviceCaps(hdc,LOGPIXELSX);
	ReleaseDC(hWnd,hdc);
	return cx;
}

BOOL WINAPI _MonGetMonitorRectFromWindow(HWND hWnd,RECT *prcResult,ULONG Flags,BOOLEAN bWorkspace)
{
	HMONITOR hmon;
	hmon = MonitorFromWindow(hWnd,Flags);

	MONITORINFO MonInfo = {0};
	MonInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(hmon,&MonInfo);

	if( bWorkspace )
		*prcResult = MonInfo.rcWork;
	else
		*prcResult = MonInfo.rcMonitor;

	return TRUE;
}

BOOL WINAPI _CenterWindow(HWND hwndChild, HWND hwndParent)
{
    RECT rcChild, rcParent;
    int  cxChild, cyChild, cxParent, cyParent;
    int  xNew, yNew;

    // Get the Height and Width of the child window
    GetWindowRect(hwndChild, &rcChild);
    cxChild = rcChild.right - rcChild.left;
    cyChild = rcChild.bottom - rcChild.top;

    // Get the Height and Width of the parent window
    GetWindowRect(hwndParent, &rcParent);
    cxParent = rcParent.right - rcParent.left;
    cyParent = rcParent.bottom - rcParent.top;

	RECT rcScreen;
	_MonGetMonitorRectFromWindow(hwndParent,&rcScreen,MONITOR_DEFAULTTONEAREST,TRUE);

    // Calculate new X position, then adjust for screen
    xNew = rcParent.left + ((cxParent - cxChild) / 2);
    if (xNew < rcScreen.left)
    {
        xNew = rcScreen.left;
    }
    else if ((xNew + cxChild) > rcScreen.right)
    {
        xNew = rcScreen.right - cxChild;
    }

    // Calculate new Y position, then adjust for screen
    yNew = rcParent.top  + ((cyParent - cyChild) / 2);
    if (yNew < rcScreen.top)
    {
        yNew = rcScreen.top;
    }
    else if ((yNew + cyChild) > rcScreen.bottom)
    {
        yNew = rcScreen.bottom - cyChild;
    }

    return SetWindowPos(hwndChild,
                        NULL,
                        xNew, yNew,
                        0,0,
                        SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
}

LPTSTR
WINAPI
_CommaFormatString(
	ULONGLONG val,
	LPTSTR pszOut
	)
{
	NUMBERFMT nfmt = {0};
	TCHAR szValue[64];
	swprintf_s(szValue,64,TEXT("%u"),val);
    nfmt.NumDigits    = 0; 
    nfmt.LeadingZero  = 0;
    nfmt.Grouping     = 3; 
    nfmt.lpDecimalSep = TEXT("."); 
    nfmt.lpThousandSep = TEXT(","); 
    nfmt.NegativeOrder = 0;
	GetNumberFormat(LOCALE_USER_DEFAULT,0,szValue,&nfmt,pszOut,100); //System Default
	return pszOut;
}

/////////////////////////////////////////////////////////////////////////////
// Date / Time Functions

#if 0
	static DWORD dwOldFlags = TIME_FORCE24HOURFORMAT;
#else
	static DWORD dwOldFlags = 0;
#endif

VOID WINAPI _GetDateTimeString(const ULONG64 DateTime,LPTSTR pszText,int cchTextMax)
{
	SYSTEMTIME st;
	FILETIME ftLocal;
	FileTimeToLocalFileTime((FILETIME*)&DateTime,&ftLocal);
	FileTimeToSystemTime(&ftLocal,&st);

	int cch;
	cch = GetDateFormat(LOCALE_USER_DEFAULT,
				0,
				&st, 
				NULL,
				pszText,cchTextMax);

	pszText[cch-1] = _T(' ');

	GetTimeFormat(LOCALE_USER_DEFAULT,
				dwOldFlags,
				&st, 
				NULL,
				&pszText[cch],cchTextMax-cch);
}

LPTSTR WINAPI _GetDateTimeStringFromFileTime(const FILETIME *DateTime,LPTSTR pszText,int cchTextMax)
{
	ULARGE_INTEGER li;
	li.HighPart = DateTime->dwHighDateTime;
	li.LowPart  = DateTime->dwLowDateTime;
	_GetDateTimeString(li.QuadPart,pszText,cchTextMax);
	return pszText;
}

VOID WINAPI _GetDateTimeStringEx(ULONG64 DateTime,LPTSTR pszText,int cchTextMax,LPTSTR DateFormat,LPTSTR TimeFormat,BOOL bDisplayAsUTC)
{
	SYSTEMTIME st;
	FILETIME ftLocal;

	if( bDisplayAsUTC )
	{
		FileTimeToSystemTime((FILETIME*)&DateTime,&st);
	}
	else
	{
		FileTimeToLocalFileTime((FILETIME*)&DateTime,&ftLocal);
		FileTimeToSystemTime(&ftLocal,&st);
	}

	int cch;
	cch = GetDateFormat(LOCALE_USER_DEFAULT,
				0,
				&st, 
				DateFormat,
				pszText,cchTextMax);

	pszText[cch-1] = _T(' ');

	GetTimeFormat(LOCALE_USER_DEFAULT,
				dwOldFlags,
				&st, 
				TimeFormat,
				&pszText[cch],cchTextMax-cch);
}

VOID WINAPI _GetDateTimeStringEx2(ULONG64 DateTime,LPTSTR pszText,int cchTextMax,LPTSTR DateFormat,LPTSTR TimeFormat,BOOL bDisplayAsUTC,BOOL bMilliseconds)
{
	SYSTEMTIME st;
	FILETIME ftLocal;
	LARGE_INTEGER liLts;
	LONGLONG ns = 10000000; // 100ns

	if( bDisplayAsUTC )
	{
		FileTimeToSystemTime((FILETIME*)&DateTime,&st);

		liLts.QuadPart = DateTime % 10000000; // Get a less than 1 second.
	}
	else
	{
		FileTimeToLocalFileTime((FILETIME*)&DateTime,&ftLocal);
		FileTimeToSystemTime(&ftLocal,&st);

		liLts.HighPart = ftLocal.dwHighDateTime;
		liLts.LowPart  = ftLocal.dwLowDateTime;
		liLts.QuadPart = liLts.QuadPart % 10000000; // Get a less than 1 second.
	}

	int cch;
	cch = GetDateFormat(LOCALE_USER_DEFAULT,
				0,
				&st, 
				DateFormat,
				pszText,cchTextMax);

	pszText[cch-1] = _T(' ');

	cch += GetTimeFormat(LOCALE_USER_DEFAULT,
				dwOldFlags,
				&st, 
				TimeFormat,
				&pszText[cch],cchTextMax-cch);

	if( bMilliseconds )
	{
		WCHAR szMilliseconds[16];
		int cchMilliseconds;

		WCHAR *p = wcschr(pszText,L'n');
		if( p )
		{
			WCHAR *pMilliseconds = p;
			// count 'n' characters
			while( *p == L'n' ) p++;
			int cch100ns = (int)(p - pMilliseconds);

			if( cch100ns > 0 && ((3 <= cch100ns) && (cch100ns <= 7)))
			{
#if 0
				cchMilliseconds = wsprintf(szMilliseconds,L"%03u",st.wMilliseconds);  // display  1ms "000"
#else
				cchMilliseconds = wsprintf(szMilliseconds,L"%07u",(UINT)liLts.QuadPart); // display 100ns "0000000"
#endif
				int i;
				for(i = 0; i < cch100ns; i++)
				{
					pMilliseconds[i] = szMilliseconds[i];
				}
			}
		}
		else
		{
			if( (cch + 3) < cchTextMax )
			{
				cch += wsprintf(szMilliseconds,L".%03u",st.wMilliseconds);
				wcscat_s(pszText,cchTextMax,szMilliseconds);
			}
		}
	}
}

VOID WINAPI _GetDateTime(ULONG64 DateTime,LPTSTR pszText,int cchTextMax)
{
	_GetDateTimeStringEx(DateTime,pszText,cchTextMax,NULL,NULL,FALSE);
}

VOID WINAPI _GetDateTimeFromFileTime(FILETIME *DateTime,LPTSTR pszText,int cchTextMax)
{
	ULARGE_INTEGER li;
	li.HighPart = DateTime->dwHighDateTime;
	li.LowPart  = DateTime->dwLowDateTime;
	_GetDateTime(li.QuadPart,pszText,cchTextMax);
}
