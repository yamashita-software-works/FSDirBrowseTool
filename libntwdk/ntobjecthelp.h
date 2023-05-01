#pragma once 

#define MAX_OBJECT_NAME 260
#define MAX_OBJECT_PATH 1024 

#ifndef NTAPI
#define NTAPI __stdcall
#endif

EXTERN_C
ULONG
NTAPI
OpenObjectDirectory(
	PCWSTR DirectoryName,
	HANDLE *pHandle
	);

EXTERN_C
ULONG
NTAPI
CloseObjectDirectory(
	HANDLE Handle
	);

EXTERN_C
ULONG 
NTAPI
QueryObjectDirectory(
    HANDLE Handle,
    PULONG pulIndex,
    PWSTR ObjectName,
    ULONG ObjectNameLength,
    PWSTR TypeName,
    ULONG TypeNameLength
    );

EXTERN_C
ULONG 
NTAPI
QueryObjectDirectory_U(
    HANDLE Handle,
    PULONG pulIndex,
    UNICODE_STRING *ObjectName,
    UNICODE_STRING *TypeName,
    BOOLEAN AllocateString
    );

EXTERN_C
ULONG
NTAPI
QuerySymbolicLinkObject(
    HANDLE hRootDirectory,
    PCWSTR pszSymbolicName,
	UNICODE_STRING *pusSymbolicName OPTIONAL,
    PWSTR ObjectName,
    int cchObjectName
    );

EXTERN_C
ULONG
NTAPI
QuerySymbolicLinkObject_U(
    HANDLE hObjectDirectory,
    PCWSTR pszSymbolicLinkName,
    UNICODE_STRING *ObjectName,
    BOOLEAN AllocateNameString
    );

EXTERN_C
ULONG
NTAPI
QuerySymbolicLinkObjectName(
    PCWSTR SymbolicLinkPath,
    PCWSTR SymbolicLinkName,
    PWSTR ObjectNameBuffer,      // name buffer
    LONG ObjectNameBufferLength, // buffer length in character
    PULONG ReturnedLength        // returns length of character including terminate null 
    );

EXTERN_C
NTSTATUS
NTAPI
LookupVolumeGuidName(
    PCWSTR NtDevicePath,
    PWSTR VolumeSymbolicLink,
    ULONG cchVolumeSymbolicLink
    );
