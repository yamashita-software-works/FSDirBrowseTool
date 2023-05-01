#pragma once

HRESULT DirectoryTraverser_CreateWindow(HWND hWnd,HWND *phWnd);
HRESULT DirectoryTraverser_InitData(HWND hWnd,HWND hWndNotify);
HRESULT DirectoryTraverser_InitLayout(HWND hWnd,RECT *prc);
HRESULT DirectoryTraverser_SelectFolder(HWND hWnd,PCWSTR pszFolderPath,UINT Reserved);
HRESULT DirectoryTraverser_FillItems(HWND hWnd,PCWSTR pszDirectoryPath);
HRESULT DirectoryTraverser_SetNotifyWnd(HWND hWnd,HWND hWndNotify);
HRESULT DirectoryTraverser_GetNotifyWnd(HWND hWnd,HWND *phWnd);
