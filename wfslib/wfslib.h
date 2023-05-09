#pragma once
//
// Helper functions for Win32/NtNative module
//

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

typedef struct _FS_REPARSE_POINT_INFORMATION_EX
{
	ULONG Flags;
	ULONG ReparseTag;
	USHORT ReparseDataLength;
    USHORT Reserved;
	union
	{
		struct {
			PWSTR TargetPath;
			ULONG TargetPathLength;
			PWSTR PrintPath;
			ULONG PrintPathLength;
		} MountPoint;

		struct {
			PWSTR TargetPath;
			ULONG TargetPathLength;
			PWSTR PrintPath;
			ULONG PrintPathLength;
			ULONG Flags;
		} SymLink;

		struct {
			ULONG Version;	  // Currently version 3
			PWSTR PackageID;  // L"Microsoft.WindowsTerminal_8wekyb3d8bbwe"
			PWSTR EntryPoint; // L"Microsoft.WindowsTerminal_8wekyb3d8bbwe!App"
			PWSTR Executable; // L"C:\Program Files\WindowsApps\Microsoft.WindowsTerminal_1.4.3243.0_x64__8wekyb3d8bbwe\wt.exe"
			PWSTR ApplicType; // "0" Integer as ASCII. "0" = Desktop bridge application; Else sandboxed UWP application
			PUCHAR Buffer;
		} AppExecLink;

		struct {
			PUCHAR Buffer;
		} GenericReparse;
	};
} FS_REPARSE_POINT_INFORMATION_EX;

