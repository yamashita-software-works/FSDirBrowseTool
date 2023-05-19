#pragma once

#include "wfslib.h"

typedef struct _FILE_INFORMATION_ALTSTREAM
{
	PWSTR Name;
	LARGE_INTEGER Size;
	LARGE_INTEGER AllocSize;
} FILE_INFORMATION_ALTSTREAM;

typedef struct _FILE_INFORMATON_EA_DATA
{
    UCHAR  Flags;
    UCHAR  NameLength;
    USHORT ValueLength;
	CHAR  *Name;
	UCHAR *Value;
} FILE_INFORMATON_EA_DATA;

typedef struct _FILE_INFORMATON_EA_BUFFER
{
	ULONG EaCount;
	FILE_INFORMATON_EA_DATA Ea[1];
} FILE_INFORMATON_EA_BUFFER;

typedef struct _FILE_INFORMATION_STRUCT
{
	PWSTR Name;
	PWSTR ShortName;
    LARGE_INTEGER FileReferenceNumber;

    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    ULONG FileAttributes;
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    ULONG NumberOfLinks;
    BOOLEAN DeletePending;
    BOOLEAN Directory;

	struct {
	    LARGE_INTEGER CreationTime;
		LARGE_INTEGER LastAccessTime;
	    LARGE_INTEGER LastWriteTime;
		LARGE_INTEGER ChangeTime;
	} DirectoryEnrty;

	struct {
		INT cAltStreamName;
		FILE_INFORMATION_ALTSTREAM *AltStreamName;
	} AltStream;

	ULONG EaSize;
	FILE_INFORMATON_EA_BUFFER *EaBuffer;

	struct {
		UCHAR ObjectId[16];
	    union {
		    struct {
			    UCHAR BirthVolumeId[16];
				UCHAR BirthObjectId[16];
	            UCHAR DomainId[16];
		    };
			UCHAR ExtendedInfo[48];
		};
	} ObjectId;

	WCHAR FileSystemName[16];
	ULONG FileSystemAttributes;
	ULONG MaximumComponentNameLength;

	FS_REPARSE_POINT_INFORMATION_EX ReparsePointInfo;

	struct {
		WOF_EXTERNAL_INFO *ExternalInfo;
		union {
			PVOID GenericPtr;
			FILE_PROVIDER_EXTERNAL_INFO_V1 *FileInfo;
			WIM_PROVIDER_EXTERNAL_INFO *WimInfo;
		};
	} Wof;

	struct
	{
		ULONG ObjectId : 1;
		ULONG ReparsePoint : 1;
		ULONG Wof : 1;
	} State;

} FILE_INFORMATION_STRUCT;

EXTERN_C
HRESULT
APIPRIVATE
NTFile_GatherFileInformation(
	HANDLE hFile,
	FILE_INFORMATION_STRUCT **pfi
	);

EXTERN_C
HRESULT
APIPRIVATE
NTFile_FreeFileInformation(
	FILE_INFORMATION_STRUCT *pfi
	);

EXTERN_C
HRESULT
APIPRIVATE
NTFile_OpenFile(
	HANDLE *phFile,
	PCWSTR FilePath,
	ULONG DesiredAccess,
	ULONG ShareAccess,
	ULONG OpenOptions
	);

EXTERN_C
HRESULT
APIPRIVATE
NTFile_CloseFile(
	HANDLE hFile
	);

EXTERN_C
BOOL
APIPRIVATE
NTFile_GetAttributeString(
	DWORD Attributes,
	LPWSTR String,
	int cchString
	);

