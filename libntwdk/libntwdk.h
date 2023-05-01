#pragma once

#include "ntpathcomponent.h"

//
// include for win32 build
//
#ifndef _NTIFS_
#ifndef _WINTERNL_

typedef struct _UNICODE_STRING {
  USHORT  Length;
  USHORT  MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

#endif

//
// WDK definitions for Win32 build compatible
//

typedef struct _FILE_DIRECTORY_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;

typedef struct _FILE_FULL_DIR_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    WCHAR FileName[1];
} FILE_FULL_DIR_INFORMATION, *PFILE_FULL_DIR_INFORMATION;

typedef struct _FILE_ID_FULL_DIR_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    LARGE_INTEGER FileId;
    WCHAR FileName[1];
} FILE_ID_FULL_DIR_INFORMATION, *PFILE_ID_FULL_DIR_INFORMATION;

typedef struct _FILE_BOTH_DIR_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    CCHAR ShortNameLength;
    WCHAR ShortName[12];
    WCHAR FileName[1];
} FILE_BOTH_DIR_INFORMATION, *PFILE_BOTH_DIR_INFORMATION;

typedef struct _FILE_ID_BOTH_DIR_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    CCHAR ShortNameLength;
    WCHAR ShortName[12];
    LARGE_INTEGER FileId;
    WCHAR FileName[1];
} FILE_ID_BOTH_DIR_INFORMATION, *PFILE_ID_BOTH_DIR_INFORMATION;

typedef struct _FILE_NAMES_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAMES_INFORMATION, *PFILE_NAMES_INFORMATION;

typedef struct _FILE_ID_GLOBAL_TX_DIR_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    LARGE_INTEGER FileId;
    GUID LockingTransactionId;
    ULONG TxInfoFlags;
    WCHAR FileName[1];
} FILE_ID_GLOBAL_TX_DIR_INFORMATION, *PFILE_ID_GLOBAL_TX_DIR_INFORMATION;

#define FILE_ID_GLOBAL_TX_DIR_INFO_FLAG_WRITELOCKED         0x00000001
#define FILE_ID_GLOBAL_TX_DIR_INFO_FLAG_VISIBLE_TO_TX       0x00000002
#define FILE_ID_GLOBAL_TX_DIR_INFO_FLAG_VISIBLE_OUTSIDE_TX  0x00000004

typedef struct _FILE_OBJECTID_INFORMATION {
    LONGLONG FileReference;
    UCHAR ObjectId[16];
    union {
        struct {
            UCHAR BirthVolumeId[16];
            UCHAR BirthObjectId[16];
            UCHAR DomainId[16];
        } DUMMYSTRUCTNAME;
        UCHAR ExtendedInfo[48];
    } DUMMYUNIONNAME;
} FILE_OBJECTID_INFORMATION, *PFILE_OBJECTID_INFORMATION;

#endif

#define USN_VERSION_2 2
#define USN_VERSION_3 3

#ifndef FILE_ATTRIBUTE_INTEGRITY_STREAM
#define FILE_ATTRIBUTE_INTEGRITY_STREAM      0x00008000
#endif

#ifndef FILE_ATTRIBUTE_NO_SCRUB_DATA
#define FILE_ATTRIBUTE_NO_SCRUB_DATA         0x00020000
#endif

#ifndef FILE_ATTRIBUTE_EA
#define FILE_ATTRIBUTE_EA                    0x00040000
#endif

#ifndef FILE_ATTRIBUTE_PINNED
#define FILE_ATTRIBUTE_PINNED                0x00080000
#endif

#ifndef FILE_ATTRIBUTE_UNPINNED
#define FILE_ATTRIBUTE_UNPINNED              0x00100000
#endif

#ifndef FILE_ATTRIBUTE_RECALL_ON_OPEN
#define FILE_ATTRIBUTE_RECALL_ON_OPEN        0x00040000
#endif

#ifndef FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS
#define FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS 0x00400000
#endif

#ifndef FILE_ATTRIBUTE_STRICTLY_SEQUENTIAL
#define FILE_ATTRIBUTE_STRICTLY_SEQUENTIAL   0x20000000  
#endif

typedef struct _FSDIRENUMCALLBACKINFO
{
    HANDLE DirectoryHandle;
    PCWSTR Path;
} FSDIRENUMCALLBACKINFO,*PFSDIRENUMCALLBACKINFO;

typedef HRESULT (CALLBACK *FSHELPENUMCALLBACKPROC)(
    ULONG InformationType,
    PVOID Information,
    PFSDIRENUMCALLBACKINFO DirEnumCallbackInfo,
    PVOID Context
    );

EXTERN_C
HRESULT
__stdcall
WinEnumFiles(
    PCWSTR Path,
    PCWSTR FileNameFilter,
    ULONG Flags,
    FSHELPENUMCALLBACKPROC Callback,
    PVOID Context
    );

#ifdef __cplusplus
extern "C" {
#endif

#include "ntnativehelp.h"

#ifdef __cplusplus
};
#endif
