// 2023.03.31
#pragma once

#include "basewindow.h"

HWND DirectoryBrowseTool_CreateWindow(HWND hWnd);
VOID DirectoryBrowseTool_InitData(HWND hWndDirBrowse,PCWSTR pszDirectoryPath);
VOID DirectoryBrowseTool_InitLayout(HWND hWndDirBrowse,const RECT *prcDesktopWorkArea);

