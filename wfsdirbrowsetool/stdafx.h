#pragma once

#include "targetver.h"

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#undef WIN32_NO_STATUS        // defines STATUS_XXX in ntddk, now using includes DDK.
#include <ntstatus.h>

#define WIN32_LEAN_AND_MEAN
#define WIN32_NO_STATUS       // no include STATUS_XXX in winnt.h
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <strsafe.h>
#include <winternl.h> // WinSDK 7.1
#include <winioctl.h>
#include <commoncontrols.h>
#include <uxtheme.h>

#include "builddefs.h"

#include "common_control_helper.h"

#if _MSC_VER <= 1500
#define nullptr NULL
#endif

#pragma warning(disable : 4995)

#include "debug.h"
#include "mem.h"
#include "libmisc.h"
#include "dparray.h"
#include "..\wfslib\wfslib.h"
#include "..\libntwdk\libntwdk.h"
#include "..\inc\common.h"
#include "..\inc\common_resid.h"

#define  _ASSERT ASSERT

HINSTANCE _GetResourceInstance();

//
// Data definitions
//
#include "fileinfo.h"

typedef struct _FILEITEMHEADER
{
	ULONG Reserved;
	PWSTR Path;
	PWSTR FileName;
} FILEITEMHEADER;

typedef struct _FILEITEM
{
	FILEITEMHEADER hdr;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    ULONG FileIndex;
    LARGE_INTEGER FileId;
    WCHAR ShortName[14];
} FILEITEM,*PFILEITEM;

class CFileItem : public FILEITEM
{
public:
	CFileItem()
	{
		memset(this,0,sizeof(FILEITEM));
	}

	CFileItem(PCWSTR pszDirPath,PCWSTR pszFile)
	{
		memset(this,0,sizeof(FILEITEM));
		if( pszDirPath )
			hdr.Path = _MemAllocString(pszDirPath);
		if( pszFile )
			hdr.FileName = _MemAllocString(pszFile);
	}

	~CFileItem()
	{
		_SafeMemFree(hdr.Path);
		_SafeMemFree(hdr.FileName);
	}
};

typedef struct _FILELIST
{
	PWSTR RootPath;
	ULONG cItemCount;
	CFileItem **pFI;
} FILELIST,*PFILELIST;

enum {
	TitleNone = 0,                // 0
	TitleName,                    // 1
	TitleAttributes,              // 2
	TitleLastWriteTime,           // 3
	TitleCreationTime,            // 4
	TitleLastAccessTime,          // 5
	TitleChangeTime,              // 6
	TitleLastWriteTimeDirEntry,   // 7
	TitleCreationTimeDirEntry,    // 8
	TitleLastAccessTimeDirEntry,  // 9
	TitleChangeTimeDirEntry,      // 10
	TitleEndOfFile,               // 11
	TitleAllocationSize,          // 12
	TitleEndOfFileDirEntry,       // 13
	TitleAllocationSizeDirEntry,  // 14
	TitleNumberOfHardLink,        // 15
	TitleDirectory,               // 16
	TitleDeletePending,           // 17
	TitleShortName,               // 18
	TitleExtension,               // 19
	TitleEAData,                  // 20
	TitleObjectId,                // 21
	TitleBirthVolumeId,           // 22
	TitleBirthObjectId,           // 23
	TitleDomainId,                // 24
	TitleFileId,                  // 25
	TitleLocation,                // 26
	TitleWofItem,                 // 27
	TitleCount,
	TitleTableSize = TitleCount,
};

//
// Folder tree item ident
//
typedef enum {
	ITEM_FOLDER_UNKNOWN,
	ITEM_FOLDER_DIRECTORY,
	ITEM_FOLDER_FILENAME,
	ITEM_FOLDER_ROOT,
	ITEM_FOLDER_PATH,
	ITEM_BLANK,
	ITEM_GROUP,
} TREEITEMTYPE;

//
// View Type
//
enum {
	VIEW_CURRENT=0,
	VIEW_ROOT,
	VIEW_FILEINFO,
	MAX_INFO_VIEW_TYPE,
};

typedef struct _SELECT_TITLE
{
	UINT TitleId;
	FILE_INFORMATION_STRUCT *pFileInfo;
} SELECT_TITLE;

HIMAGELIST GetGlobalShareImageList();
INT GetUpDirImageIndex();
