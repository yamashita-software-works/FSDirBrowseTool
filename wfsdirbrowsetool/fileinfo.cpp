//***************************************************************************
//*                                                                         *
//*  fileinfo.cpp                                                           *
//*                                                                         *
//*  File Information Functions                                             *
//*                                                                         *
//*  Create: 2023-04-03                                                     *
//*                                                                         *
//*  Author: YAMASHITA Katsuhiro                                            *
//*                                                                         *
//*  Copyright (C) YAMASHITA Katsuhiro. All rights reserved.                *
//*  Licensed under the MIT License.                                        *
//*                                                                         *
//***************************************************************************
#include "ntifs.h"
#include "windef.h"
#include "ntstatus.h"
#include "winerror.h"
#include "fileinfo.h"
#include "libntwdk.h"
#include "..\libntwdk\ntnativeapi.h"
//#include "..\libntwdk\ntnativehelp.h"
#include "builddefs.h"

#if _ENABLE_USE_FSLIB
//
// Tentatively in use dll functions.
//
#include <ntimage.h>
#include <delayimp.h>
#define _NO_NTFSDEF_
#define _NO_FSPATHBUFFER
#include "..\fslib\inc\fslib.h"
HRESULT FsLibGetReparsePointInformation(HANDLE,FILE_INFORMATION_STRUCT*);
#endif

#define PRIVATE static

//---------------------------------------------------------------------------
//
//  NTFile_OpenFile()
//
//---------------------------------------------------------------------------
EXTERN_C
HRESULT
APIENTRY
NTFile_OpenFile(
	HANDLE *phFile,
	PCWSTR FilePath,
	ULONG DesiredAccess,
	ULONG ShareAccess,
	ULONG OpenOptions
	)
{
	HRESULT hr;

	if( OpenFile_W(phFile,NULL,FilePath,DesiredAccess,ShareAccess,OpenOptions) == STATUS_SUCCESS )
	{
		hr = S_OK;
	}
	else
	{
		hr = E_FAIL;
	}
	return hr;
}

//---------------------------------------------------------------------------
//
//  NTFile_CloseFile()
//
//---------------------------------------------------------------------------
EXTERN_C
HRESULT
APIENTRY
NTFile_CloseFile(
	HANDLE hFile
	)
{
	return HRESULT_FROM_NT( NtClose(hFile) );
}

//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//  _DeviceIoControl()
//
//  PURPOSE:
//
//----------------------------------------------------------------------------
PRIVATE
BOOLEAN
NTAPI
_DeviceIoControl(
	HANDLE FileHandle,
	ULONG FsControlCode,
	PVOID InputBuffer,
    ULONG InputBufferLength,
	PVOID OutputBuffer,
    ULONG OutputBufferLength,
	PULONG BytesReturned
	)
{
	NTSTATUS Status;
	IO_STATUS_BLOCK IoStatus;

	RtlZeroMemory(&IoStatus,sizeof(IoStatus));

	Status = NtFsControlFile(FileHandle,NULL,NULL,NULL,&IoStatus,
						FsControlCode,
						InputBuffer,InputBufferLength,
						OutputBuffer,OutputBufferLength);

	if( BytesReturned )
		*BytesReturned = (ULONG)IoStatus.Information;

	RtlSetLastWin32Error( Status );

	return (Status == STATUS_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//  GetAlternateStreamNames()
//
//  PURPOSE:
//
//----------------------------------------------------------------------------
PRIVATE
NTSTATUS
APIENTRY
GetAlternateStreamNames(
	HANDLE hFile,
	INT *pAltStreamCount,
	FILE_STREAM_INFORMATION **StreamInformation
	)
{
	NTSTATUS Status;
	IO_STATUS_BLOCK IoStatus;
	INT cAltStreams = 0;
	FILE_STREAM_INFORMATION s = {0};
	FILE_STREAM_INFORMATION *pfsi = &s;
	ULONG cb = sizeof(FILE_STREAM_INFORMATION);

	if( pAltStreamCount )
		*pAltStreamCount = 0;

	Status = NtQueryInformationFile(hFile,&IoStatus,pfsi,cb,FileStreamInformation);

	if( Status == STATUS_SUCCESS )
		return Status;

	// STATUS_BUFFER_OVERFLOW
	// The output buffer was filled before all of the stream information could be returned. 
	// Only complete FILE_STREAM_INFORMATION structures are returned.
	// STATUS_INFO_LENGTH_MISMATCH
	// The specified information record length does not match the length that is required
	// for the specified information class.

	if( Status != STATUS_BUFFER_OVERFLOW )
		return Status;

	for(;;)
	{
#if 0
		cb += 8; // debug
#else
		cb += 1024;
#endif
		pfsi = (FILE_STREAM_INFORMATION *)AllocMemory( cb );

		if( pfsi == NULL )
		{
			Status = STATUS_NO_MEMORY;
			break;
		}

		Status = NtQueryInformationFile(hFile,&IoStatus,pfsi,cb,FileStreamInformation);

		if( Status == STATUS_SUCCESS )
		{
			*StreamInformation = pfsi;

			if( pAltStreamCount )
			{
				FILE_STREAM_INFORMATION *psn = pfsi;

				for(;;)
				{
					cAltStreams++;
					if( psn->NextEntryOffset == 0 )
						break;
					psn = (FILE_STREAM_INFORMATION *)((ULONG_PTR)psn + psn->NextEntryOffset);
				}
				*pAltStreamCount = cAltStreams;
			}
			break;
		}

		FreeMemory(pfsi);

		if( Status != STATUS_BUFFER_OVERFLOW )
		{
			break;
		}
	}

	return Status;
}

//----------------------------------------------------------------------------
//
//  FreeEAInformation()
//
//  PURPOSE:
//
//----------------------------------------------------------------------------
PRIVATE
NTSTATUS
APIENTRY
FreeEAInformation(
	FILE_INFORMATON_EA_BUFFER *EABuffer
	)
{
	if( EABuffer == NULL )
		return STATUS_INVALID_PARAMETER;

	ULONG i;
	for(i = 0; i < EABuffer->EaCount; i++)
	{
		if( EABuffer->Ea[i].Name )
			FreeMemory( EABuffer->Ea[i].Name );
		if( EABuffer->Ea[i].Value )
			FreeMemory( EABuffer->Ea[i].Value );
	}

	FreeMemory(EABuffer);

	return 0;
}

//----------------------------------------------------------------------------
//
//  GetEAInformation()
//
//  PURPOSE:
//
//----------------------------------------------------------------------------
PRIVATE
NTSTATUS
APIENTRY
GetEAInformation(
	HANDLE hFile,
	INT *pAltStreamCount,
	FILE_INFORMATON_EA_BUFFER **EABuffer
	)
{
	NTSTATUS Status;
	IO_STATUS_BLOCK IoStatus = {0};
	FILE_FULL_EA_INFORMATION *pEA = NULL;
	FILE_INFORMATON_EA_BUFFER *pEaBuffer = NULL;

	__try
	{
		ULONG EaIndex = 1; // Index is 1-based.
		BOOLEAN bStartScan = TRUE;

		ULONG cbEA =  sizeof(FILE_FULL_EA_INFORMATION) + 32768;
		pEA = (FILE_FULL_EA_INFORMATION *)AllocMemory( cbEA );
		if( pEA == NULL )
			__leave;

		for(;;)
		{
			Status = NtQueryEaFile(
						hFile,&IoStatus,
						pEA,cbEA,
						TRUE,// Single entry return
						NULL,0,
						&EaIndex,bStartScan);

			if( Status == STATUS_NONEXISTENT_EA_ENTRY )
			{
				break; // hadn't  ea data
			}

			if( Status == STATUS_NO_MORE_EAS )
			{
				Status = STATUS_SUCCESS;
				break; // End of EA
			}

			if( Status != STATUS_SUCCESS )
			{
				break; // some error
			}

			//
			// Allocatiom/Reallocation EA return buffer
			//
			if( pEaBuffer == NULL )
			{
				pEaBuffer = (FILE_INFORMATON_EA_BUFFER*)AllocMemory( sizeof(FILE_INFORMATON_EA_BUFFER) );
				if( pEaBuffer == NULL )
					break;
				pEaBuffer->EaCount = 0;
			}
			else
			{
				pEaBuffer = (FILE_INFORMATON_EA_BUFFER*)ReallocMemory( pEaBuffer, 
								sizeof(FILE_INFORMATON_EA_BUFFER) + (sizeof(FILE_INFORMATON_EA_DATA) * pEaBuffer->EaCount) );
				if( pEaBuffer == NULL )
					break;
			}

			//
			// Copy EA information
			//
			FILE_INFORMATON_EA_DATA *Item;
			Item = &pEaBuffer->Ea[pEaBuffer->EaCount];

			// EA basic information
			Item->Flags = pEA->Flags;
			Item->NameLength = pEA->EaNameLength;
			Item->ValueLength = pEA->EaValueLength;

			// EA name
			Item->Name = (CHAR *)AllocMemory(pEA->EaNameLength+1);
			if( Item->Name == NULL )
			{
				Status = STATUS_NO_MEMORY;
				break;
			}
			RtlCopyMemory((VOID*)Item->Name,pEA->EaName,pEA->EaNameLength);

			// EA data
			Item->Value = (UCHAR *)AllocMemory(pEA->EaValueLength);
			if( Item->Value == NULL )
			{
				Status = STATUS_NO_MEMORY;
				break;
			}
			RtlCopyMemory((VOID*)Item->Value,pEA->EaName+(pEA->EaNameLength+1),pEA->EaValueLength);

			pEaBuffer->EaCount++;
			bStartScan = FALSE;
			EaIndex++;
		}
	}
	__finally
	{
		if( pEaBuffer == NULL || pEA == NULL )
			Status = STATUS_NO_MEMORY;

		if( Status != STATUS_SUCCESS )
		{
			FreeEAInformation( pEaBuffer );
			pEaBuffer = NULL;
		}

		if( pEA != NULL )
			FreeMemory( pEA );

		*EABuffer = pEaBuffer;
	}

	return Status;
}

//----------------------------------------------------------------------------
//
//  GetObjectIdInformation()
//
//  PURPOSE:
//
//----------------------------------------------------------------------------
PRIVATE
NTSTATUS
APIENTRY
GetObjectIdInformation(
	HANDLE hFile,
	FILE_OBJECTID_BUFFER *pfob
	)
{
	FILE_OBJECTID_BUFFER fob = {0};
	BOOLEAN bSuccess;
	ULONG cb;

	bSuccess = _DeviceIoControl(hFile,FSCTL_GET_OBJECT_ID,
						NULL,0,&fob,sizeof(fob),&cb);

	if( bSuccess )
		*pfob = fob;

	return RtlGetLastWin32Error();
}

//---------------------------------------------------------------------------
//
//  NTFile_CollectFileInformation()
//
//  PURPOSE:
//
//---------------------------------------------------------------------------
EXTERN_C
HRESULT
APIENTRY
NTFile_CollectFileInformation(
	HANDLE hFile,
	FILE_INFORMATION_STRUCT **ppfi
	)
{
	IO_STATUS_BLOCK IoStatus;

	FILE_INFORMATION_STRUCT *pfi = (FILE_INFORMATION_STRUCT *)AllocMemory( sizeof(FILE_INFORMATION_STRUCT) );
	if( pfi == NULL )
		return E_OUTOFMEMORY;

	//
	// Volume information
	//
	ULONG cbfsai = sizeof(FILE_FS_ATTRIBUTE_INFORMATION) + (64);
	FILE_FS_ATTRIBUTE_INFORMATION *fsai = (FILE_FS_ATTRIBUTE_INFORMATION *)AllocMemory(cbfsai);
	NtQueryVolumeInformationFile(hFile,&IoStatus,fsai,cbfsai,FileFsAttributeInformation);
	memcpy(pfi->FileSystemName,fsai->FileSystemName,fsai->FileSystemNameLength);
	pfi->MaximumComponentNameLength = fsai->MaximumComponentNameLength;
	pfi->FileSystemAttributes = fsai->FileSystemAttributes;
	FreeMemory(fsai);

	//
	// File Basic Information
	//
	FILE_BASIC_INFORMATION fbi;
	NtQueryInformationFile(hFile,&IoStatus,&fbi,sizeof(fbi),FileBasicInformation);

	pfi->FileAttributes = fbi.FileAttributes;
	pfi->LastWriteTime = fbi.LastWriteTime;
	pfi->CreationTime = fbi.CreationTime;
	pfi->LastAccessTime = fbi.LastAccessTime;
	pfi->ChangeTime = fbi.ChangeTime;

	//
	// File Standard Information
	//
	FILE_STANDARD_INFORMATION fsi;
	NtQueryInformationFile(hFile,&IoStatus,&fsi,sizeof(fsi),FileStandardInformation);

	pfi->AllocationSize = fsi.AllocationSize;
	pfi->EndOfFile = fsi.EndOfFile;
	pfi->NumberOfLinks = fsi.NumberOfLinks;
	pfi->Directory = fsi.Directory;
	pfi->DeletePending = fsi.DeletePending;

	//
	// File Name / Alternate (Short) Name Information
	//
	ULONG cb = sizeof(FILE_NAME_INFORMATION) + WIN32_MAX_PATH_BYTES;
	FILE_NAME_INFORMATION *pfni = (FILE_NAME_INFORMATION *)AllocMemory(cb);
	if( pfni )
	{
		NtQueryInformationFile(hFile,&IoStatus,pfni,cb,FileNameInformation);
		pfni->FileName[ pfni->FileNameLength / sizeof(WCHAR) ] = UNICODE_NULL;
		pfi->Name = DuplicateString(pfni->FileName);

		NtQueryInformationFile(hFile,&IoStatus,pfni,cb,FileAlternateNameInformation);
		pfni->FileName[ pfni->FileNameLength / sizeof(WCHAR) ] = UNICODE_NULL;
		pfi->ShortName = DuplicateString(pfni->FileName);

		FreeMemory(pfni);
	}

	//
	// Alternate Stream Information
	//
	{
		FILE_STREAM_INFORMATION *StreamInformation = NULL;
		if( GetAlternateStreamNames(hFile,&pfi->AltStream.cAltStreamName,&StreamInformation) == STATUS_SUCCESS )
		{
			if( pfi->AltStream.cAltStreamName > 0 )
			{
				pfi->AltStream.AltStreamName = (FILE_INFORMATION_ALTSTREAM*)AllocMemory( pfi->AltStream.cAltStreamName * sizeof(FILE_INFORMATION_ALTSTREAM) );

				FILE_STREAM_INFORMATION *ps = StreamInformation;
				for(int i = 0;;i++)
				{
					pfi->AltStream.AltStreamName[i].Name = AllocStringLengthCb(ps->StreamName,ps->StreamNameLength);
					pfi->AltStream.AltStreamName[i].Size = ps->StreamSize;
					pfi->AltStream.AltStreamName[i].AllocSize = ps->StreamAllocationSize;
					if( ps->NextEntryOffset == 0 )
						break;
					ps = (FILE_STREAM_INFORMATION *)((ULONG_PTR)ps + ps->NextEntryOffset);
				}
			}

			FreeMemory(StreamInformation);
		}
	}

	//
	// EA(Extended Attributes)
	//
	{
		FILE_EA_INFORMATION fei = {0};
		NtQueryInformationFile(hFile,&IoStatus,&fei,sizeof(FILE_EA_INFORMATION),FileEaInformation);
		if( fei.EaSize != 0 )
		{
			pfi->EaSize = fei.EaSize;
			GetEAInformation(hFile,NULL,&pfi->EaBuffer);
		}
	}

	//
	// Object IDs Information
	//
	FILE_OBJECTID_BUFFER fob = {0};
	if( NT_SUCCESS(GetObjectIdInformation(hFile,&fob)) )
	{
		memcpy(pfi->ObjectId.ObjectId,fob.ObjectId,16);
		memcpy(pfi->ObjectId.BirthVolumeId,fob.BirthVolumeId,16);
		memcpy(pfi->ObjectId.BirthObjectId,fob.BirthObjectId,16);
		memcpy(pfi->ObjectId.DomainId,fob.DomainId,16);

		pfi->State.ObjectId = 1;
	}

	//
	// Reparse Point Information
	//
	if( pfi->FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT )
	{
#if _ENABLE_USE_FSLIB
		 FsLibGetReparsePointInformation(hFile,pfi);
#endif
	}

	//
	// File Compress Information
	//
	if( pfi->FileAttributes & FILE_ATTRIBUTE_COMPRESSED )
	{
		FILE_COMPRESSION_INFORMATION fci = {0};
		if( NT_SUCCESS(NtQueryInformationFile(hFile,&IoStatus,&fci,sizeof(fci),FileCompressionInformation)) )
		{
			;
		}
	}

	//
	// Sparse File Information
	//
	if( pfi->FileAttributes & FILE_ATTRIBUTE_SPARSE_FILE )
	{
		; // todo:
	}
	
	//
	// Wof Information
	//
	; // todo:

	*ppfi = pfi;

	return S_OK;
}

//---------------------------------------------------------------------------
//
//  NTFile_FreeFileInformation()
//
//  PURPOSE:
//
//---------------------------------------------------------------------------
EXTERN_C
HRESULT
APIENTRY
NTFile_FreeFileInformation(
	FILE_INFORMATION_STRUCT *pfi
	)
{
	if( pfi )
	{
		FreeEAInformation(pfi->EaBuffer);

		FreeMemory(pfi->Name);
		FreeMemory(pfi->ShortName);

		int i;
		for(i = 0; i < pfi->AltStream.cAltStreamName; i++)
			FreeMemory(pfi->AltStream.AltStreamName[i].Name);
		FreeMemory(pfi->AltStream.AltStreamName);

		FreeMemory(pfi);
	}
	return 0;
}

//---------------------------------------------------------------------------
//
//  NTFile_GetAttributeString()
//
//  PURPOSE:
//
//---------------------------------------------------------------------------
static struct {
	DWORD AttributeFlag;
	TCHAR AttributeChar;
} attributes_char[] = {
	{FILE_ATTRIBUTE_DIRECTORY,            L'd'},
	{FILE_ATTRIBUTE_READONLY,             L'r'},
	{FILE_ATTRIBUTE_HIDDEN,               L'h'},
	{FILE_ATTRIBUTE_SYSTEM,               L's'},
	{FILE_ATTRIBUTE_ARCHIVE,              L'a'},
	{FILE_ATTRIBUTE_ENCRYPTED,            L'e'},
	{FILE_ATTRIBUTE_COMPRESSED,           L'c'},
	{FILE_ATTRIBUTE_REPARSE_POINT,        L'l'},
	{FILE_ATTRIBUTE_SPARSE_FILE,          L'p'},
	{FILE_ATTRIBUTE_TEMPORARY,            L't'},
	{FILE_ATTRIBUTE_VIRTUAL,              L'v'},
	{FILE_ATTRIBUTE_OFFLINE,              L'o'},
	{FILE_ATTRIBUTE_DEVICE,               L'D'}, // 'x'
	{FILE_ATTRIBUTE_NORMAL,               L'n'},
	{FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,  L'i'},
// Win8/Windows Server 2012
	{FILE_ATTRIBUTE_NO_SCRUB_DATA,        L'X'}, // 'u'->'X'
	{FILE_ATTRIBUTE_INTEGRITY_STREAM,     L'V'}, // 'g'->'V'
// Win10
	{FILE_ATTRIBUTE_EA,                   L'E'},
	{FILE_ATTRIBUTE_PINNED,               L'P'},
	{FILE_ATTRIBUTE_UNPINNED,             L'U'},
	{FILE_ATTRIBUTE_RECALL_ON_OPEN,       L'R'},
	{FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS,L'D'},
	{FILE_ATTRIBUTE_STRICTLY_SEQUENTIAL,  L'Q'},
};

EXTERN_C
BOOL
APIENTRY
NTFile_GetAttributeString(
	DWORD Attributes,
	LPWSTR String,
	int cchString
	)
{
	int i,c;

	c = ARRAYSIZE(attributes_char);

	for(i = 0; i < c; i++)
	{
		if( Attributes & attributes_char[i].AttributeFlag )
		{
			*String++ = attributes_char[i].AttributeChar;
		}
		else
		{
			//*String++ = _T('-');
		}
	}

	*String = L'\0';

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////

#if _ENABLE_USE_FSLIB
//
// Tentatively in use dll functions.
//
int CheckDelayException(int exception_value)
{
    if (exception_value == VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND) ||
        exception_value == VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND))
    {
        // This example just executes the handler.
        return EXCEPTION_EXECUTE_HANDLER;
    }
    // Don't attempt to handle other errors
    return EXCEPTION_CONTINUE_SEARCH;
}

HRESULT FsLibGetReparsePointInformation(HANDLE hFile,FILE_INFORMATION_STRUCT *pfi)
{
	HRESULT hr = E_FAIL;
	DWORD dw;
	__try 
	{ 
		FS_REPARSE_POINT_INFORMATION fsrpi = {0};
		fsrpi.TargetPathLength = 32768+260;
		fsrpi.TargetPath = (PWSTR)AllocMemory(fsrpi.TargetPathLength);
		fsrpi.PrintPathLength = 32768+260;
		fsrpi.PrintPath = (PWSTR)AllocMemory(fsrpi.PrintPathLength);
		if( FsGetReparsePointInformation( hFile, NULL, FsReparsePointDetail, &fsrpi, sizeof(fsrpi)) )
		{
			//pfi
//				fsrpi.ReparseTag
//				fsrpi.TargetPath
//				fsrpi.PrintPath
		}
		FreeMemory( fsrpi.TargetPath );
		FreeMemory( fsrpi.PrintPath );
    } 
	__except( CheckDelayException( dw = GetExceptionCode() ) )
    { 
        hr = HRESULT_FROM_WIN32( (dw & 0xFFFF) );
    }
	return hr;
}
#endif
