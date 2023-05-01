//***************************************************************************
//*                                                                         *
//*  wfslib.cpp                                                             *
//*                                                                         *
//*  NT native API functions                                                *
//*                                                                         *
//*  Create: 2023-04-26                                                     *
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
#include "libntwdk.h"
#include "..\libntwdk\ntnativeapi.h"
#include "ntobjecthelp.h"
#include "wfslib.h"

//
// Utility
//
EXTERN_C
ULONG
APIENTRY
_StringFromGUID(
	const GUID *pGuid,
	PWSTR pszGuid,
	ULONG cchMax
	)
{
	return RtlNtStatusToDosError( StringFromGUID(pGuid,pszGuid,cchMax) );
}

EXTERN_C
BOOL
APIENTRY
_IsRootDirectory(
	PCWSTR pszPath
	)
{
	UNICODE_STRING us;
	RtlInitUnicodeString(&us,pszPath);
	return IsRootDirectory_U(&us) ? TRUE : FALSE;
}

EXTERN_C
BOOL
APIENTRY
_HasPrefix(
	PCWSTR pszPrefix,
	PCWSTR pszPath
	)
{
	return HasPrefix(pszPrefix,pszPath);
}

//
// NtPath
//
EXTERN_C
UINT
APIENTRY
NtPathIsNtDevicePath(
	PCWSTR pszPath
	)
{
	return (UINT)IsNtDevicePath(pszPath);
}

EXTERN_C
BOOL
APIENTRY
NtPathGetRootDirectory(
	PCWSTR pszPath,
	PWSTR pszRootDir,
	ULONG cchRootDir
	)
{
	if( pszPath == NULL || pszRootDir == NULL || cchRootDir == 0)
		return FALSE;

	UNICODE_STRING usPath;
	RtlInitUnicodeString(&usPath,pszPath);
	if( GetRootDirectory_U(&usPath) == false )
	{
		return FALSE;
	}

	if( WCHAR_LENGTH(usPath.Length) >= cchRootDir )
	{
		// Copy as much as possible.
		memcpy(pszRootDir,usPath.Buffer,WCHAR_BYTES(cchRootDir));
		pszRootDir[ cchRootDir-1 ] = UNICODE_NULL;
		return FALSE;
	}

	memcpy(pszRootDir,usPath.Buffer,usPath.Length);
	pszRootDir[ WCHAR_LENGTH(usPath.Length) ] = UNICODE_NULL;

	return TRUE;
}

EXTERN_C
BOOL
APIENTRY
NtPathGetVolumeName(
	PCWSTR pszPath,
	PWSTR pszVolumeName,
	ULONG cchVolumeName
	)
{
	if( pszPath == NULL || pszVolumeName == NULL || cchVolumeName == 0)
		return FALSE;

	UNICODE_STRING usPath;
	RtlInitUnicodeString(&usPath,pszPath);
	if( GetVolumeName_U(&usPath) == false )
	{
		return FALSE;
	}

	if( WCHAR_LENGTH(usPath.Length) >= cchVolumeName )
	{
		return FALSE;
	}

	memcpy(pszVolumeName,usPath.Buffer,usPath.Length);

	pszVolumeName[ WCHAR_LENGTH(usPath.Length) ] = UNICODE_NULL;

	return TRUE;
}

EXTERN_C
BOOL
APIENTRY
NtPathFileExists(
	PCWSTR pszPath
	)
{
	return PathFileExists_W(pszPath,NULL);
}

EXTERN_C
BOOL
APIENTRY
NtPathQueryVolumeDeviceName(
	PCWSTR pszGrobalRootPrefixedPath,
	PWSTR pszBuffer,
	ULONG cchBuffer
	)
{
	UNICODE_STRING us;
	RtlInitUnicodeString(&us,pszGrobalRootPrefixedPath);
	if( PathIsGlobalRootPrefix_U(&us) )
	{
		GetVolumeName_U(&us);
		QuerySymbolicLinkObject(NULL,NULL,&us,pszBuffer,cchBuffer);
	}
	return TRUE;
}
