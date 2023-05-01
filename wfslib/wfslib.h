#pragma once

//
// NtPath
//
EXTERN_C
UINT
APIENTRY
NtPathIsNtDevicePath(
	PCWSTR pszPath
	);

EXTERN_C
BOOL
APIENTRY
NtPathGetVolumeName(
	PCWSTR pszPath,
	PWSTR pszVolumeName,
	ULONG cchVolumeName
	);

EXTERN_C
BOOL
APIENTRY
NtPathGetRootDirectory(
	PCWSTR pszPath,
	PWSTR pszRootDir,
	ULONG cchRootDir
	);

EXTERN_C
BOOL
APIENTRY
NtPathFileExists(
	PCWSTR pszPath
	);

//
// Utility
//
EXTERN_C
ULONG
APIPRIVATE
_StringFromGUID(
	const GUID *pGuid,
	PWSTR pszGuid,
	ULONG cchMax
	);

EXTERN_C
BOOL
APIENTRY
_IsRootDirectory(
	PCWSTR pszPath
	);

EXTERN_C
BOOL
APIENTRY
_HasPrefix(
	PCWSTR pszPrefix,
	PCWSTR pszPath
	);
